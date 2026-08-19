/*
 * parser.c
 * parser and codegen module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include "debug.h"
#include "tokenizer.h"
#include "parser.h"

enum visi {
	PUB, PRIV
};

static int pos;

static bool is(enum token_type t);
static bool accept(enum token_type t);
static void expect(enum token_type t);
static void emit(FILE *, const char *fmt, ...);
static void parse_var(enum visi, FILE *o);
static void parse_fn(void);
static int num_val(void);

/*
 * Parses the tokenized source and emits assembly code for it.
 * Currently only global variables are supported, they are
 * emitted into the .data section. Function definitions are
 * not implemented yet.
 */
void parse(FILE *out)
{
	pos = 0;

	emit(out, "\t.data\n");

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
				parse_fn();
				continue;
			}
			fatal(USER_ERR, NULL, "Expected setw, setb or fn after pub!");
		}

		if (is(FNT)) {
			parse_fn();
			continue;
		}

		if (is(SETWT) || is(SETBT)) {
			parse_var(PRIV, out);
			continue;
		}

		fatal(USER_ERR, NULL, "Unexpected token!");
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
	int first = 1;

	expect(word ? SETWT : SETBT);
	accept(MULT);

	expect(IDT);
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
		int flt = 0;
		int j = pos + 1;

		while (tokens[j].type != RBKT) {
			if (tokens[j].type == FLOATT)
				flt = 1;
			j++;
		}
		if (flt && !word)
			fatal(USER_ERR, NULL, "Float in a byte array!");
		emit(out, flt ? "\t.float\t" : (word ? "\t.long\t" : "\t.byte\t"));
		pos++;
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
 * Parses a function definition. Not implemented yet.
 */
static void
parse_fn(void)
{
	fatal(USER_ERR, NULL, "Functions not implemented yet!");
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

	/* TODO floats 0.XX... */

	return val;
}
