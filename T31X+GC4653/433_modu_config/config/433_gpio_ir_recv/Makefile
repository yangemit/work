ARCH := mips
CROSS_COMPILE := mips-linux-uclibc-gnu-
KERNEL_DIR := ../../../../../../platform/T31/kernel/
KSRC := $(KERNEL_DIR)
MODPATH := $(shell pwd)
DRIVER_NAME := gpio-ir-recv


obj-m += $(DRIVER_NAME).o

modules:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(MODPATH) modules

.PHONE:clean
clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(MODPATH) clean
