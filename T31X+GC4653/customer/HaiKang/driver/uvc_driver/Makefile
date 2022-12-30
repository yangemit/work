CROSS_COMPILE ?= mips-linux-gnu-


#ISVP_ENV_KERNEL_DIR = ../../../../../../platform/T31/kernel
ISVP_ENV_KERNEL_DIR = ../../platform/kernel
#ISVP_ENV_KERNEL_DIR = /home_a/jdong/ISVP-T31-1.1.1-20200508/software/sdk/Ingenic-SDK-T31-1.1.1-20200508/opensource/kernel-hk

KDIR := ${ISVP_ENV_KERNEL_DIR}
MODULE_NAME := libuvc

$(MODULE_NAME)-objs := webcam.o

obj-m += $(MODULE_NAME).o

all: modules cpy

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

cpy:
	cp $(MODULE_NAME).o ../usbcamera/libuvc.a

clean:
	find . -name "*.o" | xargs rm -vf
	find . -name ".cmd" | xargs rm -vf
	find . -name "\.*cmd" | xargs rm -vf
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order
