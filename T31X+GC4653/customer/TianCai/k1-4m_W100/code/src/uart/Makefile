CROSS_COMPILE := mips-linux-gnu-
COMPILE_TYPE ?= uclibc

CC := $(CROSS_COMPILE)gcc
CPP := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)ld

ARFLAG := -rcs

CFLAGS := -g -Wall -Wextra -fPIC -Ofast -O2 -ffast-math -lpthread -Wno-unused-parameter -std=gnu99 
#CPPFLAGS := -std=c++11

ifeq ($(COMPILE_TYPE), uclibc)
	CFLAGS += -muclibc
endif


INCLUDE := -I../ucamera \
		-I../../libs/ucamcore/include \
		-I../../libs/t31_sdk/include \
		-I../include

OBJS := hsb.o\
	uart.o

LIB_STATIC := libuart.a
LIB_DYNAMIC := libuart.so

All: $(LIB_STATIC) $(LIB_DYNAMIC)

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cc
	$(CPP) $(CFLAGS) -lstdc++ $(INCLUDE) $(LIBS) -o $@ -c $^

$(LIB_STATIC): $(OBJS)
	$(AR) $(ARFLAG) $@ $^ 

$(LIB_DYNAMIC): $(OBJS) cpy
	$(CC) -shared $(CFLAGS) -o $@ 

clean:
	rm -f $(LIB_DYNAMIC) $(LIB_STATIC) $(OBJS)

cpy:
	cp libuart.a ../../libs/ucamcore/lib/uclibc
