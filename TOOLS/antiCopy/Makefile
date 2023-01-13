
#CROSS_COMPILE := /opt/mips-gcc540-glibc222-64bit-r3.3.0/bin/mips-linux-gnu-
CROSS_COMPILE := mips-linux-gnu-
CC := $(CROSS_COMPILE)gcc
CPP := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)ld

LIBTYPE := uclibc
HOST := T40

CFLAGS := -g -O2 -Wall -Werror
CFLAGS += -DSOC_$(HOST)

ifeq ($(LIBTYPE), uclibc)
CFLAGS += -muclibc
endif

INCLUDE := -I./include

LIBS := -L./ -lAntiCopy

TARGET := anticopy_test

all: $(TARGET)

$(TARGET): libAntiCopy.a anticopy
	$(CC) $(CFLAGS) $(INCLUDE) test/anticopy_test.c -o $@ $(LIBS)
	$(STRIP) $@

OBJS_LIB := src/chipid.o \
	src/AntiCopy_Verify.o \

OBJS_PROCESS := src/chipid.o \
		src/AntiCopy.o \

libAntiCopy.a: $(OBJS_LIB)
	$(LD) -r -Bstatic -EL $^ -o $@

anticopy: $(OBJS_PROCESS)
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@
	$(STRIP) $@

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $^

clean:
	rm -rf $(TARGET) libAntiCopy.a anticopy $(OBJS_LIB) $(OBJS_PROCESS)
