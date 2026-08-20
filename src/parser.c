/*
 * parser.c
 * parser and codegen module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "debug.h"
#include "tokenizer.h"
#include "parser.h"

enum visi {
	PUB, PRIV
};

enum sym_kind {
	LOC, GLO
};

struct sym {
	char *start;
	int length;
	int kind;
	int off;
};

struct argdesc {
	int kind;
	int lab;
	char text[64];
};

static int pos;
static FILE *code;
static int has_fn, data_used, str_cnt, arr_cnt;
static struct sym syms[128];
static int sym_cnt, loc_cnt;
static char obuf[64];

static void sec_data(FILE *o);
static void reg_sym(int kind, char *start, int length, int off);
static const char *atom_operand(void);
static struct sym *find_sym(char *start, int length);
static int atom_len(int p);
static void list_values(FILE *o);
static int list_has_float(int p);
static int list_has_char(int p);
static bool is_lit(void);
static bool is(enum token_type t);
static bool accept(enum token_type t);
static void expect(enum token_type t);
static void emit(FILE *, const char *fmt, ...);
static void parse_var(enum visi, FILE *o);
static void parse_fn(enum visi, FILE *o);
static void parse_stmts(FILE *o);
static void parse_stmt(FILE *o);
static void parse_call(FILE *o);
static void parse_var_local(FILE *o);
static void parse_ret(void);
static void eval_expr(int off);
static void eval_muldiv(void);
static void eval_atom(const char *reg);
static int chain_end(void);
static bool next_is_arith(void);
static int reg_str(FILE *o);
static int reg_arr(FILE *o);
static int num_val(void);

/*
 * Parses the tokenized source and emits assembly code for it.
 * Global variables and literals are emitted into the .data
 * section, functions are accumulated in a temporary file and
 * written as the .text section at the end.
 */
void parse(FILE *out)
{
	pos = 0;
	code = NULL;
	has_fn = 0;
	data_used = 0;
	str_cnt = 0;
	arr_cnt = 0;
	sym_cnt = 0;
	loc_cnt = 0;

	for (;;) {
		while (is(NEWT) || is(SEMIT))
			pos++;

		if (is(EOFT))
			break;

		if (accept(PUBT)) {
			if (is(SETWT) || is(SETBT)) {
				parse_var(PUB, out);
				continue;
			}
			if (is(FNT)) {
				parse_fn(PUB, out);
				continue;
			}
			fatal(USER_ERR, NULL, "Expected setw, setb or fn after pub!");
		}

		if (is(FNT)) {
			parse_fn(PRIV, out);
			continue;
		}

		if (is(SETWT) || is(SETBT)) {
			parse_var(PRIV, out);
			continue;
		}

		fatal(USER_ERR, NULL, "Unexpected token!");
	}

	if (has_fn) {
		char buf[1024];
		int n;

		emit(out, data_used ? "\n\t.text\n" : "\t.text\n");
		rewind(code);
		while ((n = (int)fread(buf, 1, sizeof buf, code)) > 0)
			fwrite(buf, 1, (size_t)n, out);
		fclose(code);
	}
}

/*
 * Parses a global variable declaration and emits it.
 * Grammar: (setw|setb) [*] name [dims...] [= init]
 * Init can be a number, a char, a float, an array literal, a string
 * or an address (&name). Dims are only allowed without init.
 * Floats only fit in words, a float initializer in a setw is
 * emitted as .float.
 */
