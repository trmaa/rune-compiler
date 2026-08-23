/*
 * parser.c
 * parser driver and codegen utilities for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <stdio.h>
#include <stdarg.h>
#include "debug.h"
#include "tokenizer.h"
#include "parser.h"
#include "symtab.h"
#include "var.h"
#include "stmt.h"

int pos;
FILE *code;
FILE *out_cur;
int has_fn, data_used, str_cnt, arr_cnt;

static int lab_cnt;

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
	out_cur = out;
	has_fn = 0;
	data_used = 0;
	str_cnt = 0;
	arr_cnt = 0;
	lab_cnt = 0;
	sym_reset();

	while (!is(EOFT)) {
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

bool is(enum token_type t)
{
	return tokens[pos].type == t;
}

bool accept(enum token_type t)
{
	if (is(t)) {
		pos++;
		return true;
	}
	return false;
}

void expect(enum token_type t)
{
	if (!accept(t))
		fatal(USER_ERR, NULL, "Expected token %s but got %s!", t_name(t),
		      t_name(tokens[pos].type));
}

void emit(FILE *out, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(out, fmt, args);
	va_end(args);
}

/* Emits the .data section header, only once. */
void sec_data(FILE *out)
{
	if (!data_used) {
		data_used = 1;
		emit(out, "\t.data\n");
	}
}

/* Returns a fresh jump label number. */
int new_label(void)
{
	return lab_cnt++;
}
