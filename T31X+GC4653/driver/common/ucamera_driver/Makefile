CROSS_COMPILE ?= mips-linux-gnu-
HOST := T31
ACM := n

ISVP_ENV_KERNEL_DIR = $(PWD)/../../../../../platform/$(HOST)/kernel

KDIR := ${ISVP_ENV_KERNEL_DIR}
MODULE_NAME := usbcamera

ifeq ($(HOST), T31)
	$(MODULE_NAME)-objs := libuac.a libuvc.a ucam_driver.o
else
	$(MODULE_NAME)-objs := libuac.a libuvc.a ucam_driver.o u_f.o
endif

ifeq ($(ACM), y)
	$(MODULE_NAME)-objs += libacm.a
	EXTRA_CFLAGS := -DINCLUDE_ACM_INTERFACE
endif

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