static void
parse_var(enum visi vis, FILE *out)
{
	bool word = is(SETWT);
	int sz = 1;

	sec_data(out);
	expect(word ? SETWT : SETBT);
	accept(MULT);

	expect(IDT);
	reg_sym(GLO, tokens[pos - 1].start, tokens[pos - 1].length, 0);
	if (vis == PUB)
		emit(out, "\t.globl\t%.*s\n", tokens[pos - 1].length, tokens[pos - 1].start);
	emit(out, "%.*s:\n", tokens[pos - 1].length, tokens[pos - 1].start);

	/* dimensions */
	while (accept(LBCT)) {
		int v;

		if (!is(INTT))
			fatal(USER_ERR, NULL, "Expected array dimension!");
		v = num_val();
		pos++;
		expect(RBCT);
		sz *= v;
	}

	if (!accept(EQT)) {
		if (sz > 1)
			emit(out, "\t.zero\t%d\n", sz * (word ? 4 : 1));
		else
			emit(out, word ? "\t.long\t0\n" : "\t.byte\t0\n");
		return;
	}

	/* initializer */
	if (is(INTT) || is(CHART) || is(FLOATT)) {
		if (sz > 1)
			fatal(USER_ERR, NULL, "Array with scalar initializer!");
		if (is(FLOATT)) {
			if (!word)
				fatal(USER_ERR, NULL, "Float initializer in a byte variable!");
			emit(out, "\t.float\t%.*s\n", tokens[pos].length, tokens[pos].start);
		} else {
			emit(out, word ? "\t.long\t%d\n" : "\t.byte\t%d\n", num_val());
		}
		pos++;
	} else if (is(LBKT)) {
		int flt = list_has_float(pos);

		if (flt && !word)
			fatal(USER_ERR, NULL, "Float in a byte array!");
		emit(out, flt ? "\t.float\t" : (word ? "\t.long\t" : "\t.byte\t"));
		list_values(out);
	} else if (is(STRT)) {
		if (sz > 1)
			fatal(USER_ERR, NULL, "Array with string initializer!");
		emit(out, "\t.string\t\"%.*s\"\n", tokens[pos].length, tokens[pos].start);
		pos++;
	} else if (accept(BANDT)) {
		expect(IDT);
		emit(out, "\t.long\t%.*s\n", tokens[pos - 1].length, tokens[pos - 1].start);
	} else {
		fatal(USER_ERR, NULL, "Invalid initializer!");
	}
}

/*
 * Parses a function definition and emits its code into the
 * .text section (accumulated in a temporary file). The first
 * function creates that temporary file.
 */
static void
parse_fn(enum visi vis, FILE *out)
{
	expect(FNT);
	expect(IDT);

	if (!has_fn) {
		code = tmpfile();
		has_fn = 1;
	}

	if (vis == PUB)
		emit(code, "\t.globl\t%.*s\n", tokens[pos - 1].length, tokens[pos - 1].start);
	emit(code, "%.*s:\n", tokens[pos - 1].length, tokens[pos - 1].start);
	emit(code, "\tpush\t%%ebp\n\tmov\t%%esp, %%ebp\n\n");

	loc_cnt = 0;

	expect(LBKT);
	parse_stmts(out);
	expect(RBKT);

	emit(code, "\tmov\t%%ebp, %%esp\n\tpop\t%%ebp\n\tret\n");
}

/*
 * Parses the statements of a function body until the closing
 * brace. Statements are separated by newlines, semicolons are
 * optional and ignored.
 */
static void
parse_stmts(FILE *out)
{
	for (;;) {
		while (is(NEWT) || is(SEMIT))
			pos++;

		if (is(RBKT))
			break;
		if (is(EOFT))
			fatal(USER_ERR, NULL, "Unexpected end of file inside a function!");

		parse_stmt(out);
	}
}

/*
 * Parses a single statement. Currently function calls,
 * variable declarations and return statements are supported.
 */
static void
parse_stmt(FILE *out)
{
	if (is(IDT) && tokens[pos + 1].type == LPT) {
		parse_call(out);
		return;
	}

	if (is(SETWT) || is(SETBT)) {
		parse_var_local(out);
		return;
	}

	if (is(RETT)) {
		parse_ret();
		return;
	}

	fatal(USER_ERR, NULL, "Statement not implemented yet!");
}

/*
 * Parses a local variable declaration and emits the stack
 * allocation plus the initialization. A bare literal or
 * variable is stored directly into the slot, more complex
 * initializers are evaluated by the expression codegen.
 * Byte variables use the same 4-byte slot but are stored
 * with movb.
 */
static void
parse_var_local(FILE *out)
{
	struct token *name;
	bool word = is(SETWT);
	int off;

	(void)out;
	expect(is(SETBT) ? SETBT : SETWT);
	accept(MULT);

	expect(IDT);
	name = &tokens[pos - 1];
	off = -(loc_cnt + 1) * 4;
	reg_sym(LOC, name->start, name->length, off);
	loc_cnt++;

	emit(code, "\tsub\t$4, %%esp\n");

	if (accept(EQT)) {
		if (is_lit() && !next_is_arith()) {
			/* literal */
			int sign = 1;

			if (is(SUBT)) {
				pos++;
				sign = -1;
			}
			if (is(CHART))
				emit(code, word ? "\tmov\t$0x%x, %d(%%ebp)\n"
					: "\tmovb\t$0x%x, %d(%%ebp)\n",
					num_val(), off);
			else if (sign < 0)
				emit(code, word ? "\tmov\t$-%d, %d(%%ebp)\n"
					: "\tmovb\t$-%d, %d(%%ebp)\n",
					num_val(), off);
			else
				emit(code, word ? "\tmov\t$%d, %d(%%ebp)\n"
					: "\tmovb\t$%d, %d(%%ebp)\n",
					num_val(), off);
			pos++;
		} else if (is(IDT) && !next_is_arith() &&
			   tokens[pos + 1].type != LPT) {
			/* copy */
			struct sym *src = find_sym(tokens[pos].start, tokens[pos].length);

			pos++;
			if (src->kind == LOC)
				emit(code, word
					? "\tmov\t%d(%%ebp), %%eax\n"
					: "\tmovb\t%d(%%ebp), %%al\n", src->off);
			else
				emit(code, word
					? "\tmov\t%.*s, %%eax\n"
					: "\tmovb\t%.*s, %%al\n",
					src->length, src->start);
			emit(code, word
				? "\tmov\t%%eax, %d(%%ebp)\n"
				: "\tmovb\t%%al, %d(%%ebp)\n", off);
		} else if (is(IDT) && tokens[pos + 1].type == LPT) {
			fatal(USER_ERR, NULL, "Calls as initializers not implemented yet!");
		} else if (!word) {
			fatal(USER_ERR, NULL, "Byte expressions not implemented yet!");
		} else {
			eval_expr(off);
		}
	}

	emit(code, "\n");
}

