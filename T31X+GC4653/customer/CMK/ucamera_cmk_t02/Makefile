CONFIG_UCLIBC_BUILD=y

CROSS_COMPILE ?= mips-linux-gnu-

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

CONFIG_ADK=n
CONFIG_ANTICOPY=y
EXEC := ucamera
OBJS := imp-common.o \
             mtd_flash.o \
    gpio.o \
   conf.o \
	pwm.o \
	motor_app.o \
	ucamera_sample.o



CFLAGS = $(INCLUDES) -O2 -Wall -march=mips32r2

ifeq ($(CONFIG_UCLIBC_BUILD), y)
CFLAGS += -muclibc
LDFLAG += -muclibc
endif


SDK_LIB_DIR = ./lib/imp
UCAM_LIB_DIR = ./lib/ucam
ANTI_LIB_DIR = ./lib/anticopy

ifeq ($(CONFIG_ADK), y)
IVS_LIB_DIR = ./lib/ivs
MXU_LIB_DIR = ./lib/mxu

IVS_INC_DIR = ./include
endif
SDK_INC_DIR = ./include

INCLUDES = -I$(SDK_INC_DIR) \
	   -I./utils

LIBS := $(SDK_LIB_DIR)/libimp.a \
		$(SDK_LIB_DIR)/libalog.a \
		$(UCAM_LIB_DIR)/libusbcamera.a \
                                $(SDK_LIB_DIR)/libsysutils.a 

ifeq ($(CONFIG_ADK), y)
LIBS += -L$(IVS_LIB_DIR) -liaac \
		-L$(IVS_LIB_DIR) -ljzdl \
		-L$(IVS_LIB_DIR) -lcustpersonDet_inf

LIBS += $(MXU_LIB_DIR)/libmxu_merge.a \
		$(MXU_LIB_DIR)/libmxu_objdetect.a \
		$(MXU_LIB_DIR)/libmxu_contrib.a \
		$(MXU_LIB_DIR)/libmxu_video.a \
		$(MXU_LIB_DIR)/libmxu_imgproc.a \
		$(MXU_LIB_DIR)/libmxu_core.a
endif

ifeq ($(CONFIG_ANTICOPY), y)
LIBS += $(ANTI_LIB_DIR)/libAntiCopy.a

endif

LDFLAG += -Wall -Wl,-gc-sections -lpthread -lm -lrt -lstdc++ -ldl

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LIBS) $(LDFLAG)
	$(STRIP) $@

clean:
	rm -f *.o *~
	rm $(EXEC)

