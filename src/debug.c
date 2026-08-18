/*
 * debug.c
 * debug and error utilities for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <stdio.h>
#include <stdarg.h>
#include "debug.h"
#include "globals.h"

void say_version()
{
	printf("rc (rune compiler) v%s\n", VERSION);
	printf("Copyright (c) 2026 %s\n", AUTHOR);
	printf("License: GPL\n");
}

void help()
{
	printf("Usage: rc [options] [file.rn]\n");
	printf("Options:\n");
	printf("\t-h print this text.\n");
	printf("\t-v show the version.\n");
}

void debug(const char *fmt, ...)
{
#ifdef DEBUG
	char buf[1024];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, sizeof buf, fmt, args);
	va_end(args);

	fprintf(stderr, "\x1b[33mDebug: %s\n\x1b[0m", buf);
#endif
}

void fatal(enum err stat, void (*cb)(), const char *fmt, ...)
{
	char buf[1024];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, sizeof buf, fmt, args);
	va_end(args);

	fprintf(stderr, "\x1b[31mFatal: %s\n\x1b[0m", buf);

	if (cb)
		cb();

	exit(stat);
}
