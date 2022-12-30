/*
 * Ingenic T01 configuration
 *
 * Copyright (c) 2015 Ingenic Semiconductor Co.,Ltd
 * Author: Elvis <huan.wang@ingenic.com>
 * Based on: include/configs/urboard.h
 *           Written by Paul Burton <paul.burton@imgtec.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_T01_H__
#define __CONFIG_T01_H__

/**
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32		/* MIPS32 CPU core */
#define CONFIG_CPU_XBURST
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_T01		/* T01 SoC */
#define CONFIG_FPGA		/* Jz_t01 FPGA */
#define CONFIG_SOC_NAME		t01
/*#define CONFIG_DDR_AUTO_SELF_REFRESH
#define CONFIG_SPL_DDR_SOFT_TRAINING*/


#define CONFIG_SYS_APLL_FREQ		1200000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_CPU_SEL_PLL		APLL
#define CONFIG_DDR_SEL_PLL		MPLL
#define CONFIG_SYS_CPU_FREQ		1200000000
#define CONFIG_SYS_MEM_FREQ		30000000

#define CONFIG_SYS_EXTAL		24000000	/* EXTAL freq: 24 MHz */
#define CONFIG_SYS_HZ			1000		/* incrementer freq */

#define CONFIG_SYS_DCACHE_SIZE		32768
#define CONFIG_SYS_ICACHE_SIZE		32768
#define CONFIG_SYS_CACHELINE_SIZE	32

/*
 *#define CONFIG_DDR_TEST_CPU
 *#define CONFIG_DDR_TEST
 */
#define CONFIG_DDR_DLL_OFF
#define CONFIG_DDR_TYPE_DDR3
#define CONFIG_DDR_PARAMS_CREATOR
#define CONFIG_DDR_HOST_CC
#define CONFIG_DDR_FORCE_SELECT_CS1
#define CONFIG_DDR_CS0          1   /* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1          0   /* 1-connected, 0-disconnected */
#define CONFIG_DDR_DW32         0   /* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDR3_TSD34096M1333C9_E

/* #define CONFIG_DDR_DLL_OFF */
/*
 * #define CONFIG_DDR_CHIP_ODT
 * #define CONFIG_DDR_PHY_ODT
 * #define CONFIG_DDR_PHY_DQ_ODT
 * #define CONFIG_DDR_PHY_DQS_ODT
 * #define CONFIG_DDR_PHY_IMPED_PULLUP		0xe
 * #define CONFIG_DDR_PHY_IMPED_PULLDOWN	0xe
 */

/**
 * Boot arguments definitions.
 */
#define BOOTARGS_COMMON "console=ttyS0,115200n8 mem=256M@0x0 mem=256M@0x30000000"

#ifdef CONFIG_SPL_MMC_SUPPORT
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc root=/dev/mmcblk0p2 rw rootdelay=1"
#elif CONFIG_SFC_NOR
	#define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc rootfstype=squashfs root=/dev/mtdblock2 rw mtdparts=jz_sfc:256k(boot),2816k(kernel),2560k(root),-(appfs)"
#endif

/**
 * Boot command definitions.
 */
#define CONFIG_BOOTDELAY 1

#ifdef CONFIG_SPL_MMC_SUPPORT
#define CONFIG_BOOTCOMMAND "mmc read 0x80600000 0x1800 0x3000; bootm 0x80600000"
#endif

#ifdef CONFIG_SFC_NOR
	#define CONFIG_BOOTCOMMAND "sf probe;sf read 0x80600000 0x40000 0x2c0000; bootm 0x80600000"
#endif /* CONFIG_SFC_NOR */


/* SFC */
#if defined(CONFIG_SPL_SFC_SUPPORT)
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_JZ_SFC_PA_6BIT
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_NAND_BLOCK_SIZE * 5)
#ifdef CONFIG_SPI_NAND
#define CONFIG_SPI_NAND_BPP			(2048 +64)	/*Bytes Per Page*/
#define CONFIG_SPI_NAND_PPB			(64)		/*Page Per Block*/
#endif /* CONFIG_SPI_NAND */
#endif /* CONFIG_SPL_SFC_SUPPORT */
/* END SFC */

