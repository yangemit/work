#!/bin/bash

rm out/* -rf

make distclean

./configure --prefix=$PWD/out --host=mips-linux-gnu CPPFLAGS="-I../zlib-1.2.11/out/include" LDFLAGS="-L../zlib-1.2.11/out/lib" CFLAGS="-muclibc" --enable-shared=no --enable-static=yes

make 

make install
