CROSS_COMPILE := mips-linux-gnu-
CC := $(CROSS_COMPILE)gcc
CPP := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
AR := $(CROSS_COMPILE)ar

COMPILE_TYPE := uclibc
CLIBTPDIR_FIX := glibc

CFLAGS := -g -O2 -Wall -Werror -std=gnu99

MODULES :=
MODULES_CLEAN :=

TARGET := hid_update

INCLUDE := -Iinclude

ifneq ($(COMPILE_TYPE), glibc)
	CFLAGS += -muclibc
	CLIBTPDIR_FIX:= uclibc
endif

ifeq ($(HAA_EN), 1)
	CFLAGS += -DHAA_ON
	OBJS += ./hid_test.o 	\
		./hsb.o      	\
		./hou.o         \
		./haa.o


	LIBS +=	-Llib/iaac/$(CLIBTPDIR_FIX) -ljzdl \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lmxu_merge \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lmxu_objdetect \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lmxu_contrib \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lmxu_video \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lmxu_imgproc \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lmxu_core

	LIBS += -Llib/iaac/$(CLIBTPDIR_FIX) -liaac \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lfaceDet \
		-Llib/iaac/$(CLIBTPDIR_FIX) -lpersonDet_inf

	LIBS += -Llib/imp/uclibc -limp \
		-Llib/imp/uclibc -lalog \
		-Llib/imp/uclibc -lsysutils

else
	OBJS += ./hid_test.o 	\
		./hsb.o      	\
		./hou.o		

endif

LIBS += -lpthread -lm -ldl -lrt -lstdc++



CFLAGS += $(INCLUDE)

all: $(TARGET)

$(TARGET): $(OBJS) $(MODULES)
	$(CC) $(CFLAGS) $(INCLUDE) $(OBJS) -o $@ $(LIBS)
	$(STRIP) $@

%o:%c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

clean: $(MODULES_CLEAN)
	rm -rf $(TARGET) *.o
