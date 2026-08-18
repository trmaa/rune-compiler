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
static int label_count;

static void emit_function(int pub);
static void emit_decl(int type);
static void emit_call(void);
static void emit_ret(void);
static void emit_expr(void);
static void emit_expr_prec(int min_prec);
static void emit_unary(void);
static void emit_primary(void);
static void emit_assignment(void);
static void emit_if(void);
static void emit_while(void);
static void emit_for(void);
static void emit_stmt(void);
static void emit_body_or_stmt(void);
static void skip_stmt(void);
static void skip_body_or_stmt(void);

static struct token
cur(void)
{
	if (current >= token_count)
		return (struct token){EOFT, "", 0};
	return tokens[current];
}

static struct token
peek(int offset)
{
	if (current + offset >= token_count)
		return (struct token){EOFT, "", 0};
	return tokens[current + offset];
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

static int
type_size(int type)
{
	return type == TYPE_BYTE ? 1 : 4;
}

static void
emit_load_var(char *name, int name_len)
{
	int idx;
	idx = find_local(name, name_len);
	if (idx >= 0) {
		if (locals[idx].type == TYPE_BYTE)
			out("\tmovzbl\t%d(%%ebp), %%eax\n", locals[idx].offset);
		else
			out("\tmov\t%d(%%ebp), %%eax\n", locals[idx].offset);
	} else if (find_global(name, name_len) >= 0) {
		idx = find_global(name, name_len);
		if (globals[idx].type == TYPE_BYTE)
			out("\tmovzbl\t%.*s, %%eax\n", name_len, name);
		else
			out("\tmov\t%.*s, %%eax\n", name_len, name);
	} else {
		fatal(USER_ERR, NULL, "Undefined variable '%.*s'", name_len, name);
	}
}

static void
emit_store_var(char *name, int name_len)
{
	int idx;
	idx = find_local(name, name_len);
	if (idx >= 0) {
		if (locals[idx].type == TYPE_BYTE)
			out("\tmovb\t%%al, %d(%%ebp)\n", locals[idx].offset);
		else
			out("\tmov\t%%eax, %d(%%ebp)\n", locals[idx].offset);
	} else if (find_global(name, name_len) >= 0) {
		idx = find_global(name, name_len);
		if (globals[idx].type == TYPE_BYTE)
			out("\tmovb\t%%al, %.*s\n", name_len, name);
		else
			out("\tmov\t%%eax, %.*s\n", name_len, name);
	} else {
		fatal(USER_ERR, NULL, "Undefined variable '%.*s'", name_len, name);
	}
}

static void emit_body(void);
static void emit_primary(void);
static int get_prec(int type);

static void
skip_expr(void)
{
	/* skip unary prefix */
	if (cur().type == TILDE || cur().type == BANG) {
		adv();
		skip_expr();
		return;
	}

	/* atom */
	if (cur().type == LPAREN) {
		adv();
		skip_expr();
		expect(RPAREN);
	} else if (cur().type == NUMT || cur().type == CHARLITT || cur().type == IDENTT) {
		adv();
	} else {
		fatal(USER_ERR, NULL, "Expected expression in skip");
		return;
	}

	/* skip binary operators and their right-hand sides */
	while (get_prec(cur().type) > 0) {
		adv();
		skip_expr();
	}
}

/*
 * Skips a single statement without generating code.
 * Used by the first pass to calculate stack sizes.
 */
static void
skip_stmt(void)
{
	if (cur().type == WORDT || cur().type == BYTET) {
		int vtype = (cur().type == WORDT) ? TYPE_WORD : TYPE_BYTE;
		adv();
		adv(); /* name */
		if (cur().type == LBRAT) {
			adv();
			if (cur().type == NUMT)
				adv();
			expect(RBRAT);
			if (cur().type == EQUALT) {
				adv();
				while (cur().type != RBRAC)
					adv();
				expect(RBRAC);
			}
		} else if (cur().type == EQUALT) {
			adv();
			skip_expr();
		}
		expect(SEMIT);
	} else if (cur().type == IFT) {
		adv();
		skip_expr();
		skip_body_or_stmt();
		while (cur().type == NEWT)
			adv();
		if (cur().type == ELSET) {
			adv();
			skip_body_or_stmt();
		}
	} else if (cur().type == WHILET) {
		adv();
		skip_expr();
		skip_body_or_stmt();
	} else if (cur().type == FORT) {
		adv();
		/* init */
		if (cur().type == WORDT || cur().type == BYTET) {
			adv();
			adv();
			if (cur().type == EQUALT) {
				adv();
				skip_expr();
			}
			expect(SEMIT);
		} else if (cur().type == IDENTT && peek(1).type == EQUALT) {
			adv();
			adv();
			skip_expr();
			expect(SEMIT);
		} else {
			expect(SEMIT);
		}
		/* condition */
		skip_expr();
		expect(SEMIT);
		/* skip update and body */
		while (cur().type != LBRAC && cur().type != NEWT)
			adv();
		skip_body_or_stmt();
	} else if (cur().type == RETT) {
		adv();
		if (cur().type != SEMIT)
			skip_expr();
		expect(SEMIT);
	} else if (cur().type == IDENTT) {
		adv();
		if (cur().type == LPAREN) {
			adv();
			while (cur().type != RPAREN) {
				if (match(COMMAT))
					continue;
				adv();
			}
			expect(RPAREN);
		} else if (cur().type == EQUALT ||
		    cur().type == PLUSEQ || cur().type == MINUSEQ ||
		    cur().type == STAREQ || cur().type == SLASHEQ) {
			adv();
			skip_expr();
		} else if (cur().type == PLUSPLUS || cur().type == MINUSMINUS) {
			adv();
		}
		expect(SEMIT);
	} else {
		fatal(USER_ERR, NULL, "Unexpected token in skip_stmt");
	}
}

/*
 * Skips a body: either a braced block or a single-line statement.
 */
static void
skip_body_or_stmt(void)
{
	if (cur().type == LBRAC) {
		adv();
		while (cur().type != RBRAC) {
			if (cur().type == NEWT) {
				adv();
				continue;
			}
			skip_stmt();
		}
		expect(RBRAC);
	} else if (cur().type == NEWT) {
		adv();
		skip_stmt();
	} else {
		fatal(USER_ERR, NULL, "Expected '{' or newline in skip");
	}
}

static int
get_prec(int type)
{
	switch (type) {
	case PIPEPIPE:              return 1;
	case AMPAMP:                return 2;
	case PIPE:                  return 3;
	case CARET:                 return 4;
	case AMP:                   return 5;
	case EQT: case NEQ:         return 6;
	case LTT: case GTT:
	case LET: case GET:         return 7;
	case LSHIFTT: case RSHIFTT: return 8;
	case PLUST: case MINUST:    return 9;
	case START: case SLASHT: case MODT: return 10;
	default:                    return 0;
	}
}

static void
emit_binop(int op)
{
	switch (op) {
	case PLUST:
		out("\tadd\t%%ebx, %%eax\n");
		break;
	case MINUST:
		out("\tsub\t%%eax, %%ebx\n");
		out("\tmov\t%%ebx, %%eax\n");
		break;
	case START:
		out("\timul\t%%ebx, %%eax\n");
		break;
	case SLASHT:
		out("\tmov\t%%eax, %%ecx\n");
		out("\tmov\t%%ebx, %%eax\n");
		out("\tcdq\n");
		out("\tidiv\t%%ecx\n");
		break;
	case MODT:
		out("\tmov\t%%eax, %%ecx\n");
		out("\tmov\t%%ebx, %%eax\n");
		out("\tcdq\n");
		out("\tidiv\t%%ecx\n");
		out("\tmov\t%%edx, %%eax\n");
		break;
	case LSHIFTT:
		out("\tmov\t%%eax, %%ecx\n");
		out("\tmov\t%%ebx, %%eax\n");
		out("\tshl\t%%cl, %%eax\n");
		break;
	case RSHIFTT:
		out("\tmov\t%%eax, %%ecx\n");
		out("\tmov\t%%ebx, %%eax\n");
		out("\tsar\t%%cl, %%eax\n");
		break;
	case AMP:
		out("\tand\t%%ebx, %%eax\n");
		break;
	case PIPE:
		out("\tor\t%%ebx, %%eax\n");
		break;
	case CARET:
		out("\txor\t%%ebx, %%eax\n");
		break;
	case LTT:
		out("\tcmp\t%%eax, %%ebx\n");
		out("\tsetl\t%%al\n");
		out("\tmovzbl\t%%al, %%eax\n");
		break;
	case GTT:
		out("\tcmp\t%%eax, %%ebx\n");
		out("\tsetg\t%%al\n");
		out("\tmovzbl\t%%al, %%eax\n");
		break;
	case EQT:
		out("\tcmp\t%%eax, %%ebx\n");
		out("\tsete\t%%al\n");
		out("\tmovzbl\t%%al, %%eax\n");
		break;
	case NEQ:
		out("\tcmp\t%%eax, %%ebx\n");
		out("\tsetne\t%%al\n");
		out("\tmovzbl\t%%al, %%eax\n");
		break;
	case LET:
		out("\tcmp\t%%eax, %%ebx\n");
		out("\tsetle\t%%al\n");
		out("\tmovzbl\t%%al, %%eax\n");
		break;
	case GET:
		out("\tcmp\t%%eax, %%ebx\n");
		out("\tsetge\t%%al\n");
		out("\tmovzbl\t%%al, %%eax\n");
		break;
	}
}

/*
 * Evaluates an expression with precedence climbing.
 * All binary operators are left-associative.
 * && and || use short-circuit evaluation.
 */
static void
emit_expr_prec(int min_prec)
{
	int op, prec;

	emit_unary();

	while (get_prec(cur().type) > min_prec) {
		op = cur().type;
		prec = get_prec(op);

		if (op == AMPAMP || op == PIPEPIPE) {
			int false_label = label_count++;
			int true_label = label_count++;

			out("\tcmp\t$0, %%eax\n");
			if (op == AMPAMP)
				out("\tje\t.L%d\n", false_label);
			else
				out("\tjne\t.L%d\n", true_label);

			adv();
			emit_expr_prec(prec + 1);

			out("\tcmp\t$0, %%eax\n");
			if (op == AMPAMP) {
				out("\tje\t.L%d\n", false_label);
				out("\tmov\t$1, %%eax\n");
				out("\tjmp\t.L%d\n", true_label);
			} else {
				out("\tjne\t.L%d\n", true_label);
				out("\tmov\t$0, %%eax\n");
				out("\tjmp\t.L%d\n", false_label);
			}

			if (op == AMPAMP) {
				out(".L%d:\n", false_label);
				out("\tmov\t$0, %%eax\n");
				out(".L%d:\n", true_label);
			} else {
				out(".L%d:\n", false_label);
				out("\tmov\t$1, %%eax\n");
				out(".L%d:\n", true_label);
			}
		} else {
			adv();
			out("\tpush\t%%eax\n");
			emit_expr_prec(prec + 1);
			out("\tpop\t%%ebx\n");
			emit_binop(op);
		}
	}
}

static void
emit_expr(void)
{
	emit_expr_prec(0);
}

static void
emit_unary(void)
{
	char *name;
	int name_len;

	if (cur().type == TILDE) {
		adv();
		emit_unary();
		out("\tnot\t%%eax\n");
		return;
	}

	if (cur().type == BANG) {
		adv();
		emit_unary();
		out("\tcmp\t$0, %%eax\n");
		out("\tsete\t%%al\n");
		out("\tmovzbl\t%%al, %%eax\n");
		return;
	}

	if (cur().type == PLUSPLUS) {
		adv();
		if (cur().type != IDENTT)
			fatal(USER_ERR, NULL, "Expected variable after '++'");
		name = cur().start;
		name_len = cur().length;
		adv();
		emit_load_var(name, name_len);
		out("\tadd\t$1, %%eax\n");
		emit_store_var(name, name_len);
		return;
	}

	if (cur().type == MINUSMINUS) {
		adv();
		if (cur().type != IDENTT)
			fatal(USER_ERR, NULL, "Expected variable after '--'");
		name = cur().start;
		name_len = cur().length;
		adv();
		emit_load_var(name, name_len);
		out("\tsub\t$1, %%eax\n");
		emit_store_var(name, name_len);
		return;
	}

	if (cur().type == LPAREN) {
		adv();
		emit_expr();
		expect(RPAREN);
		return;
	}

	emit_primary();
}

/*
 * Parses a primary expression: number or variable.
 * Result left in %eax.
 */
static int
parse_num(const char *s, int len)
{
	int val = 0;
	const char *end = s + len;

	if (s[0] == '0' && len > 1 && (s[1] == 'x' || s[1] == 'X')) {
		s += 2;
		while (s < end) {
			val *= 16;
			if (*s >= '0' && *s <= '9') val += *s - '0';
			else if (*s >= 'a' && *s <= 'f') val += *s - 'a' + 10;
			else if (*s >= 'A' && *s <= 'F') val += *s - 'A' + 10;
			s++;
		}
	} else if (s[0] == '0' && len > 1 && (s[1] == 'b' || s[1] == 'B')) {
		s += 2;
		while (s < end) {
			val = val * 2 + (*s - '0');
			s++;
		}
	} else {
		while (s < end) {
			val = val * 10 + (*s - '0');
			s++;
		}
	}
	return val;
}

static void
emit_primary(void)
{
	char *name;
	int name_len;

	if (cur().type == NUMT || cur().type == CHARLITT) {
		out("\tmov\t$%d, %%eax\n", parse_num(cur().start, cur().length));
		adv();
		return;
	}

	if (cur().type == IDENTT) {
		name = cur().start;
		name_len = cur().length;
		adv();

		/* postfix ++ */
		if (cur().type == PLUSPLUS) {
			adv();
			emit_load_var(name, name_len);
			out("\tpush\t%%eax\n");
			out("\tadd\t$1, %%eax\n");
			emit_store_var(name, name_len);
			out("\tpop\t%%eax\n");
			return;
		}

		/* postfix -- */
		if (cur().type == MINUSMINUS) {
			adv();
			emit_load_var(name, name_len);
			out("\tpush\t%%eax\n");
			out("\tsub\t$1, %%eax\n");
			emit_store_var(name, name_len);
			out("\tpop\t%%eax\n");
			return;
		}

		emit_load_var(name, name_len);
		return;
	}

	fatal(USER_ERR, NULL, "Expected expression");
}

static void
emit_assignment(void)
{
	char *name;
	int name_len;
	int op;

	name = cur().start;
	name_len = cur().length;
	adv();
	op = cur().type;

	/* postfix ++ / -- */
	if (op == PLUSPLUS || op == MINUSMINUS) {
		adv();
		emit_load_var(name, name_len);
		out("\tpush\t%%eax\n");
		if (op == PLUSPLUS)
			out("\tadd\t$1, %%eax\n");
		else
			out("\tsub\t$1, %%eax\n");
		emit_store_var(name, name_len);
		out("\tpop\t%%eax\n");
		expect(SEMIT);
		return;
	}

	/* plain assignment: x = expr */
	if (op == EQUALT) {
		adv();
		emit_expr();
		emit_store_var(name, name_len);
		expect(SEMIT);
		return;
	}

	/* compound assignment: x += expr, x -= expr, x *= expr, x /= expr */
	adv();
	emit_load_var(name, name_len);
	out("\tpush\t%%eax\n");
	emit_expr();
	out("\tpop\t%%ebx\n");

	/* map compound token to binary op token for emit_binop */
	if (op == PLUSEQ) op = PLUST;
	else if (op == MINUSEQ) op = MINUST;
	else if (op == STAREQ) op = START;
	else if (op == SLASHEQ) op = SLASHT;

	emit_binop(op);
	emit_store_var(name, name_len);
	expect(SEMIT);
}

static void
emit_if(void)
{
	int my_label;

	expect(IFT);
	emit_expr();

	my_label = label_count++;

	out("\tcmp\t$0, %%eax\n");
	out("\tje\t.L%d\n", my_label);

	emit_body_or_stmt();

	while (cur().type == NEWT)
		adv();

	if (cur().type == ELSET) {
		int else_label = label_count++;
		out("\tjmp\t.L%d\n", else_label);
		out(".L%d:\n", my_label);
		adv();
		emit_body_or_stmt();
		out(".L%d:\n", else_label);
	} else {
		out(".L%d:\n", my_label);
	}
}

static void
emit_while(void)
{
	int loop_label;
	int end_label;

	expect(WHILET);

	loop_label = label_count++;
	end_label = label_count++;

	out(".L%d:\n", loop_label);
	emit_expr();

	out("\tcmp\t$0, %%eax\n");
	out("\tje\t.L%d\n", end_label);

	emit_body_or_stmt();

	out("\tjmp\t.L%d\n", loop_label);
	out(".L%d:\n", end_label);
}

static void
emit_for(void)
{
	int loop_label;
	int end_label;
	int update_start;

	expect(FORT);

	/* init */
	if (cur().type == WORDT || cur().type == BYTET) {
		emit_decl((cur().type == WORDT) ? TYPE_WORD : TYPE_BYTE);
	} else if (cur().type == IDENTT && peek(1).type == EQUALT) {
		emit_assignment();
	} else {
		expect(SEMIT);
	}

	/* condition */
	loop_label = label_count++;
	end_label = label_count++;

	out(".L%d:\n", loop_label);
	emit_expr();

	out("\tcmp\t$0, %%eax\n");
	out("\tje\t.L%d\n", end_label);

	expect(SEMIT);

	/* save update tokens position */
	update_start = current;
	while (cur().type != LBRAC && cur().type != NEWT)
		adv();
	/* cur() is now LBRAC or NEWT */

	emit_body_or_stmt();

	/* emit update by temporarily restoring position */
	{
		int saved = current;
		current = update_start;
		while (cur().type != LBRAC && cur().type != NEWT) {
			if (cur().type == IDENTT &&
			    (peek(1).type == EQUALT || peek(1).type == PLUSEQ ||
			     peek(1).type == MINUSEQ || peek(1).type == STAREQ ||
			     peek(1).type == SLASHEQ ||
			     peek(1).type == PLUSPLUS || peek(1).type == MINUSMINUS)) {
				char *uname = cur().start;
				int uname_len = cur().length;
				int uidx = find_local(uname, uname_len);
				int is_global = (uidx < 0) ? find_global(uname, uname_len) : -1;
				int utype;
				int uoffset;

				if (uidx >= 0)
					utype = locals[uidx].type, uoffset = locals[uidx].offset;
				else if (is_global >= 0)
					utype = globals[is_global].type, uoffset = 0;
				else
					fatal(USER_ERR, NULL, "Undefined variable '%.*s'", uname_len, uname);

				adv(); /* skip var name */
				if (cur().type == PLUSPLUS) {
					adv();
					emit_load_var(uname, uname_len);
					out("\tadd\t$1, %%eax\n");
					emit_store_var(uname, uname_len);
				} else if (cur().type == MINUSMINUS) {
					adv();
					emit_load_var(uname, uname_len);
					out("\tsub\t$1, %%eax\n");
					emit_store_var(uname, uname_len);
				} else {
					int uop = cur().type;
					adv();
					if (uop == EQUALT) {
						emit_expr();
	} else if (cur().type == PLUSPLUS || cur().type == MINUSMINUS ||
	    cur().type == NUMT || cur().type == CHARLITT ||
	    cur().type == TILDE || cur().type == BANG ||
	    cur().type == LPAREN) {
		skip_expr();
		expect(SEMIT);
	} else {
						int bop;
						emit_load_var(uname, uname_len);
						out("\tpush\t%%eax\n");
						emit_expr();
						out("\tpop\t%%ebx\n");
						if (uop == PLUSEQ) bop = PLUST;
						else if (uop == MINUSEQ) bop = MINUST;
						else if (uop == STAREQ) bop = START;
						else bop = SLASHT;
						emit_binop(bop);
					}
					emit_store_var(uname, uname_len);
				}
			} else {
				adv();
			}
		}
		current = saved;
	}

	out("\tjmp\t.L%d\n", loop_label);
	out(".L%d:\n", end_label);
}

/*
 * Emits a single statement.
 */
static void
emit_stmt(void)
{
	switch (cur().type) {
	case WORDT:
		emit_decl(TYPE_WORD);
		break;
	case BYTET:
		emit_decl(TYPE_BYTE);
		break;
	case IDENTT:
		if (peek(1).type == EQUALT ||
		    peek(1).type == PLUSEQ || peek(1).type == MINUSEQ ||
		    peek(1).type == STAREQ || peek(1).type == SLASHEQ ||
		    peek(1).type == PLUSPLUS || peek(1).type == MINUSMINUS)
			emit_assignment();
		else
			emit_call();
		break;
	case RETT:
		emit_ret();
		break;
	case IFT:
		emit_if();
		break;
	case WHILET:
		emit_while();
		break;
	case FORT:
		emit_for();
		break;
	case PLUSPLUS:
	case MINUSMINUS:
	case NUMT:
	case CHARLITT:
	case TILDE:
	case BANG:
	case LPAREN:
		emit_expr();
		expect(SEMIT);
		break;
	default:
		fatal(USER_ERR, NULL, "Unexpected token in body");
	}
}

/*
 * Emits a block of statements until a closing brace is found.
 * Used by emit_body_or_stmt when a '{' is encountered.
 */
static void
emit_body(void)
{
	while (cur().type != RBRAC) {
		if (cur().type == NEWT) {
			adv();
			continue;
		}
		emit_stmt();
	}
}

/*
 * Emits either a braced block or a single-line statement.
 * Used by if, while, and for to emit their bodies.
 */
static void
emit_body_or_stmt(void)
{
	if (cur().type == LBRAC) {
		adv();
		emit_body();
		expect(RBRAC);
	} else if (cur().type == NEWT) {
		adv();
		emit_stmt();
	} else {
		fatal(USER_ERR, NULL, "Expected '{' or newline");
	}
}

static void
emit_global_data(int pub)
{
	int vtype;

	if (pub)
		globals[global_count].is_pub = 1;
	else
		globals[global_count].is_pub = 0;

	while (cur().type == WORDT || cur().type == BYTET) {
		vtype = (cur().type == WORDT) ? TYPE_WORD : TYPE_BYTE;
		adv();
		expect(IDENTT);
		globals[global_count].name = tokens[current - 1].start;
		globals[global_count].name_len = tokens[current - 1].length;
		globals[global_count].type = vtype;

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
			out("\t.zero %d\n", globals[global_count].array_size * type_size(vtype));
		} else if (cur().type == EQUALT) {
			adv();
			globals[global_count].is_array = 0;
			globals[global_count].array_size = 0;
			globals[global_count].has_init = 1;
			globals[global_count].init_val = parse_num(cur().start, cur().length);
			adv();
			expect(SEMIT);
			out("%.*s:\n", globals[global_count].name_len, globals[global_count].name);
			if (vtype == TYPE_BYTE)
				out("\t.byte %d\n", globals[global_count].init_val);
			else
				out("\t.long %d\n", globals[global_count].init_val);
		} else {
			globals[global_count].is_array = 0;
			globals[global_count].array_size = 0;
			globals[global_count].has_init = 0;
			expect(SEMIT);
			out("%.*s:\n", globals[global_count].name_len, globals[global_count].name);
			out("\t.zero %d\n", type_size(vtype));
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
		if (cur().type == NEWT) {
			adv();
			continue;
		}
		if (cur().type == WORDT || cur().type == BYTET) {
			int vtype;
			char *lname;
			int lname_len;
			vtype = (cur().type == WORDT) ? TYPE_WORD : TYPE_BYTE;
			adv();
			lname = cur().start;
			lname_len = cur().length;
			adv(); /* skip variable name */
			if (cur().type == LBRAT) {
				adv();
				stack_size += atoi(cur().start) * type_size(vtype);
				adv();
				expect(RBRAT);
				locals[local_count].name = lname;
				locals[local_count].name_len = lname_len;
				locals[local_count].type = vtype;
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
				stack_size += type_size(vtype);
				locals[local_count].name = lname;
				locals[local_count].name_len = lname_len;
				locals[local_count].type = vtype;
				locals[local_count].offset = -stack_size;
				local_count++;
				skip_expr();
				expect(SEMIT);
			} else {
				stack_size += type_size(vtype);
				locals[local_count].name = lname;
				locals[local_count].name_len = lname_len;
				locals[local_count].type = vtype;
				locals[local_count].offset = -stack_size;
				local_count++;
				expect(SEMIT);
			}
		} else {
			skip_stmt();
		}
	}

	/* align stack to 4 bytes */
	stack_size = (stack_size + 3) & ~3;

	if (stack_size > 0)
		out("\tsub\t$%d, %%esp\n", stack_size);

	/* second pass: emit code */
	current = body_start;
	emit_body();

	expect(RBRAC);

	out("\tmov\t%%ebp, %%esp\n");
	out("\tpop\t%%ebp\n");
	out("\tret\n");
}

static void
emit_decl(int type)
{
	char *name;
	int name_len;
	int idx;
	int offset;

	if (type == TYPE_WORD)
		expect(WORDT);
	else
		expect(BYTET);
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
				arr_off += type_size(type);
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

		emit_expr();
		if (type == TYPE_BYTE)
			out("\tmovb\t%%al, %d(%%ebp)\n", offset);
		else
			out("\tmov\t%%eax, %d(%%ebp)\n", offset);
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
	char *arg_starts[32];
	int arg_lengths[32];

	fn_name = cur().start;
	fn_len = cur().length;
	adv();
	expect(LPAREN);

	while (cur().type != RPAREN) {
		if (match(COMMAT))
			continue;
		arg_starts[args_count] = cur().start;
		arg_lengths[args_count] = cur().length;
		args_count++;
		adv();
	}
	expect(RPAREN);
	expect(SEMIT);

	for (i = args_count - 1; i >= 0; i--) {
		if (string_count > 0 && find_string(arg_starts[i], arg_lengths[i]) >= 0)
			out("\tpush\t$str%d\n", find_string(arg_starts[i], arg_lengths[i]));
		else if (find_local(arg_starts[i], arg_lengths[i]) >= 0) {
			int alidx = find_local(arg_starts[i], arg_lengths[i]);
			if (locals[alidx].type == TYPE_BYTE) {
				out("\tmovzbl\t%d(%%ebp), %%eax\n", locals[alidx].offset);
				out("\tpush\t%%eax\n");
			} else {
				out("\tpush\t%d(%%ebp)\n", locals[alidx].offset);
			}
		} else if (find_global(arg_starts[i], arg_lengths[i]) >= 0) {
			int agidx = find_global(arg_starts[i], arg_lengths[i]);
			if (globals[agidx].type == TYPE_BYTE) {
				out("\tmovzbl\t%.*s, %%eax\n", arg_lengths[i], arg_starts[i]);
				out("\tpush\t%%eax\n");
			} else {
				out("\tpush\t%.*s\n", arg_lengths[i], arg_starts[i]);
			}
		} else if (arg_lengths[i] == 1 && *arg_starts[i] >= '0' && *arg_starts[i] <= '9')
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
	expect(RETT);

	if (cur().type != SEMIT) {
		emit_expr();
	}

	expect(SEMIT);
}

static void
skip_fn_body(void)
{
	int depth = 0;

	while (cur().type != EOFT) {
		if (cur().type == LBRAC)
			depth++;
		else if (cur().type == RBRAC) {
			if (depth == 0) return;
			if (--depth == 0) { adv(); return; }
		}
		adv();
	}
}

void codegen(int fd)
{
	out_fd = fd;
	current = 0;
	global_count = 0;
	string_count = 0;
	label_count = 0;

	collect_strings();

	/* first pass: emit globals and strings in .data */
	out("\t.data\n");
	while (cur().type != EOFT) {
		if (cur().type == NEWT) {
			adv();
			continue;
		}
		if (cur().type == PUBT) {
			adv();
			if (cur().type == WORDT || cur().type == BYTET)
				emit_global_data(1);
			else
				skip_fn_body();
		} else if (cur().type == WORDT || cur().type == BYTET) {
			emit_global_data(0);
		} else if (cur().type == FNT) {
			adv();
			skip_fn_body();
		} else {
			adv();
		}
	}
	emit_strings();

	/* second pass: emit functions in .text */
	current = 0;
	out("\t.text\n");
	while (cur().type != EOFT) {
		if (cur().type == NEWT) {
			adv();
			continue;
		}
		if (cur().type == PUBT) {
			adv();
			if (cur().type == FNT) {
				adv();
				emit_function(1);
			} else {
				adv();
			}
		} else if (cur().type == FNT) {
			adv();
			emit_function(0);
		} else {
			adv();
		}
	}
}
