CROSS_COMPILE := mips-linux-gnu-

CC := $(CROSS_COMPILE)gcc
CPP := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)ld

ARFLAG := -rcs

CFLAGS := -g -Wall -Wextra -fPIC -Ofast -O3 -ffast-math -lpthread -Wno-unused-parameter -Werror
CPPFLAGS := -std=c++11

CFLAGS += -std=gnu99

ifeq ($(COMPILE_TYPE), uclibc)
	CFLAGS += -muclibc
	LDFLAG += -muclibc
endif

INCLUDE := -I../../include \
	   -I../../

INCLUDE += -I../../../libs/t31_sdk/include

CFLAGS += $(INCLUDE)

OBJS := module_af_control.o

TARGET := libaf.a

All: $(TARGET)

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cc
	$(CPP) $(CFLAGS) -lstdc++ $(INCLUDE) $(LIBS) -o $@ -c $^

$(TARGET): $(OBJS)
	$(AR) $(ARFLAG) $@ $^

clean:
	rm -f $(TARGET) $(OBJS)
