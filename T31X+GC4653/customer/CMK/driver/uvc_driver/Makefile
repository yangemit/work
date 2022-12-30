CROSS_COMPILE ?= mips-linux-gnu-
ISVP_ENV_KERNEL_DIR = ../../kernel-cmk
KDIR := ${ISVP_ENV_KERNEL_DIR}
MODULE_NAME := libuvc


$(MODULE_NAME)-objs := webcam.o

obj-m += $(MODULE_NAME).o

all: modules

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

cpy:
	cp $(MODULE_NAME).o ../ucamera_driver/libuvc.a

clean:
	find . -name "*.o" | xargs rm -vf
	find . -name ".cmd" | xargs rm -vf
	find . -name "\.*cmd" | xargs rm -vf
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order
