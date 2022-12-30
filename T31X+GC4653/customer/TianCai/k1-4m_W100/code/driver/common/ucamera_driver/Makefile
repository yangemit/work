CROSS_COMPILE ?= mips-linux-gnu-
HOST := T31

ISVP_ENV_KERNEL_DIR = $(PWD)/../../../../../../../../../platform/$(HOST)/kernel

KDIR := ${ISVP_ENV_KERNEL_DIR}
MODULE_NAME := usbcamera

export HOST

ifeq ($(HOST), T31)
	$(MODULE_NAME)-objs := libuac.a libuvc.a ucam_driver.o 
else
	$(MODULE_NAME)-objs := libuac.a libuvc.a ucam_driver.o u_f.o 
endif

obj-m += $(MODULE_NAME).o

all: modules

modules: uac uvc
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

uac:
	make -C ../uac_driver/	
uvc:
	make -C ../uvc_driver/	

uac_clean:
	make clean -C  ../uac_driver/	
uvc_clean:
	make clean -C ../uvc_driver/	

clean: uac_clean uvc_clean
	find . -name "*.o" | xargs rm -vf
	find . -name "*.a" | xargs rm -vf
	find . -name ".cmd" | xargs rm -vf
	find . -name "\.*cmd" | xargs rm -vf
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order
