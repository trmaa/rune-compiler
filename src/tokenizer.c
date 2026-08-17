#include <stddef.h>
#include <stdbool.h>
#include "debug.h"
#include "tokenizer.h"

struct token tokens[MAX_TOKENS];
int token_count;

static void add_token(enum token_type type, char *start, int length);

void tokenize(char *src)
{
	char *start;
	int len;
	int i = 0;

	token_count = 0;

	while (src[i] != '\0') {
		/* whitespace */
		if (src[i] == ' ' || src[i] == '\t' ||
		    src[i] == '\n' || src[i] == '\r') {
			i++;
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

			if (len == 3 && !memcmp(start, "let", 3))
				add_token(LETT, start, len);
			else if (len == 3 && !memcmp(start, "pub", 3))
				add_token(PUBT, start, len);
			else if (len == 2 && !memcmp(start, "fn", 2))
				add_token(FNT, start, len);
			else if (len == 3 && !memcmp(start, "ret", 3))
				add_token(RETT, start, len);
			else
				add_token(IDENTT, start, len);
			continue;
		}

		/* numbers */
		if (is_digit(src[i])) {
			start = &src[i];
			while (is_digit(src[i]))
				i++;
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

		/* single-char tokens */
		switch (src[i]) {
		case '[': add_token(LBRAT, &src[i], 1); i++; continue;
		case ']': add_token(RBRAT, &src[i], 1); i++; continue;
		case '{': add_token(LBRAC, &src[i], 1); i++; continue;
		case '}': add_token(RBRAC, &src[i], 1); i++; continue;
		case '(': add_token(LPAREN, &src[i], 1); i++; continue;
		case ')': add_token(RPAREN, &src[i], 1); i++; continue;
		case ',': add_token(COMMAT, &src[i], 1); i++; continue;
		case '=': add_token(EQUALT, &src[i], 1); i++; continue;
		case ';': add_token(SEMIT, &src[i], 1); i++; continue;
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
