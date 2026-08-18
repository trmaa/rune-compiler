/* tokenizer.c
 * tokenizer module for zc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <stddef.h>
#include <stdbool.h>
#include "debug.h"
#include "tokenizer.h"

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
			len;
			while (is_alnum(src[i]))
				i++;
			len = (int)(&src[i] - start);

			if (len == 4 && !memcmp(start, "word", 4))
				add_token(WORDT, start, len);
			else if (len == 4 && !memcmp(start, "byte", 4))
				add_token(BYTET, start, len);
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
			else
				add_token(IDENTT, start, len);
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
			}
			add_token(NUMT, start, (int)(&src[i] - start));
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
			add_token(CHARLITT, charbuf[token_count], strlen(charbuf[token_count]));
			continue;
		}

		/* tokens */
		switch (src[i]) {
		case '[': add_token(LBRAT, &src[i], 1); i++; continue;
		case ']': add_token(RBRAT, &src[i], 1); i++; continue;
		case '{': add_token(LBRAC, &src[i], 1); i++; continue;
		case '}': add_token(RBRAC, &src[i], 1); i++; continue;
		case '(': add_token(LPAREN, &src[i], 1); i++; continue;
		case ')': add_token(RPAREN, &src[i], 1); i++; continue;
		case ',': add_token(COMMAT, &src[i], 1); i++; continue;
		case ';': add_token(SEMIT, &src[i], 1); i++; continue;
		case '+': add_token(PLUST, &src[i], 1); i++; continue;
		case '-': add_token(MINUST, &src[i], 1); i++; continue;
		case '*': add_token(START, &src[i], 1); i++; continue;
		case '/': add_token(SLASHT, &src[i], 1); i++; continue;
		case '=':
			if (src[i+1] == '=') { add_token(EQT, &src[i], 2); i += 2; }
			else { add_token(EQUALT, &src[i], 1); i++; }
			continue;
		case '!':
			if (src[i+1] == '=') { add_token(NEQ, &src[i], 2); i += 2; }
			else { add_token(BANG, &src[i], 1); i++; }
			continue;
		case '<':
			if (src[i+1] == '<') { add_token(LSHIFTT, &src[i], 2); i += 2; }
			else if (src[i+1] == '=') { add_token(LET, &src[i], 2); i += 2; }
			else { add_token(LTT, &src[i], 1); i++; }
			continue;
		case '>':
			if (src[i+1] == '>') { add_token(RSHIFTT, &src[i], 2); i += 2; }
			else if (src[i+1] == '=') { add_token(GET, &src[i], 2); i += 2; }
			else { add_token(GTT, &src[i], 1); i++; }
			continue;
		case '&':
			if (src[i+1] == '&') { add_token(AMPAMP, &src[i], 2); i += 2; }
			else { add_token(AMP, &src[i], 1); i++; }
			continue;
		case '|':
			if (src[i+1] == '|') { add_token(PIPEPIPE, &src[i], 2); i += 2; }
			else { add_token(PIPE, &src[i], 1); i++; }
			continue;
		case '^': add_token(CARET, &src[i], 1); i++; continue;
		case '~': add_token(TILDE, &src[i], 1); i++; continue;
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
	if (token_count >= MAX_TOKENS)
		fatal(OS_ERR, NULL, "Too many tokens");
	tokens[token_count].type = type;
	tokens[token_count].start = start;
	tokens[token_count].length = length;
	token_count++;
}
