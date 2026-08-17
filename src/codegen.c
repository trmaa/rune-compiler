/* codegen.c
 * code generation module for zc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include "debug.h"
#include "tokenizer.h"
#include "codegen.h"

static struct global_var globals[MAX_GLOBALS];
static int global_count;

static struct local_var locals[MAX_LOCALS];
static int local_count;
static int stack_size;

static char strings[MAX_STRINGS][MAX_STR_LEN];
static int string_count;

static int current;
static int out_fd;

static void emit_function(int pub);
static void emit_let(void);
static void emit_call(void);
static void emit_ret(void);

static struct token
cur(void)
{
	if (current >= token_count)
		return (struct token){EOFT, "", 0};
	return tokens[current];
}

static struct token
adv(void)
{
	return tokens[current++];
}

static int
match(int type)
{
	if (cur().type == type) {
		current++;
		return 1;
	}
	return 0;
}

static int
expect(int type)
{
	if (cur().type != type)
		fatal(USER_ERR, NULL, "Expected token type %d, got %d", type, cur().type);
	current++;
	return 1;
}

static void
out(const char *fmt, ...)
{
	char buf[1024];
	va_list args;
	int len;

	va_start(args, fmt);
	len = vsnprintf(buf, sizeof buf, fmt, args);
	va_end(args);

	write(out_fd, buf, len);
}

static int
add_string(char *start, int length)
{
	if (string_count >= MAX_STRINGS)
		fatal(OS_ERR, NULL, "Too many strings");
	if (length >= MAX_STR_LEN)
		fatal(OS_ERR, NULL, "String too long");
	memcpy(strings[string_count], start, length);
	strings[string_count][length] = '\0';
	return string_count++;
}

static int
find_string(char *start, int length)
{
	int i;
	char tmp[MAX_STR_LEN];

	if (length >= MAX_STR_LEN)
		fatal(OS_ERR, NULL, "String too long");
	memcpy(tmp, start, length);
	tmp[length] = '\0';

	for (i = 0; i < string_count; i++)
		if (!strcmp(strings[i], tmp))
			return i;
	return -1;
}

static int
find_local(char *name, int name_len)
{
	int i;
	for (i = 0; i < local_count; i++)
		if (locals[i].name_len == name_len && !memcmp(locals[i].name, name, name_len))
			return i;
	return -1;
}

static int
find_global(char *name, int name_len)
{
	int i;
	for (i = 0; i < global_count; i++)
		if (globals[i].name_len == name_len && !memcmp(globals[i].name, name, name_len))
			return i;
	return -1;
}

static void
emit_global_data(int pub)
{
	if (pub)
		globals[global_count].is_pub = 1;
	else
		globals[global_count].is_pub = 0;

	while (cur().type == LETT) {
		adv();
		expect(IDENTT);
		globals[global_count].name = tokens[current - 1].start;
		globals[global_count].name_len = tokens[current - 1].length;

		if (pub) {
			out("\t.globl\t%.*s\n", globals[global_count].name_len,
			    globals[global_count].name);
			pub = 0;
		}

		if (cur().type == LBRAT) {
			adv();
			globals[global_count].is_array = 1;
			globals[global_count].array_size = atoi(cur().start);
			globals[global_count].has_init = 0;
			adv();
			expect(RBRAT);
			expect(SEMIT);
			out("%.*s:\n", globals[global_count].name_len, globals[global_count].name);
			out("\t.zero %d\n", globals[global_count].array_size * 4);
		} else if (cur().type == EQUALT) {
			adv();
			globals[global_count].is_array = 0;
			globals[global_count].array_size = 0;
			globals[global_count].has_init = 1;
			globals[global_count].init_val = atoi(cur().start);
			adv();
			expect(SEMIT);
			out("%.*s:\n", globals[global_count].name_len, globals[global_count].name);
			out("\t.long %d\n", globals[global_count].init_val);
		} else {
			globals[global_count].is_array = 0;
			globals[global_count].array_size = 0;
			globals[global_count].has_init = 0;
			expect(SEMIT);
			out("%.*s:\n", globals[global_count].name_len, globals[global_count].name);
			out("\t.zero 0\n");
		}
		global_count++;
	}
}

static void
emit_strings(void)
{
	int i;
	for (i = 0; i < string_count; i++)
		out("str%d:\n\t.string \"%s\"\n", i, strings[i]);
}

/*
 * Pre-scans the token stream to collect all string literals
 * and assign them indices. This must happen before code generation
 * so strings can be emitted in the .data section before .text.
 */
