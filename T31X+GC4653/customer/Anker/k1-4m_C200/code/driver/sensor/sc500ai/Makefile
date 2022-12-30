CROSS_COMPILE ?= mips-linux-gnu-

ISVP_ENV_KERNEL_DIR = $(PWD)/../../../platform/kernel

TO_2M ?= n 
TO_720 ?= n
KDIR := ${ISVP_ENV_KERNEL_DIR}

ISP_DRIVER_DIR = $(PWD)/../../tx-isp-t31/

ifeq (${TO_2M}, y)
	MODULE_NAME := sensor_sc500ai_hdr_t31
	$(MODULE_NAME)-objs := sc500ai_hdr.o
else

ifeq (${TO_720}, y)

	MODULE_NAME := sensor_sc500ai_720_t31
	$(MODULE_NAME)-objs := sc500ai_720.o
else
	MODULE_NAME := sensor_sc500ai_t31
	$(MODULE_NAME)-objs := sc500ai.o
endif
endif


KBUILD_EXTRA_SYMBOLS := $(ISP_DRIVER_DIR)/Module.symvers
EXTRA_CFLAGS += -I$(ISP_DRIVER_DIR)/include

all: modules

.PHONY: modules clean

obj-m := $(MODULE_NAME).o

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

clean:
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order
