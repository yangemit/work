###########
#toolchains
############
CROSS_COMPILE := mips-linux-gnu-
COMPILE_TYPE := uclibc

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

HOST := T31
MODULE_FACEZOOM :=enable
MODULE_TRACK := disable

###########
#dir
###########
TOP_DIR :=$(PWD)
SRC_DIR := $(TOP_DIR)/src
LIBS_DIR := $(TOP_DIR)/libs
UTILS_DIR := $(TOP_DIR)/utils
SDK_DIR := $(LIBS_DIR)/t31_sdk

LIB_PERSON_TRACKER_DIR := $(LIBS_DIR)/person_tracker
IAAC_DIR := $(LIB_PERSON_TRACKER_DIR)/Ingenic-IAAC/t31
IVS_DIR := $(LIB_PERSON_TRACKER_DIR)/Ingenic-IIAL/t31/IVS
MXU_DIR := $(LIB_PERSON_TRACKER_DIR)/Ingenic-IIAL/t31/MXU

export HOST COMPILE_TYPE

############
#modules
############

MODULES :=
MODULES_CLEAN :=

MODULES_PATH := $(SRC_DIR)/modules

MODULE_UCAMERA_DIR := $(MODULES_PATH)/module_ucamera
LIBS += -L$(MODULE_UCAMERA_DIR) -lucamera
MODULES += module_ucamera
MODULES_CLEAN += module_ucamera_clean

MODULE_AF_DIR := $(MODULES_PATH)/module_autofocus
LIBS += -L$(MODULE_AF_DIR) -laf
MODULES += module_af
MODULES_CLEAN += module_af_clean

#MODULE_LED_DIR := $(MODULES_PATH)/module_led
#LIBS += -L$(MODULE_LED_DIR) -lled
#MODULES += module_led
#MODULES_CLEAN += module_led_clean

MODULE_KEY_DIR := $(MODULES_PATH)/module_key
LIBS += -L$(MODULE_KEY_DIR) -lkey_gpio
MODULES += module_key
MODULES_CLEAN += module_key_clean


ifeq ($(MODULE_FACEZOOM), enable)
MODULE_FACEZOOM_DIR := $(MODULES_PATH)/module_facezoom
LIBS += -L$(MODULE_FACEZOOM_DIR) -lface_zoom
MODULES += module_facezoom
MODULES_CLEAN += module_facezoom_clean

CFLAGS += -DMODULE_FACEZOOM_ENABLE

LIBS +=  -L$(LIBS_DIR)/faceDet/lib/$(COMPILE_TYPE) -lfaceDet_inf \
	 -L$(LIBS_DIR)/faceDet/lib/$(COMPILE_TYPE) -ljzdl\
	 -L$(LIBS_DIR)/mxu/lib/$(COMPILE_TYPE) -lmxu_core -lmxu_imgproc -lmxu_contrib -lmxu_merge -lmxu_objdetect -lmxu_video \

endif

ifeq ($(MODULE_TRACK), enable)
MODULE_TRACK_DIR := $(MODULES_PATH)/module_track
LIBS += -L$(MODULE_TRACK_DIR) -ltrack
MODULES += module_track
MODULES_CLEAN += module_track_clean

MOTOR_PROCESS_DIR := $(MODULE_TRACK_DIR)/motor_process
LIBS += -L$(MOTOR_PROCESS_DIR) -lmotor_process

CFLAGS += -DMODULE_TRACK_ENABLE

