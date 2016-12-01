#!/bin/bash

BIN=../brighten
COLORS="1 2 3"
CHANGES="-128 -64 0 64 127"

	for file in *.ppm;
    do
        printf "\e[1;34mRunning tests for file: %s\e[0m\n" $file
        for color in $COLORS; do
            for change in $CHANGES; do
                printf "[%d, %d] " $color $change
                $BIN $file $color $change > test_"$color"_"$change"_$file 2> /dev/null
            done;
            printf '\n'
        done;
	done;

