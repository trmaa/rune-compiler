/*
 * tokenizer.h
 * tokenizer module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>

#define MAX_TOKENS 1024

enum token_type {
	/* data types - and declarations */
	SETWT, SETBT,
	PUBT, FNT, IDT, EQT,
	/* literals */
	STRT, INTT, FLOATT, CHART,
	/* symbols */
	LPT, RPT,
	LBCT, RBCT,
	LBKT, RBKT,
	COMT, DOTT, SEMIT,
	/* cmp */
	LTT, GTT,
	LET, GET,
	EQQT,
	/* logic */
	NOTT, ANDT, ORT,
	BNOTT, BANDT, BORT, XORT,
	NOTIT, BNOTIT, BANDIT, BORIT, XORIT,
	SLT, SRT,
	/* aritm */
	ADDT, SUBT, ADDIT, SUBIT,
	INCT, DECT,
	MULT, DIVT, MODT, MULIT, DIVIT,
	/* white space */
	NEWT, EOFT,
	/* built-ins */
	ARGT, RETT,
	IFT, ELSET, WHILET, FORT
};

struct token {
	enum token_type type;
	char *start;
	int length;
};

extern struct token tokens[MAX_TOKENS];
extern int token_count;

int num_val(const struct token *t);
bool is_alpha(char c);
bool is_digit(char c);
bool is_alnum(char c);

#endif