ifeq ($(ALGO_AUTHORIZE), yes)
	CFLAGS += -DALGO_AUTHORIZE
	LIBS += -L$(IAAC_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -liaac
else
	LINK_TYPE = -Wl,-Bstatic
endif

LIBS +=	-L$(IVS_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lperson_tracker_inf \
	-L$(IVS_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -ljzdl \
	-L$(MXU_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lmxu_merge \
	-L$(MXU_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lmxu_objdetect \
	-L$(MXU_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lmxu_contrib \
	-L$(MXU_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lmxu_video \
	-L$(MXU_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lmxu_imgproc \
	-L$(MXU_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lmxu_core

# OSD_DEBUG := no
ifeq ($(OSD_DEBUG), yes)
	CFLAGS += -DOSD_CONTROL
endif

endif

UCAMCORE_DIR := $(MODULE_UCAMERA_DIR)/ucamcorelib/

OBJS :=$(SRC_DIR)/main.o \
	$(SRC_DIR)/common/imp-common.o \
	$(SRC_DIR)/common/load_config.o \
	$(SRC_DIR)/common/thread_process.o

CFLAGS += $(INCLUDES) -O2 -Wall -march=mips32r2 -std=gnu99 -Werror

ifeq ($(COMPILE_TYPE), uclibc)
	CFLAGS += -muclibc
	LDFLAG += -muclibc
endif

INCLUDES =	-I$(LIBS_DIR)/anticopy/include \
		-I$(LIBS_DIR)/ucamcore/include \
		-I$(SDK_DIR)/include \
		-I$(UTILS_DIR)/include \
		-I$(SRC_DIR)/include \
		-I$(SRC_DIR)/ \
		-I./src/modules/module_key/include/

LIBS += 	-L$(SDK_DIR)/lib/$(COMPILE_TYPE) -limp \
		-L$(SDK_DIR)/lib/$(COMPILE_TYPE) -lalog \
		-L$(SDK_DIR)/lib/$(COMPILE_TYPE) -lsysutils \
		-L$(LIBS_DIR)/ucamcore/lib/$(COMPILE_TYPE) -lusbcamera \
		-L$(LIBS_DIR)/anticopy/lib/$(COMPILE_TYPE) -lAntiCopy \
		-L./src/modules/module_key/lib -lm -lpng16 -lz

LIBS += -L$(UTILS_DIR)/ -lutils
#LIBS += -Wall -Wl,-gc-sections -lpthread -lm -lrt -lstdc++ -ldl

LIBS += -Wl,-Bdynamic -lpthread -lm -ldl -lrt -lstdc++

.PHONY: ucamcore ucamcore_clean utils utils_clean\
	motor_process motor_process_clean \
	$(MODULES) $(MODULES_CLEAN)

CFLAGS += $(INCLUDE)

TARGET := ucamera-t31

all: $(TARGET)

$(TARGET):$(OBJS) $(MODULES) ucamcore utils motor_process
	$(CC) $(CFLAGS) $(INCLUDE) $(OBJS) $(LDFLAG) -o $@ $(LIBS)
	$(STRIP) $@

module_ucamera:
	make -C $(MODULE_UCAMERA_DIR) OSD_DEBUG=$(OSD_DEBUG) MODULE_FACEZOOM=$(MODULE_FACEZOOM)

module_ucamera_clean:
	make -C $(MODULE_UCAMERA_DIR) clean

module_af:
	make -C $(MODULE_AF_DIR)

module_af_clean:
	make -C $(MODULE_AF_DIR) clean

#module_led:
#	make -C $(MODULE_LED_DIR)

#module_led_clean:
#	make -C $(MODULE_LED_DIR) clean
module_key:
	make -C $(MODULE_KEY_DIR)

module_key_clean:
	make -C $(MODULE_KEY_DIR) clean

ifeq ($(MODULE_FACEZOOM), enable)
module_facezoom:
	make -C $(MODULE_FACEZOOM_DIR) LIBS_DIR=$(LIBS_DIR)

module_facezoom_clean:
	make -C $(MODULE_FACEZOOM_DIR) clean
endif

ifeq ($(MODULE_TRACK), enable)
motor_process:
	make -C $(MOTOR_PROCESS_DIR) COMPILE_TYPE=$(COMPILE_TYPE)

motor_process_clean:
	make -C $(MOTOR_PROCESS_DIR) clean

module_track:
	make -C $(MODULE_TRACK_DIR) COMPILE_TYPE=$(COMPILE_TYPE) TOP_DIR=$(TOP_DIR) OSD_DEBUG=$(OSD_DEBUG)

module_track_clean:
	make -C $(MODULE_TRACK_DIR) clean
endif

ucamcore:
	make -C $(UCAMCORE_DIR)

ucamcore_clean:
	make -C $(UCAMCORE_DIR) clean

utils:
	make -C $(UTILS_DIR)

utils_clean:
	make -C $(UTILS_DIR) clean

%o:%c
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -o $@ -c $^

clean: ucamcore_clean $(MODULES_CLEAN)
	rm -rf $(TARGET) $(OBJS)

distclean: ucamcore_clean $(MODULES_CLEAN)
	rm -rf $(TARGET) $(OBJS)
