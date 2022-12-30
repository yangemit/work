CROSS_COMPILE ?= mips-linux-gnu-

ifeq (${ISVP_ENV_KERNEL_DIR}, )
	ISVP_ENV_KERNEL_DIR = ../../kernel
endif

KDIR := ../../platform/kernel/
MODULE_NAME := usbcamera


$(MODULE_NAME)-objs := libuac.a libuvc.a ucam_driver.o
obj-m += $(MODULE_NAME).o

all: modules

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

install:
	cp ./libuac.a ./libdriver
	cp ./libuvc.a ./libdriver

clean:
	find . -name "*.o" | xargs rm -vf
	find . -name ".cmd" | xargs rm -vf
	find . -name "\.*cmd" | xargs rm -vf
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order
