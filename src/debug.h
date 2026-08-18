/*
 * debug.h
 * debug and error utilities for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#ifndef	DEBUG_H
#define	DEBUG_H

enum err {
	OK = 0,
	ERR = 1,
	USER_ERR = 2,
	OS_ERR = -1
};

void debug(const char *fmt, ...);
void fatal(enum err stat, void (*cb)(), const char *fmt, ...);

#endif
