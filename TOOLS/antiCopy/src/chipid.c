#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

typedef enum _SOC_TYPE {
	SOC_T10 = 0,
	SOC_T20 = 1,
	SOC_T30 = 2,
} SOC_TYPE;

#ifndef SOC_T40
#define H2CLK_PATH		"/proc/jz/clock/h2clk/rate"
#else
#define H2CLK_PATH		"/proc/jz/clock/clocks"
#endif
#define EFUSE_BASE		0x13540000
#define EFUSE_SIZE		0xf
#define EFUSE_CTL		0x0
#define EFUSE_CFG		0x4
#define EFUSE_STAT		0x8
#define EFUSE_DATA		0xc

#define SYS_BASE		0x13000000
#define SYS_SIZE		0x1000

static long long int read_h2clk(void)
{
	FILE *filep = NULL;
	char h2clkrate[512];

	filep = fopen(H2CLK_PATH, "r");
	if (filep == NULL) {
		printf("fopen %s failed:%s\n", H2CLK_PATH, strerror(errno));
		goto err_fopen_h2clk;
	}

#ifndef SOC_T40

	fgets(h2clkrate, sizeof(h2clkrate), filep);

	fclose(filep);

	if ((strlen(h2clkrate) < strlen("rate:")) || (strncmp(h2clkrate, "rate:", strlen("rate:")))) {
		printf("invalid h2clk rate format\n");
		goto err_invalid_h2clk_format;
	}

	h2clkrate[strlen(h2clkrate) - 1] = '\0';

	return strtoll(h2clkrate + strlen("rate:"), NULL, 0);

err_invalid_h2clk_format:
#else
	char str[32] = {0};

	while (!feof(filep)) {
		if (fscanf(filep, "%[^\n]", h2clkrate) < 0)
			break;
		fseek(filep , 1, SEEK_CUR);
		/* printf("clk: %s\n", h2clkrate); */
		memset(str, 0, 32);
		memcpy(str, h2clkrate + 4, strlen("div_ahb2"));
		str[strlen("div_ahb2")] = '\0';
		if (!strncmp(str, "div_ahb2", strlen("div_ahb2"))) {
			memset(str, 0, 32);
			memcpy(str, h2clkrate + 4 + strlen("div_ahb2") + 9 , 3);
			printf("h2clkrate: %sMHz\n", str);
			break;
		}

	}

	fclose(filep);

	/* printf("rate : %ld\n", strtol(str, NULL, 0)); */

	return strtol(str, NULL, 0) * 1000000;
#endif

err_fopen_h2clk:
	return -1;
}

static unsigned int get_efuse_cfg_val(long long int rate)
{
	int i = 0, val = 0, rd_adj = 0, wr_adj = 0, rd_strobe = 0, wr_strobe = 0;
	int nspertick = 1000000000 / rate;

	for (i = 0; i < 0xf; i++) {
		if(((( i + 1) * nspertick ) * 10) > 65)
			break;
	}
	if(i == 0xf) {
		printf("get efuse cfg rd_adj fail!\n");
		goto err_rd_adj;
	}
	rd_adj = wr_adj = i;

	for(i = 0; i < 0xf; i++)
		if(((rd_adj + i + 5) * nspertick ) > 35)
			break;
	if(i == 0xf) {
		printf("get efuse cfg rd_strobe fail!\n");
		goto err_rd_strobe;
	}
	rd_strobe = i;

	for(i = 1; i < 11000; i += 100) {
		val = (wr_adj + i + 1666) * nspertick;
		if( val > 9 * 1000 && val < 11 *1000)
			break;
	}
	if(i >= 11000) {
		printf("get efuse cfg wd_strobe fail!\n");
		goto err_wd_strobe;
	}
	wr_strobe = i;

	return (1 << 31 | rd_adj << 20 | rd_strobe << 16 | wr_adj << 12 | wr_strobe);

err_wd_strobe:
err_rd_strobe:
err_rd_adj:
	return 0;
}

static inline unsigned int read_reg(void *base, int offset)
{
	return *((volatile unsigned int *)(base + offset));
}

static inline void write_reg(void *base, int offset, unsigned int val)
{
	*((volatile unsigned int *)(base + offset)) = val;
}

static inline SOC_TYPE get_soc_type(unsigned int *sys_base)
{
	unsigned int soc_id = read_reg(sys_base, 0x2c);

	if ((soc_id >> 28) == 1) { /* T series */
		unsigned int id = (soc_id >> 12) & 0xffff;
		if (id == 0x05) {	// T10
			return SOC_T10;
		} else if (id == 0x2000) {
			return SOC_T20;
		} else if (id == 0x0030) {
			return SOC_T30;
		}
	}
	return SOC_T30;
}

