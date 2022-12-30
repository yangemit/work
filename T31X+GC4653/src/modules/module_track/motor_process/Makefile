CROSS_COMPILE := mips-linux-gnu-

CC := $(CROSS_COMPILE)gcc
CPP := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)ld

ARFLAG := -rcs

CFLAGS := -g -Wall -Wextra -fPIC -Ofast -O3 -ffast-math -lpthread -Wno-unused-parameter -Werror
CFLAGS += -D_GNU_SOURCE

# CPPFLAGS := -std=c++11
HOST_SOC := t31

ifeq ($(COMPILE_TYPE), uclibc)
	CFLAGS += -muclibc
endif

ifeq ($(HOST_SOC), t31)
	CFLAGS += -DSOC_T31
endif

ifeq ($(OSD_DEBUG), yes)
	CFLAGS += -DOSD_CONTROL
endif

INCLUDE := -I./

INCLUDE += -I../../../../libs/person_tracker/Ingenic-IIAL/$(HOST_SOC)/IVS/include \
	   -I../../../../libs/t31_sdk/include \
	   -I../../../include

OBJS := motor_process.o \


TARGET := libmotor_process.a

All: $(TARGET) cp

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cc
	$(CPP) $(CFLAGS) -lstdc++ $(INCLUDE) $(LIBS) -o $@ -c $^

$(TARGET): $(OBJS)
	$(AR) $(ARFLAG) $@ $^

cp:
	cp $(TARGET) ../../../../libs/motor_process/lib/$(COMPILE_TYPE)/$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