static void
collect_strings(void)
{
	int i = 0;

	while (tokens[i].type != EOFT) {
		if (tokens[i].type == STRT)
			add_string(tokens[i].start, tokens[i].length);
		i++;
	}
}

/*
 * Emits assembly code for a function declaration.
 * Performs two passes over the function body: the first
 * calculates total stack space and builds the locals table,
 * the second emits the actual assembly instructions.
 */
static void
emit_function(int pub)
{
	char *fn_name;
	int fn_name_len;
	int body_start;

	fn_name = cur().start;
	fn_name_len = cur().length;
	adv();

	local_count = 0;
	stack_size = 0;

	if (pub)
		out("\t.globl\t%.*s\n", fn_name_len, fn_name);
	out("%.*s:\n", fn_name_len, fn_name);
	out("\tpush\t%%ebp\n");
	out("\tmov\t%%esp, %%ebp\n");

	expect(LBRAC);
	body_start = current;

	/* first pass: calculate stack size and build locals table */
	while (cur().type != RBRAC) {
		if (cur().type == LETT) {
			char *lname;
			int lname_len;
			adv();
			lname = cur().start;
			lname_len = cur().length;
			adv(); /* skip variable name */
			if (cur().type == LBRAT) {
				adv();
				stack_size += atoi(cur().start) * 4;
				adv();
				expect(RBRAT);
				locals[local_count].name = lname;
				locals[local_count].name_len = lname_len;
				locals[local_count].offset = -stack_size;
				local_count++;
				if (match(EQUALT)) {
					expect(LBRAC);
					while (cur().type != RBRAC) {
						if (match(COMMAT))
							continue;
						adv();
					}
					expect(RBRAC);
				}
				expect(SEMIT);
			} else if (cur().type == EQUALT) {
				adv();
				stack_size += 4;
				locals[local_count].name = lname;
				locals[local_count].name_len = lname_len;
				locals[local_count].offset = -stack_size;
				local_count++;
				adv();
				expect(SEMIT);
			} else {
				stack_size += 4;
				locals[local_count].name = lname;
				locals[local_count].name_len = lname_len;
				locals[local_count].offset = -stack_size;
				local_count++;
				expect(SEMIT);
			}
		} else if (cur().type == RETT) {
			adv();
			if (cur().type != SEMIT)
				adv();
			expect(SEMIT);
		} else if (cur().type == IDENTT) {
			adv();
			expect(LPAREN);
			while (cur().type != RPAREN) {
				if (match(COMMAT))
					continue;
				adv();
			}
			expect(RPAREN);
			expect(SEMIT);
		} else {
			adv();
		}
	}

	if (stack_size > 0)
		out("\tsub\t$%d, %%esp\n", stack_size);

	/* second pass: emit code */
	current = body_start;

	while (cur().type != RBRAC) {
		switch (cur().type) {
		case LETT:
			emit_let();
			break;
		case IDENTT:
			emit_call();
			break;
		case RETT:
			emit_ret();
			break;
		default:
			fatal(USER_ERR, NULL, "Unexpected token in function body");
		}
	}

	expect(RBRAC);

	out("\tmov\t%%ebp, %%esp\n");
	out("\tpop\t%%ebp\n");
	out("\tret\n");
}

