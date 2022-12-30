CROSS_COMPILE ?= mips-linux-gnu-
COMPILE_TYPE ?= uclibc

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip
HOST := T31
SDK_DIR := t31_sdk
UCAMCORE_DIR := $(PWD)/../ucamcorelib/
UART_DIR := $(PWD)/../uart/

export HOST COMPILE_TYPE

EXEC := ucamera-t31

OBJS :=	imp-common.o \
	ucamera_sample.o

CFLAGS = $(INCLUDES) -O2 -Wall -march=mips32r2 -std=gnu99
ifeq ($(HOST), T31)
	CFLAGS += -DT31
endif

ifeq ($(HOST), T21)
	EXEC := ucamera-t21
	SDK_DIR := t21_sdk
	CFLAGS += -DT21
endif

ifeq ($(HOST), T20)
	EXEC := ucamera-t20
	SDK_DIR := t20_sdk
	CFLAGS += -DT20
endif

ifeq ($(COMPILE_TYPE), uclibc)
	CFLAGS += -muclibc
	LDFLAG += -muclibc
endif

all: $(EXEC)

INCLUDES = -I../../libs/anticopy/include \
		   -I../../libs/$(SDK_DIR)/include \
		   -I../../libs/ucamcore/include \
		   -I../../utils/include \
		   -I../include/ 


	LIBS := -L../../libs/$(SDK_DIR)/lib/$(COMPILE_TYPE) -limp \
		-L../../libs/$(SDK_DIR)/lib/$(COMPILE_TYPE) -lalog \
		-L../../libs/$(SDK_DIR)/lib/$(COMPILE_TYPE) -lsysutils \
		-L../../libs/ucamcore/lib/$(COMPILE_TYPE) -lusbcamera \
		-L../../libs/ucamcore/lib/$(COMPILE_TYPE) -luart \
		-L../../libs/anticopy/lib/$(COMPILE_TYPE) -lAntiCopy

LIBS += -Wall -Wl,-gc-sections -lpthread -lm -lrt -lstdc++ -ldl

ucamcore:
	make -C $(UCAMCORE_DIR)

ucamcore_clean:
	make -C $(UCAMCORE_DIR) clean

uartlib:
	make -C $(UART_DIR)

uartlib_clean:
	make -C $(UART_DIR) clean

$(EXEC):$(OBJS) ucamcore uartlib 
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $@ $(LIBS)
	$(STRIP) $@

%o:%c
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -o $@ -c $^
clean: ucamcore_clean uartlib_clean
	rm -rf $(EXEC) $(OBJS) ucamera-*

distclean: ucamcore_clean uartlib_clean
	rm -rf $(EXEC) $(OBJS) ucamera-*
	
