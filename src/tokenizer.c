/*
 * tokenizer.c
 * tokenizer module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include "debug.h"
#include "tokenizer.h"
#include "globals.h"

struct token tokens[MAX_TOKENS];
int token_count;

static char charbuf[MAX_TOKENS][16];

static void add_token(enum token_type type, char *start, int length);
static int is_hex(char c);
static int hex_val(char c);

/*
 * Tokenizes the source string into an array of tokens.
 * Recognizes keywords, identifiers, numbers, string literals,
 * and single-character symbols. Comments starting with //
 * are skipped. The resulting tokens are stored in the global
 * tokens[] array with token_count updated accordingly.
 */
void tokenize(char *src)
{
	char *start;
	int len;
	int i = 0;

	token_count = 0;

	while (src[i] != '\0') {
		/* whitespace */
		if (src[i] == ' ' || src[i] == '\t') {
			i++;
			continue;
		}

		/* newlines (collapse consecutive into one NEWT) */
		if (src[i] == '\n' || src[i] == '\r') {
			i++;
			while (src[i] == '\n' || src[i] == '\r')
				i++;
			add_token(NEWT, &src[i - 1], 1);
			continue;
		}

		/* comments */
		if (src[i] == '/' && src[i+1] == '/') {
			while (src[i] != '\n' && src[i] != '\0')
				i++;
			continue;
		}

		/* identifiers / keywords */
		if (is_alpha(src[i])) {
			start = &src[i];
			while (is_alnum(src[i]))
				i++;
			len = (int)(&src[i] - start);

			if (len == 3 && !memcmp(start, "let", 3))
				add_token(SETWT, start, len);
			else if (len == 3 && !memcmp(start, "leb", 3))
				add_token(SETBT, start, len);
			else if (len == 4 && !memcmp(start, "setw", 4))
				add_token(SETWT, start, len);
			else if (len == 4 && !memcmp(start, "setb", 4))
				add_token(SETBT, start, len);
			else if (len == 3 && !memcmp(start, "pub", 3))
				add_token(PUBT, start, len);
			else if (len == 2 && !memcmp(start, "fn", 2))
				add_token(FNT, start, len);
			else if (len == 3 && !memcmp(start, "ret", 3))
				add_token(RETT, start, len);
			else if (len == 2 && !memcmp(start, "if", 2))
				add_token(IFT, start, len);
			else if (len == 4 && !memcmp(start, "else", 4))
				add_token(ELSET, start, len);
			else if (len == 5 && !memcmp(start, "while", 5))
				add_token(WHILET, start, len);
			else if (len == 3 && !memcmp(start, "for", 3))
				add_token(FORT, start, len);
			else if (len == 3 && !memcmp(start, "arg", 3))
				add_token(ARGT, start, len);
			else
				add_token(IDT, start, len);
			continue;
		}

		/* numbers */
		if (is_digit(src[i])) {
			start = &src[i];
			if (src[i] == '0' && (src[i+1] == 'x' || src[i+1] == 'X')) {
				i += 2;
				while (is_hex(src[i]))
					i++;
			} else if (src[i] == '0' && (src[i+1] == 'b' || src[i+1] == 'B')) {
				i += 2;
				while (src[i] == '0' || src[i] == '1')
					i++;
			} else {
				while (is_digit(src[i]))
					i++;
				/* float literals (3.14, 0.5) */
				if (src[i] == '.' && is_digit(src[i+1])) {
					i++;
					while (is_digit(src[i]))
						i++;
					add_token(FLOATT, start, (int)(&src[i] - start));
					continue;
				}
			}

			add_token(INTT, start, (int)(&src[i] - start));
			continue;
		}

		/* string literals */
		if (src[i] == '"') {
			i++;
			start = &src[i];
			while (src[i] != '"' && src[i] != '\0')
				i++;
			add_token(STRT, start, (int)(&src[i] - start));
			i++;
			continue;
		}

		/* char literals */
		if (src[i] == '\'') {
			char val;
			i++; /* skip opening ' */
			if (src[i] == '\\') {
				i++;
				switch (src[i]) {
				case 'n':  val = '\n'; i++; break;
				case 't':  val = '\t'; i++; break;
				case '\\': val = '\\'; i++; break;
				case '\'': val = '\''; i++; break;
				case '0':  val = '\0'; i++; break;
				case 'x': {
					int h = 0, digits = 0;
					i++;
					while (is_hex(src[i]) && digits < 2) {
						h = h * 16 + hex_val(src[i]);
						i++;
						digits++;
					}
					val = (char)h;
					break;
				}
				case 'b': {
					int b = 0, bits = 0;
					i++;
					while ((src[i] == '0' || src[i] == '1') && bits < 8) {
						b = b * 2 + (src[i] - '0');
						i++;
						bits++;
					}
					val = (char)b;
					break;
				}
				default:
					fatal(USER_ERR, NULL, "Unknown escape '\\%c'", src[i]);
				}
			} else {
				val = src[i];
				i++;
			}
			i++; /* skip closing ' */
			sprintf(charbuf[token_count], "%d", (int)(unsigned char)val);
			add_token(CHART, charbuf[token_count], strlen(charbuf[token_count]));
			continue;
		}

		/* tokens */
		switch (src[i]) {
		case '[': add_token(LBCT, &src[i], 1); i++; continue;
		case ']': add_token(RBCT, &src[i], 1); i++; continue;
		case '{': add_token(LBKT, &src[i], 1); i++; continue;
		case '}': add_token(RBKT, &src[i], 1); i++; continue;
		case '(': add_token(LPT, &src[i], 1); i++; continue;
		case ')': add_token(RPT, &src[i], 1); i++; continue;
		case ',': add_token(COMT, &src[i], 1); i++; continue;
		case ';': add_token(SEMIT, &src[i], 1); i++; continue;
		case '+':
			if (src[i+1] == '+') { add_token(INCT, &src[i], 2); i += 2; }
			else if (src[i+1] == '=') { add_token(ADDIT, &src[i], 2); i += 2; }
			else { add_token(ADDT, &src[i], 1); i++; }
			continue;
		case '-':
			if (src[i+1] == '-') { add_token(DECT, &src[i], 2); i += 2; }
			else if (src[i+1] == '=') { add_token(SUBIT, &src[i], 2); i += 2; }
			else { add_token(SUBT, &src[i], 1); i++; }
			continue;
		case '*':
			if (src[i+1] == '=') { add_token(MULIT, &src[i], 2); i += 2; }
			else { add_token(MULT, &src[i], 1); i++; }
			continue;
		case '/':
			if (src[i+1] == '=') { add_token(DIVIT, &src[i], 2); i += 2; }
			else { add_token(DIVT, &src[i], 1); i++; }
			continue;
		case '%': add_token(MODT, &src[i], 1); i++; continue;
		case '=':
			if (src[i+1] == '=') { add_token(EQQT, &src[i], 2); i += 2; }
			else { add_token(EQT, &src[i], 1); i++; }
			continue;
		case '!':
			if (src[i+1] == '=') { add_token(NOTIT, &src[i], 2); i += 2; }
			else { add_token(NOTT, &src[i], 1); i++; }
			continue;
		case '<':
			if (src[i+1] == '<') { add_token(SLT, &src[i], 2); i += 2; }
			else if (src[i+1] == '=') { add_token(LET, &src[i], 2); i += 2; }
			else { add_token(LTT, &src[i], 1); i++; }
			continue;
		case '>':
			if (src[i+1] == '>') { add_token(SRT, &src[i], 2); i += 2; }
			else if (src[i+1] == '=') { add_token(GET, &src[i], 2); i += 2; }
			else { add_token(GTT, &src[i], 1); i++; }
			continue;
		case '&':
			if (src[i+1] == '&') { add_token(ANDT, &src[i], 2); i += 2; }
			else if (src[i+1] == '=') { add_token(BANDIT, &src[i], 2); i += 2; }
			else { add_token(BANDT, &src[i], 1); i++; }
			continue;
		case '|':
			if (src[i+1] == '|') { add_token(ORT, &src[i], 2); i += 2; }
			else if (src[i+1] == '=') { add_token(BORIT, &src[i], 2); i += 2; }
			else { add_token(BORT, &src[i], 1); i++; }
			continue;
		case '^':
			if (src[i+1] == '=') { add_token(XORIT, &src[i], 2); i += 2; }
			else { add_token(XORT, &src[i], 1); i++; }
			continue;
		case '~':
			if (src[i+1] == '=') { add_token(BNOTIT, &src[i], 2); i += 2; }
			else { add_token(BNOTT, &src[i], 1); i++; }
			continue;
		}

		fatal(USER_ERR, NULL, "Unexpected character '%c'", src[i]);
	}

	add_token(EOFT, "", 0);
}

