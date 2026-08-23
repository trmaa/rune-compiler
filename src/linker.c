/*
 * linker.c
 * fork-exec wrapper to cc for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <string.h>
#include "debug.h"
#include "globals.h"

#define MAXF 32

void link(const char *files[])
{
	char obj[MAXF][32];
	char *args[MAXF + 8];
	int n = 0, i, j = 0, pid, stat;

	args[j++] = "cc";
	args[j++] = "-m32";
	args[j++] = "-no-pie";
	args[j++] = "-o";
	args[j++] = CONFIG.out;

	for (i = 0; files[i]; i++) {
		int len = strlen(files[i]);

		if (!strcmp(&files[i][len-3], ".rn")) {
			strcpy(obj[n], files[i]);
			strcpy(&obj[n][len-3], ".o");
			args[j++] = obj[n++];
		} else {
			args[j++] = (char *) files[i];
		}
	}
	args[j] = NULL;

	pid = fork();
	if (pid == 0) {
		execvp(args[0], args);
		fatal(OS_ERR, NULL, "%s failed!", args[0]);
	}

	wait(&stat);
	if (stat != 0)
		fatal(ERR, NULL, "link failed!");

	for (i = 0; i < n; i++)
		unlink(obj[i]);
}
