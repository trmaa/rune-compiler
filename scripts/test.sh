#! /usr/bin/env bash

# test.sh
#
# Script to test the compiler.
#
# Copyright (c) 2026 Pablo Trik Marin
# License: GPL

./rc tests/*.rn 2>&1 >/dev/null
cd tests
./comp-tests
cd ..
