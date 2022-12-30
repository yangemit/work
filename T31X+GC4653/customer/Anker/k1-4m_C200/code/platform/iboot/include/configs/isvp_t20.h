/*
 * Ingenic isvp configuration
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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

#ifndef __CONFIG_ISVP_T20_H__
#define __CONFIG_ISVP_T20_H__

/**
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32		/* MIPS32 CPU core */
#define CONFIG_CPU_XBURST
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_T20		/* T20 SoC */
/*#define CONFIG_DDR_AUTO_SELF_REFRESH*/
#define CONFIG_SPL_DDR_SOFT_TRAINING

#ifdef CONFIG_LITE_VERSION
#define CONFIG_SYS_APLL_FREQ		712704000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((59 << 20) | (2 << 14) | (1 << 11) | (1 << 8))
#define CONFIG_SYS_APLL_FRAC		0x645a1c
#else
#define CONFIG_SYS_APLL_FREQ		860160000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((71 << 20) | (2 << 14) | (1 << 11) | (1 << 8))
#define CONFIG_SYS_APLL_FRAC		0xae147a
#endif

#define DDR_450M

#ifdef DDR_500M
#define CONFIG_SYS_MPLL_FREQ		1000000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((125 << 20) | (3 << 14) | (1 << 11) | (1 << 8))
#elif defined DDR_450M
#define CONFIG_SYS_MPLL_FREQ		 900000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((75 << 20) | (2 << 14) | (1 << 11) | (1 << 8))
#elif defined DDR_400M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#else
#error please define DDR_FREQ
#endif

#define CONFIG_SYS_VPLL_FREQ		1200000000	/*If VPLL not use mast be set 0*/

#define SEL_SCLKA		2
#define SEL_CPU			1
#define SEL_H0			2
#define SEL_H2			2

#ifdef DDR_500M
#define DIV_PCLK		10
#define DIV_H2			5
#define DIV_H0			5
#elif defined DDR_450M
#define DIV_PCLK		10
#define DIV_H2			5
#define DIV_H0			5
#elif defined DDR_400M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#else
#error please define DDR_FREQ
#endif

#define DIV_L2			2
#define DIV_CPU			1
#define CONFIG_SYS_CPCCR_SEL		(((SEL_SCLKA & 3) << 30)			\
									 | ((SEL_CPU & 3) << 28)			\
									 | ((SEL_H0 & 3) << 26)				\
									 | ((SEL_H2 & 3) << 24)				\
									 | (((DIV_PCLK - 1) & 0xf) << 16)	\
									 | (((DIV_H2 - 1) & 0xf) << 12)		\
									 | (((DIV_H0 - 1) & 0xf) << 8)		\
									 | (((DIV_L2 - 1) & 0xf) << 4)		\
									 | (((DIV_CPU - 1) & 0xf) << 0))

#define CONFIG_CPU_SEL_PLL		APLL
#define CONFIG_DDR_SEL_PLL		MPLL
#define CONFIG_SYS_CPU_FREQ		CONFIG_SYS_APLL_FREQ

#ifdef DDR_500M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_450M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_400M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 3)
#else
#error please define DDR_FREQ
#endif


#define CONFIG_SYS_EXTAL		24000000	/* EXTAL freq: 24 MHz */
#define CONFIG_SYS_HZ			1000		/* incrementer freq */

#define CONFIG_SYS_DCACHE_SIZE		32768
#define CONFIG_SYS_ICACHE_SIZE		32768
#define CONFIG_SYS_CACHELINE_SIZE	32

#define CONFIG_SYS_UART_INDEX		1
#define CONFIG_BAUDRATE			115200

/*#define CONFIG_DDR_TEST_CPU
#define CONFIG_DDR_TEST*/

#define CONFIG_DDR_TYPE_DDR2
#define CONFIG_DDR_PARAMS_CREATOR
#define CONFIG_DDR_HOST_CC
#define CONFIG_DDR_FORCE_SELECT_CS1
#define CONFIG_DDR_CS0			1	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1			0	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_DW32			0	/* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDRC_CTRL_PDT DDRC_CTRL_PDT_128
#ifdef CONFIG_DDR2_128M
#define CONFIG_DDR2_M14D1G1664A
#else
#define CONFIG_DDR2_M14D5121632A
#endif
#define DDR2_CHIP_DRIVER_OUT_STRENGTH 0

#define CONFIG_DDR_PHY_IMPEDANCE 40000
#define CONFIG_DDR_PHY_ODT_IMPEDANCE 50000 //75000
/*#define CONFIG_DDR_PHY_IMPED_PULLUP	0xf*/
/*#define CONFIG_DDR_PHY_IMPED_PULLDOWN	0xf*/

