
CROSS_COMPILE :=
CC := $(CROSS_COMPILE)gcc
CPP := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
AR := $(CROSS_COMPILE)ar

CFLAGS := -g -O2 -Wall -Werror

INCLUDE :=

LIBS :=

OBJS := package_gen.o \
	md5.o

all: package_gen

package_gen: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) $(OBJS) -o $@ $(LIBS)
	$(STRIP) $@

%o:%c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

clean:
	rm $(OBJS)
	rm -rf package_gen
	rm -rf update_package.bin
	rm -rf ./firmware/firmware.bin
