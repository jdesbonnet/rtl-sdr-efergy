#!/bin/bash
C_FILES="elite-decode"

for f in $C_FILES; do
	gcc -o $f $f.c
done

