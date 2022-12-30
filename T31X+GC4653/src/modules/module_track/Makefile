CROSS_COMPILE := mips-linux-gnu-
CC := $(CROSS_COMPILE)gcc
CPP := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
AR := $(CROSS_COMPILE)ar

ARFLAG := -rcs

HOST_SOC := t31
COMPILE_TYPE := glibc
LINK_TYPE :=
OSD_DEBUG := no
ALGO_AUTHORIZE := no

# dir
TOP_DIR := $(PWD)/../../../
#SOURCE_DIR := $(TOP_DIR)/src/modules/module_track
LIB_DIR := $(TOP_DIR)/libs/
SDK_DIR := $(LIB_DIR)/t31_sdk
UTILS_DIR := $(TOP_DIR)/utils
LIB_PERSON_TRACKER_DIR := $(LIB_DIR)/person_tracker
IAAC_DIR := $(LIB_PERSON_TRACKER_DIR)/Ingenic-IAAC/$(HOST_SOC)
IVS_DIR := $(LIB_PERSON_TRACKER_DIR)/Ingenic-IIAL/$(HOST_SOC)/IVS
MXU_DIR := $(LIB_PERSON_TRACKER_DIR)/Ingenic-IIAL/$(HOST_SOC)/MXU

CFLAGS := -g -O2 -Wall -Werror -std=gnu99
CFLAGS += -D_GNU_SOURCE

MODULES :=
MODULES_CLEAN :=

MOTOR_PROCESS_DIR := ./motor_process
LIBS += -L$(MOTOR_PROCESS_DIR) -lmotor_process
MODULES += motor_process
MODULES_CLEAN += module_process_clean

ifeq ($(OSD_DEBUG), yes)
	CFLAGS += -DOSD_CONTROL
endif

ifneq ($(COMPILE_TYPE), glibc)
	CFLAGS += -muclibc
endif

ifeq ($(HOST_SOC), t31)
	CFLAGS += -DSOC_T31
endif

# ifeq ($(ALGO_AUTHORIZE), yes)
# 	CFLAGS += -DALGO_AUTHORIZE
# 	LIBS += -L$(IAAC_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -liaac
# else
# 	LINK_TYPE = -Wl,-Bstatic
# endif

# 	LIBS +=	-L$(IVS_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lperson_tracker_inf \
# 		-L$(IVS_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -ljzdl \
# 		-L$(MXU_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lmxu_merge \
# 		-L$(MXU_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lmxu_objdetect \
# 		-L$(MXU_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lmxu_contrib \
# 		-L$(MXU_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lmxu_video \
# 		-L$(MXU_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lmxu_imgproc \
# 		-L$(MXU_DIR)/lib/$(COMPILE_TYPE) $(LINK_TYPE) -lmxu_core

OBJS := module_track_control.o \
	motor_control.o

ifeq ($(OSD_DEBUG), yes)
OBJS += ./osd/render.o
endif

TARGET := libtrack.a

INCLUDE := -I./include \
	   -I$(IAAC_DIR)/include \
	   -I$(IVS_DIR)/include \
	   -I$(MXU_DIR)/include \
	   -I$(UTILS_DIR)/include \

INCLUDE += -I$(SDK_DIR)/include \
	   -I../include \
	   -I../../include/ \
	   -I../../ \
	   -I.

LIBS += -L../../utils/ -lutils


#LIBS += -Wl,-Bdynamic -lpthread -lm -ldl -lrt -lstdc++
LIBS += -lpthread -lm -ldl -lrt -lstdc++

.PHONY: $(MODULES) $(MODULES_CLEAN)

CFLAGS += $(INCLUDE)

all: $(MODULES) $(TARGET)

$(TARGET): $(OBJS)
	$(AR) $(ARFLAG) $@ $^

#motor_process:
#	make -C $(MOTOR_PROCESS_DIR) COMPILE_TYPE=$(COMPILE_TYPE)

#motor_process_clean:
#	make -C $(MOTOR_PROCESS_DIR) clean

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) $(LIBS) -o $@ -c $^

%.o:%.cc
	$(CPP) $(CFLAGS) -lstdc++ $(INCLUDE) $(LIBS) -o $@ -c $^

clean: $(MODULES_CLEAN)
	rm -rf $(TARGET) $(OBJS)
