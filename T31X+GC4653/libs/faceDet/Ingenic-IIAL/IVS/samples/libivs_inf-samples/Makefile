CONFIG_UCLIBC_BUILD=y
CUR_DIR=$(shell pwd)

SDK_DIR ?= /home_a/wlli/Falcon_release/Luban/solution/Kiva/libs/t31_sdk/
# SDK_DIR ?= $(ISVP_ENV_RELEASE_DIR_SDK)
SDK_INC_DIR = $(SDK_DIR)/include

IIAL_DIR ?= ${CUR_DIR}/../../../../Ingenic-IIAL
MXU_DIR ?= $(IIAL_DIR)/MXU
IVS_DIR ?= $(IIAL_DIR)/IVS
IVS_INC_DIR = $(IVS_DIR)/include

IAAC_DIR ?= ${CUR_DIR}/../../../../Ingenic-IAAC
IAAC_INC_DIR = $(IAAC_DIR)/include

CROSS_COMPILE ?= mips-linux-gnu-

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

INCLUDES = -I$(SDK_INC_DIR) -I$(IVS_INC_DIR) -I$(IAAC_INC_DIR)
CFLAGS = $(INCLUDES) -O2 -Wall -march=mips32r2
LDFLAG = -Wall -Wl,-gc-sections -lpthread -lm -lrt -lstdc++

ifeq ($(CONFIG_UCLIBC_BUILD), y)
CFLAGS += -muclibc
LDFLAG += -muclibc
endif

ifeq ($(CONFIG_UCLIBC_BUILD), y)
SDK_LIB_DIR = $(SDK_DIR)/lib/uclibc
IAAC_LIB_DIR = $(IAAC_DIR)/lib/uclibc
MXU_LIB_DIR = $(MXU_DIR)/lib/uclibc
IVS_LIB_DIR = $(IVS_DIR)/lib/uclibc
else
SDK_LIB_DIR = $(SDK_DIR)/lib/glibc
IAAC_LIB_DIR = $(IAAC_DIR)/lib/glibc
MXU_LIB_DIR = $(MXU_DIR)/lib/glibc
IVS_LIB_DIR = $(IVS_DIR)/lib/glibc
endif

LIBS = $(SDK_LIB_DIR)/libimp.a $(SDK_LIB_DIR)/libalog.a \
	   $(MXU_LIB_DIR)/libmxu_merge.a $(MXU_LIB_DIR)/libmxu_objdetect.a \
	   $(MXU_LIB_DIR)/libmxu_contrib.a $(MXU_LIB_DIR)/libmxu_video.a \
	   $(MXU_LIB_DIR)/libmxu_imgproc.a $(MXU_LIB_DIR)/libmxu_core.a \
	   -L$(IAAC_LIB_DIR) -liaac

SAMPLES =  sample-faceDet

all: $(SAMPLES)

sample-faceDet: sample-common.o sample-faceDet.o
	$(CC) -o $@ $^  -L$(IVS_LIB_DIR) -lfaceDet_inf -ljzdl $(LIBS) $(LDFLAG)
	$(STRIP) $@

%.o:%.c sample-common.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o *~

distclean: clean
	rm -f $(SAMPLES)
