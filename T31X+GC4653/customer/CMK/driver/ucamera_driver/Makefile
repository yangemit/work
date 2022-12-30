CROSS_COMPILE ?= mips-linux-gnu-

ISVP_ENV_KERNEL_DIR = ../../kernel-cmk
KDIR := ${ISVP_ENV_KERNEL_DIR}
MODULE_NAME := usbcamera


$(MODULE_NAME)-objs := libuac.a libuvc.a ucam_driver.o
obj-m += $(MODULE_NAME).o

all: modules

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

cpy:
	scp $(MODULE_NAME).ko 193.169.4.110:/home/ppchen/nfs

clean:
	find . -name "*.o" | xargs rm -vf
	find . -name ".cmd" | xargs rm -vf
	find . -name "\.*cmd" | xargs rm -vf
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order
