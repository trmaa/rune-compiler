/*
 * var.c
 * global variables and literal data module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <stdio.h>
#include "debug.h"
#include "tokenizer.h"
#include "parser.h"
#include "symtab.h"
#include "var.h"

static void list_values(FILE *out);
static int list_has_float(int p);
static int list_has_char(int p);

/*
 * Parses a global variable declaration and emits it.
 * Grammar: (setw|setb) [*] name [dims...] [= init]
 * Init can be a number, a char, a float, an array literal, a string
 * or an address (&name). Dims are only allowed without init.
 * Floats only fit in words, a float initializer in a setw is
 * emitted as .float.
 */
void
parse_var(enum visi vis, FILE *out)
{
	bool word = is(SETWT);
	int sz = 1;
	int ptr;

	sec_data(out);
	expect(word ? SETWT : SETBT);
	ptr = accept(MULT);

	expect(IDT);
	sym_register(GLO, tokens[pos - 1].start, tokens[pos - 1].length, 0, ptr);
	if (vis == PUB)
		emit(out, "\t.globl\t%.*s\n", tokens[pos - 1].length, tokens[pos - 1].start);
	emit(out, "%.*s:\n", tokens[pos - 1].length, tokens[pos - 1].start);

	/* dimensions */
	while (accept(LBCT)) {
		int v;

		if (!is(INTT))
			fatal(USER_ERR, NULL, "Expected array dimension!");
		v = num_val(&tokens[pos]);
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
			emit(out, word ? "\t.long\t%d\n" : "\t.byte\t%d\n",
			     num_val(&tokens[pos]));
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
 * Registers a string literal into the .data section and
 * returns its label number.
 */
int
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
int
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
			emit(out, "%d", num_val(&tokens[pos]));
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