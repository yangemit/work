#
# (C) Copyright 2012
# Texas Instruments Incorporated - http://www.ti.com/
# Aneesh V <aneesh@ti.com>
#
# This file is released under the terms of GPL v2 and any later version.
# See the file COPYING in the root directory of the source tree for details.
#
# Based on common/Makefile.
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libspl.o

SOBJS-y	= start.o

COBJS-y += iboot.o
COBJS-$(CONFIG_SPL_MMC_SUPPORT) += sd_load.o
COBJS-$(CONFIG_SPL_SFC_NOR) += sfc_nor_load.o
COBJS-$(CONFIG_LZO) += lzo1x_decompress.o
COBJS-$(CONFIG_LZMA) += LzmaDec.o LzmaTools.o

COBJS-y += boot_uimage.o
COBJS-y += printf.o

COBJS	:= $(sort $(COBJS-y))
SRCS	:= $(START:.o=.S) $(SOBJS-y:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))
START	:= $(addprefix $(obj),$(START))

all:	$(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

#########################################################################
