CONFIG_UCLIBC_BUILD=y
CUR_DIR=$(shell pwd)

# SDK_DIR ?= $(ISVP_ENV_RELEASE_DIR_SDK)
SDK_DIR := $(CUR_DIR)/../../../../../sdk/t31/
SDK_INC_DIR = $(SDK_DIR)/include/sdk

# IIAL_DIR ?= ${CUR_DIR}/../../../../../Ingenic-IIAL/t31
MXU_DIR ?= $(CUR_DIR)/../../../MXU/
IVS_DIR ?= $(CUR_DIR)/../../../IVS
IVS_INC_DIR = $(IVS_DIR)/include

IAAC_DIR ?= $(CUR_DIR)/../../../../../Ingenic-IAAC/t31
IAAC_INC_DIR = $(IAAC_DIR)/include

CROSS_COMPILE ?= mips-linux-gnu-

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

INCLUDES = -I$(SDK_INC_DIR) -I$(IVS_INC_DIR) -I$(IAAC_INC_DIR)
CFLAGS = $(INCLUDES) -O2 -Wall -march=mips32r2 -Werror
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
	   -L$(MXU_LIB_DIR) -lmxu_merge \
	   -L$(MXU_LIB_DIR) -lmxu_objdetect \
	   -L$(MXU_LIB_DIR) -lmxu_contrib \
	   -L$(MXU_LIB_DIR) -lmxu_video \
	   -L$(MXU_LIB_DIR) -lmxu_imgproc \
	   -L$(MXU_LIB_DIR) -lmxu_core \
	   -L$(IAAC_LIB_DIR) -liaac

SAMPLES =  sample-person-tracker

all: $(SAMPLES)

sample-person-tracker: sample-common.o sample-person-tracker.o
	$(CC) -o $@ $^  -L$(IVS_LIB_DIR) -lperson_tracker_inf -ljzdl $(LIBS) $(LDFLAG)
	$(STRIP) $@

%.o:%.c sample-common.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o *~

distclean: clean
	rm -f $(SAMPLES)