bool is_alpha(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool is_digit(char c)
{
	return c >= '0' && c <= '9';
}

bool is_alnum(char c)
{
	return is_alpha(c) || is_digit(c);
}

/*
 * Numeric value of a literal token. Handles decimal,
 * hexadecimal (0x..) and binary (0b..) literals.
 */
int num_val(const struct token *t)
{
	const char *s = t->start;
	int len = t->length;
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

static int
is_hex(char c)
{
	return is_digit(c) ||
	       (c >= 'a' && c <= 'f') ||
	       (c >= 'A' && c <= 'F');
}

static int
hex_val(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return c - 'A' + 10;
}

static void
add_token(enum token_type type, char *start, int length)
{
	int i;

	if (token_count >= MAX_TOKENS)
		fatal(OS_ERR, NULL, "Too many tokens");
	tokens[token_count].type = type;
	tokens[token_count].start = start;
	tokens[token_count].length = length;
	token_count++;

	if (CONFIG.debug) {
		debug("Tokenized -> %s", t_name(type));
		for (i = 0; i < length; i++)
			fprintf(stderr, "%c", start[i]);
		fprintf(stderr, "\n");
	}
}

char *t_name(enum token_type t)
{
	switch (t) {
	case SETWT:  return "SETW";
	case SETBT:  return "SETB";
	case PUBT:   return "PUB";
	case FNT:    return "FN";
	case IDT:    return "ID";
	case EQT:    return "EQ";
	case STRT:   return "STR";
	case INTT:   return "INT";
	case FLOATT: return "FLOAT";
	case CHART:  return "CHAR";
	case LPT:    return "LP";
	case RPT:    return "RP";
	case LBCT:   return "LBC";
	case RBCT:   return "RBC";
	case LBKT:   return "LBK";
	case RBKT:   return "RBK";
	case COMT:   return "COM";
	case DOTT:   return "DOT";
	case SEMIT:  return "SEMI";
	case LTT:    return "LT";
	case GTT:    return "GT";
	case LET:    return "LE";
	case GET:    return "GE";
	case EQQT:   return "EQQ";
	case NOTT:   return "NOT";
	case ANDT:   return "AND";
	case ORT:    return "OR";
	case BNOTT:  return "BNOT";
	case BANDT:  return "BAND";
	case BORT:   return "BOR";
	case XORT:   return "XOR";
	case NOTIT:  return "NOTI";
	case BNOTIT: return "BNOTI";
	case BANDIT: return "BANDI";
	case BORIT:  return "BORI";
	case XORIT:  return "XORI";
	case SLT:    return "SL";
	case SRT:    return "SR";
	case ADDT:   return "ADD";
	case SUBT:   return "SUB";
	case ADDIT:  return "ADDI";
	case SUBIT:  return "SUBI";
	case INCT:   return "INC";
	case DECT:   return "DEC";
	case MULT:   return "MUL";
	case DIVT:   return "DIV";
	case MODT:   return "MOD";
	case MULIT:  return "MULI";
	case DIVIT:  return "DIVI";
	case NEWT:   return "NEW";
	case EOFT:   return "EOF";
	case ARGT:   return "ARG";
	case RETT:   return "RET";
	case IFT:    return "IF";
	case ELSET:  return "ELSE";
	case WHILET: return "WHILE";
	case FORT:   return "FOR";
	}
	return "THIS AIN'T A TOKEN!";
}
