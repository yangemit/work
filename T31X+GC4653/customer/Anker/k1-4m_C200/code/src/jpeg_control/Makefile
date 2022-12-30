CROSS_COMPILE := mips-linux-gnu-
COMPILE_TYPE := uclibc

CC := $(CROSS_COMPILE)gcc
CPP := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)ld

ARFLAG := -rcs

CFLAGS := -g -Wall -Wextra -fPIC -Ofast -O3 -ffast-math -lpthread -Wno-unused-parameter -std=gnu99 -Werror
CPPFLAGS := -std=c++11

ifeq ($(COMPILE_TYPE), uclibc)
CFLAGS += -muclibc
endif

INCLUDE := -Iinclude \
	-I../../libs/libjpeg/include

LIBS := -L../../libs/libjpeg/lib/$(COMPILE_TYPE)/ -ljpeg

OBJS := jpeg_control.o

TARGET := libjpeg_control.a

All: $(TARGET) cpy

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cc
	$(CPP) $(CFLAGS) -lstdc++ $(INCLUDE) $(LIBS) -o $@ -c $^

$(TARGET): $(OBJS)
	$(AR) $(ARFLAG) $@ $^

cpy:
	cp $(TARGET) ../../libs/libjpeg/lib/$(COMPILE_TYPE)/

sample_test:
	# $(CC) $(CFLAGS) $(INCLUDE) $(OBJS) -o $@ $(LIBS) -ljpeg_control -lm -ldl -lrt -lpthread
	$(CC) ./test/jpeg_decode.c -o sample_test -L./ -ljpeg_control $(LIBS) -I./include $(CFLAGS)
	$(STRIP) $@

sample_test_clean:
	rm -rf sample_test ./test/*.o

clean:
	rm -f $(TARGET) $(OBJS) sample_test
