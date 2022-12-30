ARCH := mips
CROSS_COMPILE := mips-linux-gnu-
MODPATH := $(shell pwd)
DRIVER_NAME := gsensor-dw9714
HOST := T31
KERNEL_DIR :=$(PWD)/../../../../../../../../../platform/$(HOST)/kernel

KSRC := $(KERNEL_DIR)


obj-m := $(DRIVER_NAME).o
$(DRIVER_NAME)-y += dw9714.o

modules:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(MODPATH) modules

.PHONE:clean
clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(MODPATH) clean
