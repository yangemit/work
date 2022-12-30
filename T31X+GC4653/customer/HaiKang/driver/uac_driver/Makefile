CROSS_COMPILE ?= mips-linux-gnu-
HOST := T31

#ISVP_ENV_KERNEL_DIR = $(PWD)/../../../../platform/$(HOST)/kernel
ISVP_ENV_KERNEL_DIR = $(PWD)/../../platform/kernel

KDIR := ${ISVP_ENV_KERNEL_DIR}
MODULE_NAME := libuac

$(MODULE_NAME)-objs := f_uac1.o utils/node_list.o

obj-m += $(MODULE_NAME).o

all: modules cpy

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

cpy:
	cp $(MODULE_NAME).o ../usbcamera/libuac.a

clean:
	find . -name "*.o" | xargs rm -vf
	find . -name ".cmd" | xargs rm -vf
	find . -name "\.*cmd" | xargs rm -vf
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order
