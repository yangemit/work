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
	   -I../../ \
	   -I../../../utils/include \

INCLUDE += -I../../../libs/t31_sdk/include

INCLUDE += -I$(LIBS_DIR)/faceDet/include \

OBJS := module_key_gpio.o 

TARGET := libkey_gpio.a

All: $(TARGET)

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

#%.o:%.cpp
#	$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

#%.o:%.cc
#	$(CPP) $(CFLAGS) -lstdc++ $(INCLUDE) $(LIBS) -o $@ -c $^

$(TARGET): $(OBJS)
	$(AR) $(ARFLAG) $@ $^

clean:
	rm -f $(TARGET) $(OBJS)
