CROSS_COMPILE := mips-linux-gnu-
CC := $(CROSS_COMPILE)gcc
CPP := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
AR := $(CROSS_COMPILE)ar

ARFLAG := -rcs

HOST_SOC := t31
LIBTYPE := uclibc
CLIBTPDIR_FIX := glibc
CLIBTPSTR := LIBTYPE=glibc
LINK_TYPE :=
OSD_DEBUG := yes
# OSD_DEBUG := no
ALGO_AUTHORIZE := yes
# ALGO_AUTHORIZE := no

# dir
TOP_DIR := $(PWD)/../..
SOURCE_DIR := $(TOP_DIR)/src/module_faceAE
LIB_DIR := $(TOP_DIR)/libs
SDK_DIR := $(LIB_DIR)/t31_sdk
UTILS_DIR := $(TOP_DIR)/utils
LIB_PERSON_TRACKER_DIR := $(LIB_DIR)/faceDet
IAAC_DIR := $(LIB_PERSON_TRACKER_DIR)/Ingenic-IAAC
IVS_DIR := $(LIB_PERSON_TRACKER_DIR)/Ingenic-IIAL/IVS
MXU_DIR := $(LIB_PERSON_TRACKER_DIR)/Ingenic-IIAL/MXU

#export env
export LIBTYPE HOST_SOC OSD_DEBUG

CFLAGS := -g -O2 -Wall -std=gnu99
# CFLAGS := -g -O2 -Wall -Werror -std=gnu99
CFLAGS += -D_GNU_SOURCE

CFLAGS += -DMODULE_FACEAE_ENABLE

ifeq ($(OSD_DEBUG), yes)
	CFLAGS += -DOSD_CONTROL
endif

ifneq ($(LIBTYPE), glibc)
	CFLAGS += -muclibc
	CLIBTPDIR_FIX := uclibc
	CLIBTPSTR = LIBTYPE=uclibc
endif

ifeq ($(HOST_SOC), t31)
	CFLAGS += -DSOC_T31
endif


OBJS := $(SOURCE_DIR)/module_faceAE.o

ifeq ($(OSD_DEBUG), yes)
OBJS += $(SOURCE_DIR)/osd/render.o \
	$(SOURCE_DIR)/osd/osd_show.o
endif

TARGET := libmodule_faceAE.a

INCLUDE := -I$(SOURCE_DIR)/include \
	   -I$(IAAC_DIR)/include \
	   -I$(IVS_DIR)/include \
	   -I$(MXU_DIR)/include \
	   -I$(UTILS_DIR)/include \
	   -I$(SOURCE_DIR)

INCLUDE += -I$(SDK_DIR)/include \
	   -I../include \
	   -I./include

LIBS += -L../../utils/ -lutils

ifeq ($(ALGO_AUTHORIZE), yes)
	CFLAGS += -DALGO_AUTHORIZE
	LIBS += -L$(IAAC_DIR)/lib/$(CLIBTPDIR_FIX) $(LINK_TYPE) -liaac
else
	LINK_TYPE = -Wl,-Bstatic
endif

	LIBS +=	-L$(IVS_DIR)/lib/$(CLIBTPDIR_FIX) $(LINK_TYPE) -lfaceDet_inf \
		-L$(IVS_DIR)/lib/$(CLIBTPDIR_FIX) $(LINK_TYPE) -ljzdl \
		-L$(MXU_DIR)/lib/$(CLIBTPDIR_FIX) $(LINK_TYPE) -lmxu_merge \
		-L$(MXU_DIR)/lib/$(CLIBTPDIR_FIX) $(LINK_TYPE) -lmxu_objdetect \
		-L$(MXU_DIR)/lib/$(CLIBTPDIR_FIX) $(LINK_TYPE) -lmxu_contrib \
		-L$(MXU_DIR)/lib/$(CLIBTPDIR_FIX) $(LINK_TYPE) -lmxu_video \
		-L$(MXU_DIR)/lib/$(CLIBTPDIR_FIX) $(LINK_TYPE) -lmxu_imgproc \
		-L$(MXU_DIR)/lib/$(CLIBTPDIR_FIX) $(LINK_TYPE) -lmxu_core


LIBS += -Wl,-Bdynamic -lpthread -lm -ldl -lrt -lstdc++
.PHONY:	utils utils_clean \

CFLAGS += $(INCLUDE)

all: utils $(TARGET)

$(TARGET): $(OBJS)
	$(AR) $(ARFLAG) $@ $^

utils:
	make -C $(UTILS_DIR) $(CLIBTPSTR)

utils_clean:
	make -C $(UTILS_DIR) clean

%o:%c
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $^

clean: utils_clean
	rm -rf $(TARGET) $(OBJS)
