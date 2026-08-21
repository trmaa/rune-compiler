/*
 * expr.h
 * expression codegen module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#ifndef EXPR_H
#define EXPR_H

#include "symtab.h"

void eval_expr(int off);
void expr_eax(void);
void expr_into(const char *reg);
void eval_atom(const char *reg);
int deref_is_byte(struct sym *s);
int is_lit(void);
int op_follows(int p);

#endif
