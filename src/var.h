/*
 * var.h
 * global variables and literal data module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#ifndef VAR_H
#define VAR_H

#include <stdio.h>
#include "parser.h"

void parse_var(enum visi vis, FILE *out);
int reg_str(void);
int reg_arr(void);

#endif