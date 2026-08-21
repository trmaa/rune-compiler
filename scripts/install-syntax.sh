#! /usr/bin/env bash

# install-syntax.sh
#
# Install syntax highlighting for vim.
#
# Copyright (c) 2026 Pablo Trik Marin
# License: GPL

mkdir -p ~/.vim/syntax/
mkdir -p ~/.vim/ftdetect/

cp -r syntax/* ~/.vim/syntax/
cp -r ftdetect/* ~/.vim/ftdetect/