/* MMC */
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_JZ_MMC 1

#ifdef CONFIG_JZ_MMC_MSC0
#define CONFIG_JZ_MMC_SPLMSC 0
#define CONFIG_JZ_MMC_MSC0_PB_4BIT 1
#endif
#ifdef CONFIG_JZ_MMC_MSC1
#define CONFIG_JZ_MMC_SPLMSC 1
#define CONFIG_JZ_MMC_MSC1_PC 1
#endif

/* I2C */
#define CONFIG_SOFT_I2C
#define CONFIG_SYS_I2C_SPEED		50     /* the function is not implemented */
#define CONFIG_SYS_I2C_SLAVE		0x00   /* the function is not implemented */

#define CONFIG_SOFT_I2C_GPIO_SCL	GPIO_PD(31)
#define CONFIG_SOFT_I2C_GPIO_SDA	GPIO_PD(30)

/* GPIO */
#define CONFIG_JZ_GPIO

/*eeprom*/
#ifdef CONFIG_CMD_EEPROM
#define CONFIG_SYS_I2C_EEPROM_ADDR  0x50
/*#define CONFIG_ENV_EEPROM_IS_ON_I2C*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#endif

/* USB */
/*#define CONFIG_CMD_FASTBOOT*/
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_JZ_DWC2_UDC_V1_1
#define CONFIG_FASTBOOT_GADGET
#define CONFIG_FASTBOOT_FUNCTION
#define CONFIG_G_FASTBOOT_VENDOR_NUM	(0x18d1)
#define CONFIG_G_FASTBOOT_PRODUCT_NUM	(0xdddd)
#define CONFIG_USB_GADGET_VBUS_DRAW 500

/**
 * Miscellaneous configurable options
 */
#define CONFIG_DOS_PARTITION

#define CONFIG_SYS_FLASH_BASE	0 /* init flash_base as 0 */

#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "# "
#define CONFIG_SYS_CBSIZE 1024 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_MONITOR_LEN		(768 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(64 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 * 1024)

/*#define DBG_USE_UNCACHED_MEMORY*/
#ifdef DBG_USE_UNCACHED_MEMORY
#define CONFIG_SYS_SDRAM_BASE		0xA0000000 /* uncached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP	0xB0000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0xA8000000
#define CONFIG_SYS_MEMTEST_START	0xA0000000
#define CONFIG_SYS_MEMTEST_END		0xA8000000

#define CONFIG_SYS_TEXT_BASE		0xA0100000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#else /* DBG_USE_UNCACHED_MEMORY */
#define CONFIG_SYS_SDRAM_BASE		0x80000000 /* cached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP	0x90000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0x88000000
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x88000000

#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#endif /* DBG_USE_UNCACHED_MEMORY */

/**
 * Environment
 */
#ifdef  CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE         (32 << 10)
#endif

/**
 * SPL configuration
 */
#define CONFIG_SPL_NO_CPU_SUPPORT_CODE

#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-nor-spl.lds"
#define CONFIG_SPL_TEXT_BASE		0xba000000
#else
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-spl.lds"
#define CONFIG_SPL_TEXT_BASE		0x80001000
#endif

/**
 * serial config
 **/
#define CONFIG_SPL_SERIAL_SUPPORT

#define SERIAL_BANDRATE_921600

#ifdef SERIAL_BANDRATE_921600
#define CONFIG_SYS_UART_INDEX		1
#else /* SERIAL_BANDRATE_921600 */
#define CONFIG_SYS_UART_INDEX		0
#define CONFIG_BAUDRATE			115200
#endif /* SERIAL_BANDRATE_921600 */

/**
 * boot uImage configs
 **/
#define MAX_U_IMAGE_LIMIT	(1280 * 1024) // 1.25MB

#endif /* __CONFIG_T01_H__ */