/*
 * Parses a return statement. The value is loaded into %eax.
 * Literals, variables and add/subtract expressions are
 * supported. The left side goes into %eax and every operator
 * applies its right side directly as a memory operand.
 */
static void
parse_ret(void)
{
	expect(RETT);

	emit(code, "\tmov\t%s, %%eax\n", atom_operand());
	while (is(ADDT) || is(SUBT)) {
		bool add = accept(ADDT);

		if (!add)
			pos++;
		emit(code, add ? "\tadd\t%s, %%eax\n" : "\tsub\t%s, %%eax\n",
		     atom_operand());
	}

	emit(code, "\n");
}

/*
 * Evaluates an addition/subtraction expression into the stack
 * slot at the given offset. The right side is evaluated first
 * and stored into the slot, then the left side is added to it.
 * The left side is found by scanning the token stream for the
 * operator that follows the first multiplication chain.
 */
static void
eval_expr(int off)
{
	int opos, op, mark, end;

	opos = chain_end();
	if (opos == -1) {
		eval_muldiv();
		emit(code, "\tmov\t%%eax, %d(%%ebp)\n", off);
		return;
	}

	op = tokens[opos].type;
	mark = pos;

	pos = opos + 1;
	eval_expr(off);
	end = pos;

	pos = mark;
	eval_muldiv();
	if (op == ADDT)
		emit(code, "\tadd\t%%eax, %d(%%ebp)\n", off);
	else
		emit(code, "\tsub\t%%eax, %d(%%ebp)\n\tneg\t%d(%%ebp)\n", off, off);
	pos = end;
}

/*
 * Walks the token stream from the current position over the
 * first multiplication chain (atom [muldiv op atom]*) and
 * returns the position of the addition/subtraction operator
 * that follows it, or -1 if there is none.
 */
static int
chain_end(void)
{
	int p, len;

	if ((len = atom_len(pos)) == 0)
		return -1;
	p = pos + len;
	while (tokens[p].type == MULT || tokens[p].type == DIVT ||
	       tokens[p].type == MODT) {
		p++;
		if ((len = atom_len(p)) == 0)
			return -1;
		p += len;
	}

	if (tokens[p].type == ADDT || tokens[p].type == SUBT)
		return p;
	return -1;
}

/*
 * Evaluates a multiplication/division chain into %eax.
 * The divisor (or multiplier) is loaded into %ebx.
 */
static void
eval_muldiv(void)
{
	int tmp_pos;

	eval_atom("eax");
	while (is(MULT) || is(DIVT) || is(MODT)) {
		if (is(DIVT) || is(MODT)) {
			emit(code, "\txor\t%%edx, %%edx\n");
			tmp_pos = pos;
			pos++;
			eval_atom("ebx");
			emit(code, "\tdiv\t%%ebx\n");
			swap(&pos, &tmp_pos);
			if (is(MODT))
				emit(code, "\tmov\t%%edx, %%eax\n");
			swap(&pos, &tmp_pos);
		} else {
			pos++;
			eval_atom("ebx");
			emit(code, "\tmul\t%%ebx\n");
		}
	}
}

/*
 * Evaluates a single operand (literal, local or global
 * variable) into the given register. Chars are emitted in
 * hexadecimal, negative literals keep their sign.
 */
