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

struct sym {
	char *start;
	int length;
	int kind;
	int off;
};

void sym_reset(void);
void sym_locals_clear(void);
void sym_register(int kind, char *start, int length, int off);
struct sym *sym_find(char *start, int length);
int sym_local_count(void);

#endif