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
	printf("Options:\t\t(opts can be one char long)\n");
	printf("\t-debug\t\tprints extra info about the compiling process.\n");
	printf("\t-help\t\tprint this text.\n");
	printf("\t-silent\t\tlogs dont show up.\n");
	printf("\t-version\tshow the version.\n");
}

void log(const char *fmt, ...)
{
	if (!CONFIG.silent) {
		char buf[1024];
		va_list args;

		va_start(args, fmt);
		vsnprintf(buf, sizeof buf, fmt, args);
		va_end(args);

		printf("%s\n", buf);
	}
}

void debug(const char *fmt, ...)
{
	if (CONFIG.debug) {
		char buf[1024];
		va_list args;

		va_start(args, fmt);
		vsnprintf(buf, sizeof buf, fmt, args);
		va_end(args);

		fprintf(stderr, "\x1b[33mDebug: %s\n\x1b[0m", buf);
	}
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