static inline int efuse_read_chipid_max4bytes(void *addr, int size, void *base, int offset, int readstep)
{
	int checkcnt = 0x200;
	unsigned int data = 0;
	size = size < 4 ? size : 4;
	write_reg(base, EFUSE_STAT, 0);
	write_reg(base, EFUSE_CTL, ((offset / readstep) << 21) | ((size - 1) << 16) | (1 << 0));
	while ((!(read_reg(base, EFUSE_STAT) & 0x1)) && (checkcnt-- > 0) && usleep(1));
	if (checkcnt <= 0) {
		printf("un waited status\n");
		return -1;
	}
	data = read_reg(base, EFUSE_DATA);
	memcpy(addr, &data, size);

	return 0;
}

/* crc16 check from soc */
static unsigned int mb_crc16(unsigned char *buf, unsigned int len)
{
	int i,j;
	unsigned int c,cksum=0xFFFF;

	for (i=0;i<len;i++)
	{
		c=*(buf+i) & 0x00FF;
		cksum^=c;
		for(j=0;j<8;j++)
		{
			if (cksum & 0x0001){
				cksum>>=1;
				cksum^=0xA001;
			}
			else
				cksum>>=1;
		}
	}
	return(cksum);
}

/* chip_id crc16 check from soc */
static int check_Chip(unsigned char *chipCodeId)
{
	unsigned int cksum,crc_cksum;
	unsigned int data[3];
	unsigned char crc_data[28];

	data[2] = chipCodeId[11]<<24 | chipCodeId[10]<<16 | chipCodeId[9]<<8 | chipCodeId[8];
	data[1] = chipCodeId[7]<<24 | chipCodeId[6]<<16 | chipCodeId[5]<<8 | chipCodeId[4];
	data[0] = chipCodeId[1]<<8 | chipCodeId[0];
	crc_cksum = chipCodeId[3]<<8 | chipCodeId[2];

	sprintf((char *)crc_data, "%x%x%x", data[2], data[1], data[0]);

	cksum = mb_crc16(crc_data, strlen((char *)crc_data));
	if(cksum == crc_cksum) {
		return 0;
	} else {
		return -1;
	}
}

int get_chip_id(unsigned char *chipCodeId)
{
	long long int h2clkrate = 0;
	int memfd = -1, pagesize = 0, sys_maplen = 0, efuse_maplen = 0;
	unsigned int *sys_base = NULL, *efuse_base = NULL, efuse_cfg_val = 0, efusestep = 1;
	int i = 0;
	int chipCodeIdLen = 12;
	SOC_TYPE soc = 0;

	if ((h2clkrate = read_h2clk()) < 0) {
		printf("err to read h2clk rate is %lld\n", h2clkrate);
		goto err_read_h2clkrate;
	}

	memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd < 0) {
		printf("open mem failed:%s\n", strerror(errno));
		goto err_open_mem;
	}

	pagesize = getpagesize();

	sys_maplen = ((SYS_SIZE + (pagesize - 1)) & ~(pagesize - 1));
	sys_base = mmap(NULL, sys_maplen, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, SYS_BASE);
	if (sys_base == NULL) {
		close(memfd);
		printf("open mem failed:%s\n", strerror(errno));
		goto err_mmap_sys_base;
	}

	/* void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset); */
	efuse_maplen = ((EFUSE_SIZE + (pagesize - 1)) & ~(pagesize - 1));
	efuse_base = mmap(NULL, efuse_maplen, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, EFUSE_BASE);
	if (efuse_base == NULL) {
		close(memfd);
		printf("open mem failed:%s\n", strerror(errno));
		goto err_mmap_efuse_base;
	}

	close(memfd);

	soc = get_soc_type(sys_base);
	if (soc < 0) {
		printf("get soc type failed:%s\n", strerror(errno));
		goto err_get_soc_type;
	}

	if ((soc >= SOC_T10) && (soc <= SOC_T20)) {
		efusestep = 1;
	} else {
		efusestep = 4;
	}

	efuse_cfg_val = get_efuse_cfg_val(h2clkrate);
	write_reg(efuse_base, EFUSE_CFG, efuse_cfg_val);

	for (i  = 0; i < chipCodeIdLen / 4; i++) {
		efuse_read_chipid_max4bytes(chipCodeId + 4 * i, 4, efuse_base, 4 * i, efusestep);
	}

	for (i *= 4; i < chipCodeIdLen; i++) {
		efuse_read_chipid_max4bytes(chipCodeId + i, 1, efuse_base, i, efusestep);
	}

	munmap(efuse_base, efuse_maplen);
	munmap(sys_base, sys_maplen);

	*((unsigned int *)(chipCodeId + chipCodeIdLen)) = 0x53effe35;

	return check_Chip(chipCodeId);

err_get_soc_type:
	munmap(efuse_base, efuse_maplen);
err_mmap_efuse_base:
	munmap(sys_base, sys_maplen);
err_mmap_sys_base:
err_open_mem:
err_read_h2clkrate:
	return -1;
}
