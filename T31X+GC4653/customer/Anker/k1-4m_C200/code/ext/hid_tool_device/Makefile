CROSS_COMPILE := mips-linux-gnu-
CC := $(CROSS_COMPILE)gcc
CPP := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
AR := $(CROSS_COMPILE)ar

COMPILE_TYPE := uclibc
CLIBTPDIR_FIX := glibc


CFLAGS := $(INCLUDE)-g -O2 -Wall -Werror -std=gnu99

MODULES_CLEAN :=

TARGET := hid_update

INCLUDE := -I../../src/include \
		-I./include

ifneq ($(COMPILE_TYPE), glibc)
	CFLAGS += -muclibc
	CLIBTPDIR_FIX:= uclibc
endif

ifeq ($(HAA_EN), 1)
	CFLAGS += -DHAA_ON
	OBJS += ./hid_test.o 	\
		./hsb.o      	\
		./hou.o         \
		./haa.o         \
		./imp_hsb.o


	LIBS += -Llib/iaac/$(CLIBTPDIR_FIX) -ljzdl \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lmxu_merge \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lmxu_objdetect \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lmxu_contrib \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lmxu_video \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lmxu_imgproc \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lmxu_core

	LIBS += -Llib/iaac/$(CLIBTPDIR_FIX) -liaac \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lfaceDet \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lpersonDet_inf


else
	LIBS += -Llib/imp/$(CLIBTPDIR_FIX) -limp \
		-Llib/imp/$(CLIBTPDIR_FIX) -lalog \
		-Llib/imp/$(CLIBTPDIR_FIX) -lsysutils

	OBJS += ./hid_test.o 	\
		./hsb.o      	\
		./hou.o		\
		./imp_hsb.o
endif

LIBS += -lpthread -lm -ldl -lrt -lstdc++


all: $(TARGET)

$(TARGET): $(OBJS) $(MODULES)
	$(CC) $(CFLAGS) $(INCLUDE) $(OBJS) -o $@ $(LIBS)
	$(STRIP) $@

%o:%c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

clean: $(MODULES_CLEAN)
	rm -rf $(TARGET) *.o
