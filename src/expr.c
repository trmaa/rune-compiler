/*
 * expr.c
 * expression codegen module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "tokenizer.h"
#include "parser.h"
#include "symtab.h"
#include "expr.h"

static char obuf[64];

void swap(int *a, int *b);

static void eval_muldiv(void);
static void eval_atom(const char *reg);
static int chain_end(void);
static int atom_len(int p);

/*
 * Evaluates an addition/subtraction expression into the stack
 * slot at the given offset. The right side is evaluated first
 * and stored into the slot, then the left side is added to it.
 * The left side is found by scanning the token stream for the
 * operator that follows the first multiplication chain.
 */
void
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
			emit(code, "\tmov\t$0x%x, %%%s\n", num_val(&tokens[pos]), reg);
		else if (sign < 0)
			emit(code, "\tmov\t$-%d, %%%s\n", num_val(&tokens[pos]), reg);
		else
			emit(code, "\tmov\t$%d, %%%s\n", num_val(&tokens[pos]), reg);
		pos++;
	} else if (is(IDT)) {
		struct sym *s = sym_find(tokens[pos].start, tokens[pos].length);

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
const char *
atom_operand(void)
{
	int sign = 1;

	if (is(SUBT) && tokens[pos + 1].type == INTT) {
		pos++;
		sign = -1;
	}

	if (is(INTT) || is(CHART)) {
		if (is(CHART))
			snprintf(obuf, sizeof obuf, "$0x%x", num_val(&tokens[pos]));
		else if (sign < 0)
			snprintf(obuf, sizeof obuf, "$-%d", num_val(&tokens[pos]));
		else
			snprintf(obuf, sizeof obuf, "$%d", num_val(&tokens[pos]));
		pos++;
	} else if (is(IDT)) {
		struct sym *s = sym_find(tokens[pos].start, tokens[pos].length);

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

int
next_is_arith(void)
{
	enum token_type t = tokens[pos + 1].type;

	return t == ADDT || t == SUBT || t == MULT || t == DIVT || t == MODT;
}

int
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