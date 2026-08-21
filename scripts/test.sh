#! /usr/bin/env bash

# test.sh
#
# Script to test the compiler.
#
# Copyright (c) 2026 Pablo Trik Marin
# License: GPL

./rc tests/*.rn
cd tests
./comp-tests
cd ..
