#! /usr/bin/env bash

# comp-tests.sh
#
# Compile all generated assembly by *.rn.
#
# Copyright (c) 2026 Pablo Trik Marin
# License: GPL

main() {
	local files=*.rn

	for f in $files; do
		echo -n "Compiling $f... "

		local i=${f//.rn/.s}
		local o=${f//.rn/.o}

		if [[ ! -f $i ]]; then
			echo -e "\e[31m\n$f wasn't compiled\e[0m" >&2
			exit 1
		fi

		if ! as --32 $i -o $o 2>/dev/null; then
			echo -e "\e[31m\n$f didn't compile right\e[0m" >&2
			exit 1
		fi

		if  nm $o 2>/dev/null | grep main >/dev/null; then
			local bin=${o//.o/}
			if ! cc -m32 -no-pie $o test-lib.o -o $bin 2>/dev/null; then
				echo -e "\e[31m\n$f didn't link right\e[0m" >&2
				exit 1
			fi
		else
			echo -n "Skipping $o, it has no main simbol "
		fi

		echo OK
	done

	echo 'Ready to test!'
}

main
