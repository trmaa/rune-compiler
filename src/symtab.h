/*
 * symtab.h
 * symbol table module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#ifndef SYMTAB_H
#define SYMTAB_H

enum sym_kind {
	LOC, GLO
};

/* base types - the type a pointer chain finally points to */
enum base_type {
	T_WORD, T_BYTE
};

struct sym {
	char *start;
	int length;
	int kind;
	int off;
	int is_ptr;	/* pointer depth (number of stars) */
	int base;	/* enum base_type of the final pointee */
};

void sym_reset(void);
void sym_locals_clear(void);
void sym_register(int kind, char *start, int length, int off, int is_ptr,
		  int base);
struct sym *sym_find(char *start, int length);
int sym_local_count(void);

#endif