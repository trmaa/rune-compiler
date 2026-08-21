#! /usr/bin/env bash

# comp-tests.sh
#
# Remove all generated assembly and binaries by *.rn.
#
# Copyright (c) 2026 Pablo Trik Marin
# License: GPL

main() {
	local files=*.rn

	for f in $files; do
		rm -f ${f//.rn/.s}
		rm -f ${f//.rn/.o}
		rm -f ${f//.rn/}
	done
}

main
