/*
 * symtab.c
 * symbol table module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <stddef.h>
#include <string.h>
#include "debug.h"
#include "symtab.h"

#define SYM_MAX 128

static struct sym syms[SYM_MAX];
static int sym_cnt, loc_cnt;

void sym_reset(void)
{
	sym_cnt = 0;
	loc_cnt = 0;
}

/* Clears only the local symbols, keeping the globals. */
void sym_locals_clear(void)
{
	loc_cnt = 0;
}

void sym_register(int kind, char *start, int length, int off, int is_ptr, int base)
{
	if (sym_cnt >= SYM_MAX)
		fatal(USER_ERR, NULL, "Too many variables!");

	syms[sym_cnt].start = start;
	syms[sym_cnt].length = length;
	syms[sym_cnt].kind = kind;
	syms[sym_cnt].off = off;
	syms[sym_cnt].is_ptr = is_ptr;
	syms[sym_cnt].base = base;
	sym_cnt++;
	if (kind == LOC)
		loc_cnt++;
}

/*
 * Finds a variable by name, preferring the local table over
 * the global one. Returns NULL if the variable does not exist.
 */
struct sym *
sym_lookup(char *start, int length)
{
	int i;

	for (i = 0; i < sym_cnt; i++)
		if (syms[i].kind == LOC && syms[i].length == length &&
		    !memcmp(syms[i].start, start, (size_t)length))
			return &syms[i];

	for (i = 0; i < sym_cnt; i++)
		if (syms[i].kind == GLO && syms[i].length == length &&
		    !memcmp(syms[i].start, start, (size_t)length))
			return &syms[i];

	return NULL;
}

/*
 * Finds a variable by name, preferring the local table over
 * the global one. Fatals if the variable does not exist.
 */
struct sym *
sym_find(char *start, int length)
{
	struct sym *s = sym_lookup(start, length);

	if (!s)
		fatal(USER_ERR, NULL, "Unknown variable '%.*s'!", length, start);
	return s;
}

int sym_local_count(void)
{
	return loc_cnt;
}
