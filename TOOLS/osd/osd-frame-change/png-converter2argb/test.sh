#!/bin/bash

rm ARGB converterPNG2ARGB

gcc converterPNG2ARGB.c -o converterPNG2ARGB /usr/lib/x86_64-linux-gnu/libpng16.so.16.34.0
