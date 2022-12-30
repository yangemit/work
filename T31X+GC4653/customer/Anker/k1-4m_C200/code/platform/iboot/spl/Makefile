#
# (C) Copyright 2000-2011
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2011
# Daniel Schwierzeck, daniel.schwierzeck@googlemail.com.
#
# (C) Copyright 2011
# Texas Instruments Incorporated - http://www.ti.com/
# Aneesh V <aneesh@ti.com>
#
# This file is released under the terms of GPL v2 and any later version.
# See the file COPYING in the root directory of the source tree for details.
#
# Based on top-level Makefile.
#

CURDIR := $(shell pwd)

CONFIG_SPL_BUILD := y
export CONFIG_SPL_BUILD

OBJCOPY = mips-linux-gnu-objcopy
LD = mips-linux-gnu-ld.bfd

include $(TOPDIR)/config.mk
CPUDIR := arch/mips/cpu/$(CPU)

LDFLAGS =  -G 0 -static -n -nostdlib -EL -m elf32ltsmip \
	-T $(obj)u-boot-spl.lds --gc-sections --gc-sections -Bstatic
OBJCFLAGS := --remove-section=.dynsym --gap-fill=0xff

# We want the final binaries in this directory
obj := $(OBJTREE)/spl/

LIBS-y += $(CPUDIR)/lib$(CPU).o
LIBS-y += $(CPUDIR)/$(SOC)/lib$(SOC).o
LIBS-y += src/libspl.o
LIBS-y += lib/libgeneric.o
LIBS-y += lib/zlib/libz.o

LIBS := $(addprefix $(SPLTREE)/,$(sort $(LIBS-y)))

__LIBS := $(subst $(obj),,$(LIBS))

ALL-y	+= $(obj)u-boot-spl.bin

all:	$(ALL-y)

$(obj)u-boot-spl.bin:	$(obj)u-boot-spl
	$(OBJCOPY) $(OBJCFLAGS) -O binary $< $@
	$(OBJTREE)/tools/ingenic-tools/spi_checksum $@

$(obj)u-boot-spl: $(LIBS) $(obj)u-boot-spl.lds
	cd $(obj) && $(LD) $(LDFLAGS) --start-group $(__LIBS) --end-group -Map u-boot-spl.map -o u-boot-spl

$(LIBS):
	$(MAKE) -C $(SRCTREE)$(dir $(subst $(SPLTREE),,$@))