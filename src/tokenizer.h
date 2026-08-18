/* tokenizer.h
 * tokenizer module for zc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>

#define MAX_TOKENS 1024

enum token_type {
	WORDT, BYTET, PUBT, FNT, RETT,
	IFT, ELSET, WHILET, FORT,
	IDENTT, NUMT, STRT, CHARLITT,
	LBRAT, RBRAT,
	LBRAC, RBRAC,
	LPAREN, RPAREN,
	COMMAT, EQUALT, SEMIT,
	LTT, GTT, EQT, NEQ, LET, GET,
	LSHIFTT, RSHIFTT,
	PLUST, MINUST, START, SLASHT,
	AMP, PIPE, CARET,
	TILDE, BANG,
	AMPAMP, PIPEPIPE,
	EOFT
};

struct token {
	int type;
	char *start;
	int length;
};

extern struct token tokens[MAX_TOKENS];
extern int token_count;

bool is_alpha(char c);
bool is_digit(char c);
bool is_alnum(char c);

#endif