/* #define CONFIG_DDR_DLL_OFF */

/*#define CONFIG_DDR_CHIP_ODT*/
/*#define CONFIG_DDR_PHY_ODT*/
/*#define CONFIG_DDR_PHY_DQ_ODT*/
/*#define CONFIG_DDR_PHY_DQS_ODT*/

/**
 * Boot arguments definitions.
 */
/* #define BOOTARGS_COMMON "console=ttyS1,115200n8 mem=36M@0x0 ispmem=8M@0x2400000 rmem=20M@0x2c00000" */

/**
 * Drivers configuration.
 */
/* MMC */
#if defined(CONFIG_JZ_MMC_MSC0) || defined(CONFIG_JZ_MMC_MSC1)
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_JZ_MMC			1
#endif  /* JZ_MMC_MSC0 || JZ_MMC_MSC1 */

#ifdef CONFIG_JZ_MMC_MSC0
#define CONFIG_JZ_MMC_SPLMSC 0
#define CONFIG_JZ_MMC_MSC0_PB 1
#endif

#ifdef CONFIG_SFC_COMMOND
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
#define CONFIG_JZ_SFC_PA
#define CONFIG_JZ_SFC
#define CONFIG_SFC_NOR
#define CONFIG_SPI_FLASH_INGENIC
#define CONFIG_SPI_DUAL
/*#define CONFIG_SPI_QUAD*/
#endif

#ifdef CONFIG_NORFLASH_32M
#define CONFIG_SPI_FLASH_BAR
#endif
/* SFC */
#define CONFIG_SFC_MIN_ALIGN 0x1000  /*0x1000->4K Erase,0x8000->32k 0x10000->64k*/
#if defined(CONFIG_SPL_SFC_SUPPORT)
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_JZ_SFC_PA
#ifdef CONFIG_SPI_NAND
#define CONFIG_UBOOT_OFFSET	CONFIG_SPL_MAX_SIZE/*(26  * 1024)*/
#define CONFIG_SPI_NAND_BPP	(2048 +64)  /*Bytes Per Page*/
#define CONFIG_SPI_NAND_PPB	(64)        /*Page Per Block*/
#define CONFIG_SPL_SFC_NAND
#define CONFIG_CMD_SFC_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0xb3441000
#else /* CONFIG_SPI_NAND */
#define CONFIG_JZ_SFC
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_INGENIC
#define CONFIG_SPI_FLASH
#define CONFIG_UBOOT_OFFSET	CONFIG_SPL_MAX_SIZE/*(26  * 1024)*/
#define CONFIG_SPL_SFC_NOR
#define CONFIG_SPI_DUAL
/*#define CONFIG_SPI_QUAD*/
#endif
#endif /* CONFIG_SPL_SFC_SUPPORT */

#ifdef CONFIG_JZ_SFC
#define CONFIG_SPIFLASH_PART_OFFSET     0x3c00
#define CONFIG_SPI_NORFLASH_PART_OFFSET     0x3c74
#define CONFIG_NOR_MAJOR_VERSION_NUMBER     1
#define CONFIG_NOR_MINOR_VERSION_NUMBER     0
#define CONFIG_NOR_REVERSION_NUMBER     0
#define CONFIG_NOR_VERSION     (CONFIG_NOR_MAJOR_VERSION_NUMBER | (CONFIG_NOR_MINOR_VERSION_NUMBER << 8) | (CONFIG_NOR_REVERSION_NUMBER <<16))
#endif
/* END SFC */

/* MTD */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE





/**
 * Environment
 */
#ifdef CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_MONITOR_LEN + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)
#elif CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE	(1024 * 16)
#define CONFIG_ENV_SIZE			(1024 * 16)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_MONITOR_LEN + CONFIG_UBOOT_OFFSET)
#else
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_NAND_BLOCK_SIZE * 5)
#endif

/**
 * SPL configuration
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK

#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH		"$(CPUDIR)/$(SOC)"

#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-nor-spl.lds"
#else /* CONFIG_SPL_NOR_SUPPORT */
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-spl.lds"
#endif /* CONFIG_SPL_NOR_SUPPORT */

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_TEXT_BASE		0x80001000
#define CONFIG_SPL_MAX_SIZE		(26 * 1024)

#ifdef CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_SYS_SPI_BOOT_FREQ	1000000
#endif /* CONFIG_SPL_SPI_SUPPORT */

/**
 * boot uImage configs
 **/
#define SPI_NOR_U_IMAGE_OFFSET	0x40000
#define MAX_U_IMAGE_LIMIT	(2048 * 1024 * 2) // 4MB

#endif /*__CONFIG_ISVP_H__*/