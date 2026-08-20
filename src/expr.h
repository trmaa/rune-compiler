/*
 * expr.h
 * expression codegen module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#ifndef EXPR_H
#define EXPR_H

void eval_expr(int off);
const char *atom_operand(void);
int next_is_arith(void);
int is_lit(void);

#endif