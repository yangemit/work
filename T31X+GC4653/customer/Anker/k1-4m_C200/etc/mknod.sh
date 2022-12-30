#!/bin/bash

test -d ./root-uclibc-1.1 || tar -xf root-uclibc-1.1.tar.gz
test -d ./root-uclibc-1.1/dev || mkdir -p ./root-uclibc-1.1/dev

cd ./root-uclibc-1.1/dev/
test -e  console || sudo mknod console c 5 1
test -e null || sudo mknod null c 1 3

cd -