static void
eval_atom(const char *reg)
{
	int sign = 1;

	if (is(SUBT) && tokens[pos + 1].type == INTT) {
		pos++;
		sign = -1;
	}

	if (is(INTT) || is(CHART)) {
		if (is(CHART))
			emit(code, "\tmov\t$0x%x, %%%s\n", num_val(), reg);
		else if (sign < 0)
			emit(code, "\tmov\t$-%d, %%%s\n", num_val(), reg);
		else
			emit(code, "\tmov\t$%d, %%%s\n", num_val(), reg);
		pos++;
	} else if (is(IDT)) {
		struct sym *s = find_sym(tokens[pos].start, tokens[pos].length);

		pos++;
		if (s->kind == LOC)
			emit(code, "\tmov\t%d(%%ebp), %%%s\n", s->off, reg);
		else
			emit(code, "\tmov\t%.*s, %%%s\n", s->length, s->start, reg);
	} else if (is(FLOATT)) {
		fatal(USER_ERR, NULL, "Float expressions not implemented yet!");
	} else {
		fatal(USER_ERR, NULL, "Expression not implemented yet!");
	}
}

/*
 * Returns the textual operand of the current atom and
 * consumes it. Used for pushes and return arithmetic.
 */
static const char *
atom_operand(void)
{
	int sign = 1;

	if (is(SUBT) && tokens[pos + 1].type == INTT) {
		pos++;
		sign = -1;
	}

	if (is(INTT) || is(CHART)) {
		if (is(CHART))
			snprintf(obuf, sizeof obuf, "$0x%x", num_val());
		else if (sign < 0)
			snprintf(obuf, sizeof obuf, "$-%d", num_val());
		else
			snprintf(obuf, sizeof obuf, "$%d", num_val());
		pos++;
	} else if (is(IDT)) {
		struct sym *s = find_sym(tokens[pos].start, tokens[pos].length);

		pos++;
		if (s->kind == LOC)
			snprintf(obuf, sizeof obuf, "%d(%%ebp)", s->off);
		else
			snprintf(obuf, sizeof obuf, "%.*s", s->length, s->start);
	} else {
		fatal(USER_ERR, NULL, "Expression as operand not implemented yet!");
	}
	return obuf;
}

static bool
next_is_arith(void)
{
	enum token_type t = tokens[pos + 1].type;

	return t == ADDT || t == SUBT || t == MULT || t == DIVT || t == MODT;
}

static bool
is_lit(void)
{
	return is(INTT) || is(CHART) ||
	       (is(SUBT) && tokens[pos + 1].type == INTT);
}

/* Number of tokens an atom takes starting at position p. */
static int
atom_len(int p)
{
	if (tokens[p].type == INTT || tokens[p].type == CHART ||
	    tokens[p].type == IDT)
		return 1;
	if (tokens[p].type == SUBT && tokens[p + 1].type == INTT)
		return 2;
	return 0;
}

/*
 * Finds a variable by name, preferring the local table over
 * the global one. Fatals if the variable does not exist.
 */
static struct sym *
find_sym(char *start, int length)
{
	int i;

	for (i = 0; i < sym_cnt; i++)
		if (syms[i].kind == LOC && syms[i].length == length &&
		    !memcmp(syms[i].start, start, (size_t)length))
			return &syms[i];

	for (i = 0; i < sym_cnt; i++)
		if (syms[i].kind == GLO && syms[i].length == length &&
		    !memcmp(syms[i].start, start, (size_t)length))
			return &syms[i];

	fatal(USER_ERR, NULL, "Unknown variable '%.*s'!", length, start);
	return NULL;
}

static void
reg_sym(int kind, char *start, int length, int off)
{
	if (sym_cnt >= 128)
		fatal(USER_ERR, NULL, "Too many variables!");

	syms[sym_cnt].start = start;
	syms[sym_cnt].length = length;
	syms[sym_cnt].kind = kind;
	syms[sym_cnt].off = off;
	sym_cnt++;
}

/*
 * Parses a function call with its arguments and emits the
 * pushes (in reverse order), the call and the stack cleanup.
 * Strings and array literals are emitted to the .data section
 * as strN/arrN labels, every other argument is pushed as a
 * raw operand (literal or variable).
 */
