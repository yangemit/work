#
# Ingenic tools makefile
#
# Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
# Author: Zoro <ykli@ingenic.cn>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

HOSTCPPFLAGS = -I$(TOPDIR)/include -DUSE_HOSTCC

DDR-$(CONFIG_DDR_PARAMS_CREATOR)	+= ddr_params_creator ddr_reg_values.h
SPI-$(CONFIG_SPI_SPL_CHECK)		+= spi_checksum

TOOLS	= $(DDR-y) $(SPI-y) $(SPL-y)
TOOLS	:= $(addprefix $(obj),$(TOOLS))

all:	$(obj).depend $(TOOLS)

DDR-TYPE-y = ddr_params_creator.o
DDR-TYPE-$(CONFIG_DDR_TYPE_LPDDR2) += lpddr2_params.o
DDR-TYPE-$(CONFIG_DDR_TYPE_LPDDR) += lpddr_params.o
DDR-TYPE-$(CONFIG_DDR_TYPE_DDR2) += ddr2_params.o
DDR-TYPE-$(CONFIG_DDR_TYPE_DDR3) += ddr3_params.o

define PRIVATE_BUILD
$(1):$(2) $(3)
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -c -o $(1) $(2)
endef

ALL_INCLUDE_PRIVATE := $(SRCTREE)/include/configs/$(BOARD).h ddr_params_creator.h
all:	$(obj).depend $(TOOLS)

$(obj)ddr_params_creator: $(DDR-TYPE-y)
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

$(foreach f,$(DDR-TYPE-y), \
	$(eval $(call PRIVATE_BUILD,$(f),$(f:.o=.c),$(ALL_INCLUDE_PRIVATE))))

$(obj)ddr_reg_values.h:	$(obj)ddr_params_creator
	$(OBJTREE)/tools/ingenic-tools/ddr_params_creator > $(OBJTREE)/include/generated/$@

$(obj)spi_checksum:	spi_checksum.c
	$(HOSTCC) $(HOSTCFLAGS_NOPED) $(HOSTLDFLAGS) -o $@ $^
	$(HOSTSTRIP) $@

clean:
	rm -f $(TOOLS) $(obj).depend

distclean:	clean

#########################################################################
# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
