CROSS_COMPILE ?= mips-linux-gnu-
COMPILE_TYPE ?= uclibc

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

EXEC := ucamera
OBJS := ucamera.o
HOST := T31

LIBS :=

LIB_STATIC := libusbcamera.a
LIB_DYNAMIC := libusbcamera.so

CFLAGS = -fPIC -O2 -Wall -march=mips32r2 -Werror
CFLAGS += -D$(HOST)

ifeq ($(COMPILE_TYPE), uclibc)
CFLAGS += -muclibc
LDFLAG += -muclibc
endif

LDFLAG += -Wall -Wl,-gc-sections -lpthread -lm -lrt -lstdc++ -ldl

ALL: $(LIB_STATIC) $(LIB_DYNAMIC) cpy

$(LIB_STATIC): $(OBJS)
	$(LD) -r -Bstatic -EL $^ -o $@ $(LIBS)

$(LIB_DYNAMIC): $(OBJS)
	$(CC) -shared $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -rf $(LIB_STATIC) $(LIB_DYNAMIC) $(OBJS)

cpy:
ifeq ($(COMPILE_TYPE), uclibc)
	cp libusbcamera.a ../../../../libs/ucamcore/lib/uclibc/
else
	cp libusbcamera.a ../../../../libs/ucamcore/lib/glibc/
endif
