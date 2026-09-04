#! /usr/bin/env bash

# grep.sh
#
# Script to look up the code base in search of strings.
# Usage at: ./grep -h
#
# Copyright (c) 2026 Pablo Trik Marin
# License: GPL

OK=0
ERR=1

help() {
	local stat=$1

	echo "Use: ./grep [opts] 'string to search'"
	echo 'Out: The file and line number where that was found in src/'
	echo 'Opts: -h prints this msg'

	exit $stat
}

main() {
	if (( $# < 1 )); then
		echo -e '\e[33mTo grep the base, you need a string to grep for\e[0m' >&2
		help $ERR
	fi

	[[ "$1" == '-h' ]] && help $OK

	local search="$1"

	local f
	for f in src/*; do
		[[ -f "$f" ]] || continue # omit dirs

		local match=$(nl -ba -v 0 < "$f" | grep -F "$search")
		if [[ -n $match ]]; then
			echo "$f"
			echo "$match"
		fi
	done | less -F
}

main "$@"
