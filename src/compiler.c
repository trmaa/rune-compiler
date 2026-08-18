/*
 * compiler.c
 * compiler orchestrator for zc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "debug.h"
#include "tokenizer.h"

static void get_paths(const char *path, char *src, char *out);

void compile(const char *path)
{
	int ifd, ofd;
	char spt[32], opt[32];

	char *src;
	off_t src_sz;

	get_paths(path, spt, opt);

	/* load input file in memory */
	ifd = open(spt, O_RDONLY);
	if (ifd == -1)
		fatal(USER_ERR, NULL, "%s does not exist!", path);
	src_sz = lseek(ifd, 0, SEEK_END);
	lseek(ifd, 0, SEEK_SET);
	src = malloc(src_sz + 1);
	read(ifd, src, src_sz);
	src[src_sz] = '\0';
	close(ifd);
	/* file loaded at src */

	tokenize(src);

	ofd = open(opt, O_CREAT | O_WRONLY | O_TRUNC, 0644);

	/* rest of steps */

	close(ofd);
}

static void
get_paths(const char *path, char *src, char *out)
{
	int len = strlen(path);

	if (strcmp(&path[len-3], ".ru"))
		fatal(USER_ERR, NULL, "%s is not a .ru file!", path);

	strcpy(src, path);
	strcpy(out, path);
	strcpy(&out[len-3], ".s");
}
