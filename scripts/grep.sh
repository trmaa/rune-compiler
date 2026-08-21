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
	echo "Use: ./grep [opts] 'string to search'"
	echo 'Out: The file and line number where that was found in src/'
	echo 'Opts: -h prints this msg'
	exit $1
}

main() {
	if (( $# < 1 )); then
		echo -e '\e[33mTo grep the base, you need a string to grep for\e[0m'
		help $ERR
	fi

	[[ $1 == '-h' ]] && help $OK

	local f
	for f in src/*; do
		local match=$(nl -ba < $f | grep -F $1)
		if [[ -n $match ]]; then
			echo $f >&2
			echo "$match"
		fi
	done
}

main $@
