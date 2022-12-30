HOST_SOC := t31
CUSTOMER :=
ifeq ($(HOST_SOC), t31)
	ENV_KERNEL_DIR = ../../../../platform/T31/kernel
	EXTRA_CFLAGS += -DSOC_T31
else
	ENV_KERNEL_DIR = ../../../../platform/T20/kernel
	EXTRA_CFLAGS += -DSOC_T20
endif

DEFS =
DEFS += -DCONFIG_GPIO_SW1

ifeq ($(CUSTOMER), jpts)
DEFS += -DCONFIG_GPIO_SW2
endif
EXTRA_CFLAGS += $(DEFS)

KDIR := ${ENV_KERNEL_DIR}
MODULE_NAME := key_board

all: modules

.PHONY: modules clean

$(MODULE_NAME)-objs := gpio_keys.o

obj-m := $(MODULE_NAME).o

modules:
	@$(MAKE) -C $(KDIR) M=$(shell pwd) $@

clean:
	find . -name "*.o" | xargs rm -vf
	find . -name ".cmd" | xargs rm -vf
	find . -name "\.*cmd" | xargs rm -vf
	@rm -rf *.o *~ .depend .*.cmd  *.mod.c .tmp_versions *.ko *.symvers modules.order
