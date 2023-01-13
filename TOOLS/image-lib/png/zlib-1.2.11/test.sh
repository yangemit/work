#!/bin/bash

rm out/* -rf

make distclean

CFLAGS="-muclibc" CXXFLAGS="-muclibc" LDFLAGS="-muclibc"

export CC=mips-linux-gnu-gcc

./configure  --prefix=$PWD/out --static

make 

make install
