/*
 * parser.h
 * parser and codegen module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>
#include "tokenizer.h"

enum visi {
	PUB, PRIV
};

/* shared codegen state */
extern int pos;
extern FILE *code;
extern FILE *out_cur;
extern int has_fn, data_used, str_cnt, arr_cnt;

/* token walking and output helpers */
bool is(enum token_type t);
bool accept(enum token_type t);
void expect(enum token_type t);
void emit(FILE *out, const char *fmt, ...);
void sec_data(FILE *out);
int new_label(void);

void parse(FILE *out);

#endif