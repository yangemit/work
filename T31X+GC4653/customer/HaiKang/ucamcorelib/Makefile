CONFIG_UCLIBC_BUILD=y

CROSS_COMPILE ?= mips-linux-uclibc-gnu-

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

OBJS := ucamera.o

LIBS :=

LIB_STATIC := libusbcamera.a
LIB_DYNAMIC := libusbcamera.so
LIBTYPE := glibc

CFLAGS = -fPIC -O2 -Wall -march=mips32r2

ifeq ($(CONFIG_UCLIBC_BUILD), y)
CFLAGS += -muclibc
LDFLAG += -muclibc
LIBTYPE = uclibc
endif


LDFLAG += -Wall -Wl,-gc-sections -lpthread -lm -lrt -lstdc++ -ldl

ALL: $(LIB_STATIC) $(LIB_DYNAMIC)

$(LIB_STATIC): $(OBJS)
	$(LD) -r -Bstatic -EL $^ -o $@ $(LIBS)

$(LIB_DYNAMIC): $(OBJS)
	$(CC) -shared $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -rf $(OBJS) $(LIB_STATIC) $(LIB_DYNAMIC)

cpy:
	cp libusbcamera.a ../ucamera/lib/ucam/
	cp usbcamera.h ../ucamera/include/ucam
install:
	cp libusbcamera.a ./libucam/lib/$(LIBTYPE)
	cp usbcamera.h ./libucam/include

