/* codegen.h
 * code generation module for zc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#define MAX_GLOBALS 64
#define MAX_LOCALS 64
#define MAX_STRINGS 64
#define MAX_STR_LEN 256

struct global_var {
	char *name;
	int name_len;
	int is_array;
	int array_size;
	int has_init;
	int init_val;
	int is_pub;
};

struct local_var {
	char *name;
	int name_len;
	int offset;
};

void codegen(int out_fd);

#endif