static void
emit_let(void)
{
	char *name;
	int name_len;
	int idx;
	int offset;

	expect(LETT);
	name = cur().start;
	name_len = cur().length;
	expect(IDENTT);

	if (cur().type == LBRAT) {
		adv();
		adv();
		expect(RBRAT);

		if (match(EQUALT)) {
			int arr_off;
			expect(LBRAC);
			idx = find_local(name, name_len);
			arr_off = locals[idx].offset;
			while (cur().type != RBRAC) {
				if (match(COMMAT))
					continue;
				out("\tmov\t$%d, %d(%%ebp)\n",
				    atoi(cur().start), arr_off);
				arr_off += 4;
				adv();
			}
			expect(RBRAC);
		}
		expect(SEMIT);
		return;
	}

	if (cur().type == EQUALT) {
		adv();
		idx = find_local(name, name_len);
		offset = locals[idx].offset;

		if (cur().type == NUMT) {
			out("\tmov\t$%d, %d(%%ebp)\n", atoi(cur().start), offset);
			adv();
		} else if (cur().type == IDENTT) {
			if (find_local(cur().start, cur().length) >= 0) {
				out("\tmov\t%d(%%ebp), %%eax\n", locals[find_local(cur().start, cur().length)].offset);
				out("\tmov\t%%eax, %d(%%ebp)\n", offset);
			} else if (find_global(cur().start, cur().length) >= 0) {
				out("\tmov\t%.*s, %%eax\n", cur().length, cur().start);
				out("\tmov\t%%eax, %d(%%ebp)\n", offset);
			}
			adv();
		}
		expect(SEMIT);
		return;
	}

	expect(SEMIT);
}

static void
emit_call(void)
{
	char *fn_name;
	int fn_len;
	int args_count = 0;
	int i;
	char *arg_types[32];
	char *arg_starts[32];
	int arg_lengths[32];

	fn_name = cur().start;
	fn_len = cur().length;
	adv();
	expect(LPAREN);

	while (cur().type != RPAREN) {
		if (match(COMMAT))
			continue;
		arg_types[args_count] = cur().start;
		arg_starts[args_count] = cur().start;
		arg_lengths[args_count] = cur().length;
		args_count++;
		adv();
	}
	expect(RPAREN);
	expect(SEMIT);

	for (i = args_count - 1; i >= 0; i--) {
		if (strings[0][0] && find_string(arg_starts[i], arg_lengths[i]) >= 0)
			out("\tpush\t$str%d\n", find_string(arg_starts[i], arg_lengths[i]));
		else if (find_local(arg_starts[i], arg_lengths[i]) >= 0)
			out("\tpush\t%d(%%ebp)\n", locals[find_local(arg_starts[i], arg_lengths[i])].offset);
		else if (find_global(arg_starts[i], arg_lengths[i]) >= 0)
			out("\tpush\t%.*s\n", arg_lengths[i], arg_starts[i]);
		else if (arg_lengths[i] == 1 && *arg_starts[i] >= '0' && *arg_starts[i] <= '9')
			out("\tpush\t$%d\n", atoi(arg_starts[i]));
		else
			out("\tpush\t$%d\n", atoi(arg_starts[i]));
	}

	out("\tcall\t%.*s\n", fn_len, fn_name);
	if (args_count > 0)
		out("\tadd\t$%d, %%esp\n", args_count * 4);
}

static void
emit_ret(void)
{
	int idx;

	expect(RETT);

	if (cur().type == SEMIT) {
		adv();
		return;
	}

	if (cur().type == NUMT) {
		out("\tmov\t$%d, %%eax\n", atoi(cur().start));
		adv();
	} else if (cur().type == IDENTT) {
		idx = find_local(cur().start, cur().length);
		if (idx >= 0)
			out("\tmov\t%d(%%ebp), %%eax\n", locals[idx].offset);
		else if (find_global(cur().start, cur().length) >= 0)
			out("\tmov\t%.*s, %%eax\n", cur().length, cur().start);
		adv();
	}

	expect(SEMIT);
}

void codegen(int fd)
{
	out_fd = fd;
	current = 0;
	global_count = 0;
	string_count = 0;

	collect_strings();

	out("\t.data\n");
	out("\t.text\n");
	while (cur().type != EOFT) {
		if (cur().type == PUBT) {
			adv();
			if (cur().type == FNT) {
				adv();
				emit_function(1);
			} else if (cur().type == LETT) {
				emit_global_data(1);
			} else {
				fatal(USER_ERR, NULL, "Expected fn or let after pub");
			}
		} else if (cur().type == FNT) {
			adv();
			emit_function(0);
		} else if (cur().type == LETT) {
			emit_global_data(0);
		} else {
			fatal(USER_ERR, NULL, "Expected function or global declaration");
		}
	}
	emit_strings();
}
