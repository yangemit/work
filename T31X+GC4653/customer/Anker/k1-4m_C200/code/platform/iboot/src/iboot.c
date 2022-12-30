/*
 * (C) Copyright 2010
 * Texas Ingenic, <www.ingenic.com>
 *
 * Vane <jingbin.bi@ingenic.com>
 *
 */
#include <common.h>
#include <spl.h>

//#define GPIO_SPEED_DEBUG

#ifndef SPI_NOR_U_IMAGE_OFFSET
#define SPI_NOR_U_IMAGE_OFFSET	 0x8000
#endif


#ifndef SD_U_IMAGE_OFFSET
#define SD_U_IMAGE_OFFSET	0x300000
#endif

#define MAX_U_IMAGE_LIMIT	(4096 * 1024) // 4MB

#ifdef CONFIG_NOR_UPDATE
#define SPI_NOR_U_IMAGE_UPDATE_KENERL_OFFSET		(12*1024*1024 + 256*1024)
#define SPI_NOR_U_IMAGE_UPDATE_FLAG_OFFSET	 	(16*1024*1024 - 32*1024)
#define CPM_KENREL_UPDATE_SING 	0x55504454
#define CPM_KENREL_LOG_SING 	0x55503343
#define UPDATE_FLAG_LEN 	4
#endif


/*
 *#ifndef MAX_U_IMAGE_LIMIT
 *#endif
 */

#define LOAD_DST_ADDR		0x80600000

/* Pointer to as well as the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;
gd_t gdata __attribute__ ((section(".data")));

struct global_info ginfo __attribute__ ((section(".data"))) = {
	.extal		= CONFIG_SYS_EXTAL,
	.cpufreq	= CONFIG_SYS_CPU_FREQ,
	.ddrfreq	= CONFIG_SYS_MEM_FREQ,
	.uart_idx	= CONFIG_SYS_UART_INDEX,
};

extern void pll_init(void);
extern void sdram_init(void);


#define GPIO_SPEED_DEBUG


void debug_gpio(int gpio_bit, int value)
{
	*(volatile unsigned int *)(0xb0011000 + 0x18) = (1 << gpio_bit); //PCINTC 0
	*(volatile unsigned int *)(0xb0011000 + 0x24) = (1 << gpio_bit); //PCMSKS 1
	*(volatile unsigned int *)(0xb0011000 + 0x38) = (1 << gpio_bit); //PCPAT1C 0

	if (value == 1) {
		*(volatile unsigned int *)(0xb0011000 + 0x44) = (1 << gpio_bit); //PCPAT0C 1
	} else {
		*(volatile unsigned int *)(0xb0011000 + 0x48) = (1 << gpio_bit); //PCPAT0C 1
	}

}

void iboot_main(ulong dummy)
{
	/* PB20 output high level test */
	/* debug_gpio(20, 1); */

	/* Set global data pointer */
	unsigned int flash_flag = 0;
	int ret = 0;

	gd = &gdata;

	/* Setup global info */
	gd->arch.gi = &ginfo;

	gpio_init();

	/*gpio_port_direction_input(1,0x20000000);*/

#ifndef CONFIG_FPGA
	/* Init uart first */
	enable_uart_clk();
#endif /* CONFIG_FPGA */

	jz_serial_init();
	puts("jz_serial_init ok\n");
	/* debug_gpio(21, 1); */

#ifndef CONFIG_FPGA
	/* debug("Timer init\n"); */
	timer_init();

	/* debug("CLK stop\n"); */
	clk_prepare();

	/* debug("PLL init\n"); */
	pll_init();

	/* debug("CLK init\n"); */
	clk_init();
#endif /* CONFIG_FPGA */

	/* debug("SDRAM init\n"); */
	sdram_init();
	debug("SDRAM init ok\n");
	/* ddr_basic_tests(); */
	/* debug("ddr_basic_tests ok\n"); */

	 /*MUST access 0xa3fffffc address */
	*(volatile unsigned int *)0xa3fffffc = 0x12345678;

	/* Clear the BSS */
	memset(__bss_start, 0, (char *)&__bss_end - __bss_start);

	/*SFC init*/
	sfc_init();
#ifndef CONFIG_NOR_UPDATE
	printf("\nspl_sfc_nor_load_kernel ...\n");
	spl_sfc_nor_load_kernel(SPI_NOR_U_IMAGE_OFFSET,
			MAX_U_IMAGE_LIMIT,
			LOAD_DST_ADDR);
	 boot_uImage(LOAD_DST_ADDR, MAX_U_IMAGE_LIMIT);
#else
	spl_sfc_nor_load_update_flag(SPI_NOR_U_IMAGE_UPDATE_FLAG_OFFSET, UPDATE_FLAG_LEN, &flash_flag);

	if(flash_flag == CPM_KENREL_UPDATE_SING || flash_flag == CPM_KENREL_LOG_SING){
	/* load update kernel */
		printf("\nspl_sfc_nor_load_update_kernel flash_flag=0x%x...\n",flash_flag);
		spl_sfc_nor_load_kernel(SPI_NOR_U_IMAGE_UPDATE_KENERL_OFFSET,
				MAX_U_IMAGE_LIMIT,
				LOAD_DST_ADDR);

	} else {
	/* load kernel */
		printf("\nspl_sfc_nor_load_kernel flash_flag=0x%x...\n",flash_flag);
		spl_sfc_nor_load_kernel(SPI_NOR_U_IMAGE_OFFSET,
				MAX_U_IMAGE_LIMIT,
				LOAD_DST_ADDR);
	}

	/* start kernel */
	ret = boot_uImage(LOAD_DST_ADDR, MAX_U_IMAGE_LIMIT);
	if(ret) {
		printf("\nsfc_nor_load_kernel is faild ,reload load kernel\n");
		if(flash_flag == CPM_KENREL_UPDATE_SING || flash_flag == CPM_KENREL_LOG_SING){
			/* load kernel */
			spl_sfc_nor_load_kernel(SPI_NOR_U_IMAGE_OFFSET,
					MAX_U_IMAGE_LIMIT,
					LOAD_DST_ADDR);
		} else {
			spl_sfc_nor_load_kernel(SPI_NOR_U_IMAGE_UPDATE_KENERL_OFFSET,
					MAX_U_IMAGE_LIMIT,
					LOAD_DST_ADDR);
		}
		ret = boot_uImage(LOAD_DST_ADDR, MAX_U_IMAGE_LIMIT);
	}
#endif
}
