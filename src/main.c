/* main.c
 * CLI entry point for zc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include "debug.h"

const char *AUTHOR = "Pablo Trik Marin";
const char *VERSION = "a0.1-slime";

/* possible const struct conf {...} CONFIG; */

void help();

static int get_opts(int argc, char *argv[]);

main(int argc, char *argv[])
{
	int fid;

	if (argc == 1)
		fatal(USER_ERR, help, "%s needs args!", argv[0]);

	fid = get_opts(argc, argv);

	if (fid >= argc)
		fatal(USER_ERR, help, "%s needs files to compile!", argv[0]);

	for (; fid < argc; fid++)
		compile(argv[fid]);

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
		default:
			fatal(USER_ERR, help, "Wrong opt!");
		}

		i++;
	}

	return i;
}
