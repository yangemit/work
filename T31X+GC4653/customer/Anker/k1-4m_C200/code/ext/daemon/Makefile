CROSS_COMPILE ?= mips-linux-gnu-
COMPILE_TYPE := uclibc

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

EXEC := update_bin

OBJS :=	./update_bin.o

CFLAGS = $(INCLUDES) -O2 -Wall -march=mips32r2 -std=gnu99 -Werror

ifeq ($(COMPILE_TYPE), uclibc)
	CFLAGS += -muclibc
	LDFLAG += -muclibc
endif

all: $(EXEC)

LIBS += -Wall -Wl,-gc-sections -lpthread -lm -lrt -lstdc++ -ldl

$(EXEC):$(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $@ $(LIBS)
	$(STRIP) $@

%o:%c
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -o $@ -c $^
clean:
	rm -rf $(EXEC) $(OBJS) ucamera-t31*


