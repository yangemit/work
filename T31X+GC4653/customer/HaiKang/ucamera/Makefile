CONFIG_UCLIBC_BUILD=y

CROSS_COMPILE ?= mips-linux-gnu-

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

CONFIG_ANTICOPY=y
EXEC := ucamera
OBJS := imp-common.o \
	ucamera_sample.o \
	isp_3a.o \
	isp_global_object.o \
	isp_inner.o \
	isp_init.o \
	vcm_controler.o \
	tof_controler.o	\
	pwm_controler.o

OBJS += ./xu_controler.o
CFLAGS = $(INCLUDES) -O2 -Wall -march=mips32r2 -g

ifeq ($(CONFIG_UCLIBC_BUILD), y)
CFLAGS += -muclibc
LDFLAG += -muclibc
endif


SDK_LIB_DIR = ./lib/imp
UCAM_LIB_DIR = ./lib/ucam
ANTI_LIB_DIR = ./lib/anticopy
HIK_ISP_DIR = ./lib/hikisp
JX_DIR = ./lib/jx

SDK_INC_DIR = ./include

INCLUDES = -I$(SDK_INC_DIR) \
	   -I./utils

LIBS := $(SDK_LIB_DIR)/libimp.a \
		$(SDK_LIB_DIR)/libalog.a \
		$(UCAM_LIB_DIR)/libusbcamera.a \
		$(HIK_ISP_DIR)/libawb_kernel.a \
		$(HIK_ISP_DIR)/libjz_ccm.a \
		$(JX_DIR)/libaf.a

ifeq ($(CONFIG_ANTICOPY), y)
LIBS += $(ANTI_LIB_DIR)/libAntiCopy.a

endif

LDFLAG += -Wall -Wl,-gc-sections -lpthread -lm -lrt -lstdc++ -ldl -g

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LIBS) $(LDFLAG)
	$(STRIP) $@

clean:
	rm -f *.o *~
	rm -fr $(EXEC)