static void
parse_call(FILE *out)
{
	enum { ASTR, AARR, AOP };
	struct argdesc args[32];
	int n = 0, i;
	struct token *name = &tokens[pos];

	pos++; /* IDT */
	expect(LPT);

	while (!is(RPT)) {
		if (n >= 32)
			fatal(USER_ERR, NULL, "Too many arguments!");
		if (is(STRT)) {
			args[n].kind = ASTR;
			args[n].lab = reg_str(out);
			pos++;
		} else if (is(LBKT)) {
			args[n].kind = AARR;
			args[n].lab = reg_arr(out);
		} else {
			args[n].kind = AOP;
			snprintf(args[n].text, sizeof args[n].text, "%s",
				 atom_operand());
		}
		n++;
		if (accept(COMT))
			continue;
		break;
	}
	expect(RPT);

	for (i = n - 1; i >= 0; i--) {
		switch (args[i].kind) {
		case ASTR:
			emit(code, "\tpush\t$str%d\n", args[i].lab);
			break;
		case AARR:
			emit(code, "\tpush\t$arr%d\n", args[i].lab);
			break;
		default:
			emit(code, "\tpush\t%s\n", args[i].text);
			break;
		}
	}
	emit(code, "\tcall\t%.*s\n", name->length, name->start);
	if (n > 0)
		emit(code, "\tadd\t$%d, %%esp\n", n * 4);
	emit(code, "\n");
}

/*
 * Registers a string literal into the .data section and
 * returns its label number.
 */
static int
reg_str(FILE *out)
{
	sec_data(out);
	emit(out, "str%d:\n\t.string\t\"%.*s\"\n", str_cnt,
	     tokens[pos].length, tokens[pos].start);

	return str_cnt++;
}

/*
 * Registers an array literal into the .data section and
 * returns its label number. All-int literals become .long,
 * literals with floats become .float and literals with
 * chars become .byte.
 */
static int
reg_arr(FILE *out)
{
	int flt = list_has_float(pos), byt = list_has_char(pos);

	sec_data(out);
	emit(out, "arr%d:\n", arr_cnt);
	if (flt)
		emit(out, "\t.float\t");
	else if (byt)
		emit(out, "\t.byte\t");
	else
		emit(out, "\t.long\t");
	list_values(out);

	return arr_cnt++;
}

/* Number of float values in the array literal at position p. */
static int
list_has_float(int p)
{
	int n = 0;

	while (tokens[p].type != RBKT) {
		if (tokens[p].type == FLOATT)
			n++;
		p++;
	}
	return n;
}

/* Number of char values in the array literal at position p. */
static int
list_has_char(int p)
{
	int n = 0;

	while (tokens[p].type != RBKT) {
		if (tokens[p].type == CHART)
			n++;
		p++;
	}
	return n;
}

/*
 * Emits the values of the array literal at the current
 * position after the callers directive, consuming up to and
 * including the closing bracket.
 */
static void
list_values(FILE *out)
{
	int first = 1;

	pos++; /* LBKT */
	while (!is(RBKT)) {
		if (!first)
			emit(out, ", ");
		first = 0;
		if (is(INTT) || is(CHART)) {
			emit(out, "%d", num_val());
			pos++;
		} else if (is(FLOATT)) {
			emit(out, "%.*s", tokens[pos].length, tokens[pos].start);
			pos++;
		} else {
			fatal(USER_ERR, NULL, "Invalid value in array literal!");
		}
		if (accept(COMT))
			continue;
		break;
	}
	expect(RBKT);
	emit(out, "\n");
}

/* Emits the .data section header, only once. */
static void
sec_data(FILE *out)
{
	if (!data_used) {
		data_used = 1;
		emit(out, "\t.data\n");
	}
}

static bool
is(enum token_type t)
{
	return tokens[pos].type == t;
}

static bool
accept(enum token_type t)
{
	if (is(t)) {
		pos++;
		return true;
	}
	return false;
}

static void
expect(enum token_type t)
{
	if (!accept(t))
		fatal(USER_ERR, NULL, "Expected token %d but got %d!", t,
		      tokens[pos].type);
}

static void
emit(FILE *out, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(out, fmt, args);
	va_end(args);
}

/*
 * Numeric value of the current token. Handles decimal,
 * hexadecimal (0x..) and binary (0b..) literals.
 */
static int
num_val(void)
{
	char *s = tokens[pos].start;
	int len = tokens[pos].length;
	int base = 10, val = 0, i = 0;

	if (len > 2 && s[0] == '0') {
		if (s[1] == 'x' || s[1] == 'X')
			base = 16, i = 2;
		else if (s[1] == 'b' || s[1] == 'B')
			base = 2, i = 2;
	}

	for (; i < len; i++) {
		int d;

		if (s[i] >= '0' && s[i] <= '9')
			d = s[i] - '0';
		else if (s[i] >= 'a' && s[i] <= 'f')
			d = s[i] - 'a' + 10;
		else if (s[i] >= 'A' && s[i] <= 'F')
			d = s[i] - 'A' + 10;
		else
			break;
		val = val * base + d;
	}

	return val;
}
