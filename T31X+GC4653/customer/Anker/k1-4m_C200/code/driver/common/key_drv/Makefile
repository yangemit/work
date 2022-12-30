CROSS_COMPILE ?= mips-linux-gnu-

KDIR := ../../../platform/kernel

MODULE_NAME := key_drv
$(MODULE_NAME)-objs := gpio_key_drv.o

all: modules

.PHONY: modules clean

obj-m += $(MODULE_NAME).o

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

clean:
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order
