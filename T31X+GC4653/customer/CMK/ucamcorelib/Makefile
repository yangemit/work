CONFIG_UCLIBC_BUILD=y

CROSS_COMPILE ?= mips-linux-uclibc-gnu-

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

EXEC := ucamera
OBJS := ucamera.o

LIBS :=

LIB_STATIC := libusbcamera.a
LIB_DYNAMIC := libusbcamera.so

CFLAGS = -fPIC -O2 -Wall  -march=mips32r2

ifeq ($(CONFIG_UCLIBC_BUILD), y)
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
	rm -f *.o *~
	rm $(LIB_STATIC) $(LIB_DYNAMIC)

cpy:
	cp libusbcamera.a ../ucamera_cmk_t02/lib/ucam/
	cp usbcamera.h ../ucamera_cmk_t02/include/ucam/
