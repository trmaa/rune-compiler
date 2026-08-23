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
const char *VERSION = "r1.4-grafite";

struct conf CONFIG;

void help();

static int get_opts(int argc, char *argv[]);

main(int argc, char *argv[])
{
	int fid, i;

	if (argc == 1)
		fatal(USER_ERR, help, "%s needs args!", argv[0]);

	fid = get_opts(argc, argv);

	if (fid >= argc)
		fatal(USER_ERR, help, "%s needs files to compile!", argv[0]);

	for (i = fid; i < argc; i++) {
		log("Compiling: %s", argv[i]);
		compile(argv[i]);

		if (CONFIG.assemble) {
			log("Assembling: %s", argv[i]);
			assemble(argv[i]);
		}
	}

	if (CONFIG.link) {
		log("Linking: %s", CONFIG.out);
		link(&argv[fid]);
	}

	return OK;
}

static int
get_opts(int argc, char *argv[])
{
	int i = 1;

	/* defaults */
	CONFIG.verbose = false;
#ifdef DEBUG
	CONFIG.debug = true;
#else
	CONFIG.debug = false;
#endif
	CONFIG.assemble = true;
	CONFIG.link = true;
	strcpy(CONFIG.out, "a.out");

	while (i < argc && argv[i][0] == '-') {
		switch (argv[i][1]) {
		case 'c':
			CONFIG.link = false;
			break;
		case 'd':
			CONFIG.debug = true;
			break;
		case 'h':
			help();
			exit(OK);
		case 'o':
			strcpy(CONFIG.out, argv[++i]);
			break;
		case 'V':
			CONFIG.verbose = true;
			break;
		case 'S':
			CONFIG.assemble = false;
			CONFIG.link = false;
			break;
		case 'v':
			say_version();
			exit(OK);
		default:
			fatal(USER_ERR, help, "Wrong opt!");
		}

		i++;
	}

	return i;
}
