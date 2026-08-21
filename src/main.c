/*
 * main.c
 * CLI entry point for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <stdbool.h>
#include "globals.h"
#include "debug.h"

const char *AUTHOR = "Pablo Trik Marin";
const char *VERSION = "a0.6-clean";

struct conf CONFIG;

void help();

static int get_opts(int argc, char *argv[]);

main(int argc, char *argv[])
{
	int fid;

	if (argc == 1)
		fatal(USER_ERR, help, "%s needs args!", argv[0]);

	fid = get_opts(argc, argv);

#ifdef DEBUG
	CONFIG.debug = true;
#endif

	if (fid >= argc)
		fatal(USER_ERR, help, "%s needs files to compile!", argv[0]);

	for (; fid < argc; fid++) {
		log("Compiling: %s\n", argv[fid]);
		compile(argv[fid]);
	}

	return OK;
}

static int
get_opts(int argc, char *argv[])
{
	int i = 1;

	while (i < argc && argv[i][0] == '-') {
		switch (argv[i][1]) {
		case 'h':
			help();
			exit(OK);
		case 'v':
			say_version();
			exit(OK);
		case 's':
			CONFIG.silent = true;
			break;
		case 'd':
			CONFIG.debug = true;
			break;
		default:
			fatal(USER_ERR, help, "Wrong opt!");
		}

		i++;
	}

	return i;
}
