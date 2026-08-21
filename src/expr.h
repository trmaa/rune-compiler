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
void eval_atom(const char *reg);
void eval_muldiv(void);
int deref_is_byte(struct sym *s);
const char *atom_operand(void);
int next_is_arith(void);
int is_lit(void);

#endif