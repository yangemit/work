CROSS_COMPILE ?= mips-linux-gnu-

CC := $(CROSS_COMPILE)gcc
CPP := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)ld

ARFLAG := -rcs

CFLAGS := -g -O2 -Wall -Werror

ifeq ($(LIBTYPE), uclibc)
	CFLAGS += -muclibc
endif

# include #
INCLUDE := -I./include

# libs #
LIBS :=

# objs #
OBJS :=	src/node_list.o \
	src/config/module_config.o

TARGET := libutils.a

All: $(TARGET)

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cpp
	$(CPP) $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cc
	$(CPP) $(CFLAGS) -lstdc++ $(INCLUDE) $(LIBS) -o $@ -c $^

$(TARGET): $(OBJS)
	$(LD) -r -Bstatic -EL $^ -o $@ $(LIBS)

clean:
	rm -f $(TARGET) $(OBJS)
