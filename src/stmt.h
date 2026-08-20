/*
 * stmt.h
 * statement codegen module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#ifndef STMT_H
#define STMT_H

#include <stdio.h>
#include "parser.h"

void parse_fn(enum visi vis, FILE *out);

#endif