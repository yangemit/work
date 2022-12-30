CROSS_COMPILE ?= mips-linux-gnu-

HOST_SOC := t31
ifeq ($(HOST_SOC), t31)
	ISVP_ENV_KERNEL_DIR = ../../../../platform/T31/kernel
	EXTRA_CFLAGS += -DSOC_T31
else
	ISVP_ENV_KERNEL_DIR = ../../../../platform/T20/kernel
	EXTRA_CFLAGS += -DSOC_T20
endif

KDIR := ${ISVP_ENV_KERNEL_DIR}
MODULE_NAME := step_motor

all: modules

.PHONY: modules clean

$(MODULE_NAME)-objs := motor.o
obj-m := $(MODULE_NAME).o

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

clean:
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order
