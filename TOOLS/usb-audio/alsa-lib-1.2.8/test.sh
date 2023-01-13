#!/bin/bash

rm /system/alsa/ -rf

make clean

#uclibc
#CFLAGS+="-muclibc" CPPFLAGS+="-muclibc" CXXFLAGS+="-muclibc" ./configure --prefix=/system/alsa/ --host=mips-linux-gnu --with-configdir=/system/alsa/ --with-plugindir=/system/alsa/lib/

#glibc
./configure --prefix=/system/alsa/ --host=mips-linux-gnu --with-configdir=/system/alsa/ --with-plugindir=/system/alsa/lib/

make
sudo -s
source /etc/profile

make install
