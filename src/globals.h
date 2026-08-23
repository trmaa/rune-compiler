/*
 * globals.h
 * global extern declarations for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h>

struct conf {
	bool silent;
	bool debug;
	bool assemble;
	bool link;
	char out[16];
};

extern const char *AUTHOR;
extern const char *VERSION;
extern struct conf CONFIG;

#endif
