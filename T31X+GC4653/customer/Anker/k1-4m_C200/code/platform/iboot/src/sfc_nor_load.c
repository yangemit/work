#include <common.h>
#include <spl.h>
/*#include <asm/io.h>*/
#include <asm/arch/clk.h>
#include <asm/arch/sfc.h>
#include <asm/arch/spi.h>

//#define TEST
#ifdef CONFIG_SPI_QUAD_MODE
#define QUAD_MODE_SUPPORT 1
#define QUAD_MODE_NOT_SUPPORT 0

#define QUAD_MODE_ENABLE 1
#define QUAD_MODE_DISABLE 0

struct spi_quad_mode {
	unsigned char dummy_byte;
	unsigned char RDSR_CMD;
	unsigned char WRSR_CMD;
	unsigned char RD_DATE_SIZE;//the data is write the spi status register for QE bit
	unsigned char WD_DATE_SIZE;//the data is write the spi status register for QE bit
	unsigned char cmd_read;
	unsigned char sfc_mode;
	unsigned int RDSR_DATE;//the data is write the spi status register for QE bit
	unsigned int WRSR_DATE;//this bit should be the flash QUAD mode enable
};

struct spi_nor_platdata {
	char name[32];
	unsigned int pagesize;
	unsigned int sectorsize;
	unsigned int chipsize;
	unsigned int erasesize;
	unsigned int id;
	unsigned char addrsize;

	/* the attribute of NOR eraseing, the common mode */
	unsigned char cmd_blkerase;
	unsigned short erase_maxbusy;
	unsigned int blocksize;

	/* MAX Busytime for page program, unit: ms */
	unsigned short  pp_maxbusy;
	/* MAX Busytime for sector erase, unit: ms */
	unsigned short se_maxbusy;
	/* MAX Busytime for chip erase, unit: ms */
	unsigned short ce_maxbusy;

	/* Flash status register num, Max support 3 register */
	unsigned short st_regnum;
	char quad_support;	/* The flash whether supports the ops mode */
	char quad_status;	/* The flash whether has enabled the quad mode */
	struct spi_quad_mode quad;
};

struct spi_nor_platdata norflashs[] = {
	{
		.name			= "MX25L6406F",
		.pagesize		= 256,
		.sectorsize		= 4*1024,
		.chipsize		= 8192*1024,
		.erasesize		= 64*1024,
		.id				= 0xc22017,
		.addrsize		= 3,
		.cmd_blkerase	= 0xd8,
		.erase_maxbusy	= 1200,
		.blocksize		= 64*1024,
		.pp_maxbusy		= 3,		/* 3ms */
		.se_maxbusy		= 400,		/* 400ms */
		.ce_maxbusy		= 8*1000,	/* 80s */
		.st_regnum		= 3,
		.quad_support	= QUAD_MODE_SUPPORT,
		.quad = {
			.dummy_byte		= 8,
			.RDSR_CMD		= CMD_RDSR,
			.WRSR_CMD		= CMD_WRSR,
			.RDSR_DATE		= 0x40,//the data is write the spi status register for QE bit
			.RD_DATE_SIZE	= 1,
			.WRSR_DATE		= 0x40,//this bit should be the flash QUAD mode enable
			.WD_DATE_SIZE	= 1,
			.cmd_read		= CMD_QREAD,
			.sfc_mode		= TRAN_QUAD,
		},
	},

	{
		.name           = "MX25L12835F",
		.pagesize       = 256,
		.sectorsize     = 4 * 1024,
		.chipsize       = 16384 * 1024,
		.erasesize      = 32 * 1024,//4 * 1024,
		.id             = 0xc22018,
		.addrsize		= 3,
		.cmd_blkerase	= 0x52,
		.erase_maxbusy	= 1000,
		.blocksize      = 32 * 1024,
		.pp_maxbusy     = 3,            /* 3ms */
		.se_maxbusy     = 400,          /* 400ms */
		.ce_maxbusy     = 8 * 10000,    /* 80s */
		.st_regnum      = 3,
		.quad_support	= QUAD_MODE_SUPPORT,
		.quad = {
			.dummy_byte		= 8,
			.RDSR_CMD		= CMD_RDSR,
			.WRSR_CMD		= CMD_WRSR,
			.RDSR_DATE		= 0x40,//the data is write the spi status register for QE bit
			.RD_DATE_SIZE	= 1,
			.WRSR_DATE		= 0x40,//this bit should be the flash QUAD mode enable
			.WD_DATE_SIZE	= 1,
			.cmd_read		= CMD_QREAD,
			.sfc_mode		= TRAN_QUAD,
		},
	},

	{
		.name           = "MX25U12835F",
		.pagesize       = 256,
		.sectorsize     = 4 * 1024,
		.chipsize       = 8 * 1024 * 1024,
		.erasesize      = 32 * 1024,//4 * 1024,
		.id             = 0xc22538,
		.addrsize		= 3,
		.cmd_blkerase	= 0x52,
		.erase_maxbusy	= 1000,
		.blocksize      = 32 * 1024,
		.pp_maxbusy     = 3,            /* 3ms */
		.se_maxbusy     = 400,          /* 400ms */
		.ce_maxbusy     = 8 * 10000,    /* 80s */
		.st_regnum      = 3,
		.quad_support	= QUAD_MODE_SUPPORT,
		.quad = {
			.dummy_byte		= 8,
			.RDSR_CMD		= CMD_RDSR,
			.WRSR_CMD		= CMD_WRSR,
			.RDSR_DATE		= 0x40,//the data is write the spi status register for QE bit
			.RD_DATE_SIZE	= 1,
			.WRSR_DATE		= 0x40,//this bit should be the flash QUAD mode enable
			.WD_DATE_SIZE	= 1,
			.cmd_read		= CMD_QREAD,
			.sfc_mode		= TRAN_QUAD,
		},
	},

	{
		.name           = "MX25L25645G",
		.pagesize       = 256,
		.sectorsize     = 4 * 1024,
		.chipsize       = 32 * 1024 * 1024,
		.erasesize      = 32 * 1024,//4 * 1024,
		.id             = 0xc22019,
		.addrsize		= 3,
		.cmd_blkerase	= 0x52,
		.erase_maxbusy	= 1000,
		.blocksize      = 32 * 1024,
		.pp_maxbusy     = 3,            /* 3ms */
		.se_maxbusy     = 400,          /* 400ms */
		.ce_maxbusy     = 8 * 10000,    /* 80s */
		.st_regnum      = 3,
		.quad_support	= QUAD_MODE_SUPPORT,
		.quad = {
			.dummy_byte		= 8,
			.RDSR_CMD		= CMD_RDSR,
			.WRSR_CMD		= CMD_WRSR,
			.RDSR_DATE		= 0x40,//the data is write the spi status register for QE bit
			.RD_DATE_SIZE	= 1,
			.WRSR_DATE		= 0x40,//this bit should be the flash QUAD mode enable
			.WD_DATE_SIZE	= 1,
			.cmd_read		= CMD_QREAD,
			.sfc_mode		= TRAN_QUAD,
		},
	},

	{
		.name           = "GD25Q256D",
		.pagesize       = 256,
		.sectorsize     = 4 * 1024,
		.chipsize       = 32 * 1024 * 1024,
		.erasesize      = 32 * 1024,//4 * 1024,
		.id             = 0xc84019,
		.addrsize		= 3,
		.cmd_blkerase	= 0x52,
		.erase_maxbusy	= 1000,
		.blocksize      = 32 * 1024,
		.pp_maxbusy     = 3,            /* 3ms */
		.se_maxbusy     = 400,          /* 400ms */
		.ce_maxbusy     = 8 * 10000,    /* 80s */
		.st_regnum      = 3,
		.quad_support	= QUAD_MODE_SUPPORT,
		.quad = {
			.dummy_byte		= 8,
			.RDSR_CMD		= CMD_RDSR,
			.WRSR_CMD		= CMD_WRSR,
			.RDSR_DATE		= 0x40,//the data is write the spi status register for QE bit
			.RD_DATE_SIZE	= 1,
			.WRSR_DATE		= 0x40,//this bit should be the flash QUAD mode enable
			.WD_DATE_SIZE	= 1,
			.cmd_read		= CMD_QREAD,
			.sfc_mode		= TRAN_QUAD,
		},
	},
	{
		.name           = "GD25Q128",
		.pagesize       = 256,
		.sectorsize     = 4 * 1024,
		.chipsize       = 16 * 1024 * 1024,
		.erasesize      = 32 * 1024,//4 * 1024,
		.id             = 0xc84018,
		.addrsize		= 3,
		.cmd_blkerase	= 0x52,
		.erase_maxbusy	= 1000,
		.blocksize      = 32 * 1024,
		.pp_maxbusy     = 3,            /* 3ms */
		.se_maxbusy     = 400,          /* 400ms */
		.ce_maxbusy     = 8 * 10000,    /* 80s */
		.st_regnum      = 3,
		.quad_support	= QUAD_MODE_SUPPORT,
		.quad = {
			.dummy_byte		= 8,
			.RDSR_CMD		= CMD_RDSR,
			.WRSR_CMD		= CMD_WRSR,
			.RDSR_DATE		= 0x2,//the data is write the spi status register for QE bit
			.RD_DATE_SIZE	= 1,
			.WRSR_DATE		= 0x2,//this bit should be the flash QUAD mode enable
			.WD_DATE_SIZE	= 1,
			.cmd_read		= CMD_QREAD,
			.sfc_mode		= TRAN_QUAD,
		},
	},

	{
		.name           = "EN25QH256A",
		.pagesize       = 256,
		.sectorsize     = 4 * 1024,
		.chipsize       = 32 * 1024 * 1024,
		.erasesize      = 32 * 1024,//4 * 1024,
		.id             = 0x1c7019,
		.addrsize		= 3,
		.cmd_blkerase	= 0x52,
		.erase_maxbusy	= 1000,
		.blocksize      = 32 * 1024,
		.pp_maxbusy     = 3,            /* 3ms */
		.se_maxbusy     = 400,          /* 400ms */
		.ce_maxbusy     = 8 * 10000,    /* 80s */
		.st_regnum      = 3,
		.quad_support	= QUAD_MODE_SUPPORT,
		.quad = {
			.dummy_byte		= 8,
			.RDSR_CMD		= CMD_RDSR,
			.WRSR_CMD		= CMD_WRSR,
			.RDSR_DATE		= 0x2,//the data is write the spi status register for QE bit
			.RD_DATE_SIZE	= 1,
			.WRSR_DATE		= 0x2,//this bit should be the flash QUAD mode enable
			.WD_DATE_SIZE	= 1,
			.cmd_read		= CMD_QREAD,
			.sfc_mode		= TRAN_QUAD,
		},
	},

};

struct jz_sfc {
	unsigned int  addr;
	unsigned int  len;
	unsigned int  cmd;
	unsigned int  addr_plus;
	unsigned char channel;
	unsigned char dir;	/* read or write */
	unsigned char mode;
	unsigned char daten;
	unsigned char addr_size;
	unsigned char pollen;
	unsigned char phase;
	unsigned char dummy_byte;
};

static struct spi_nor_platdata *pnorflash = NULL;
unsigned int quad_mode_is_set = 0;
#endif /* CONFIG_SPI_QUAD_MODE */

static uint32_t jz_sfc_readl(unsigned int offset)
{
	return readl(SFC_BASE + offset);
}

static void jz_sfc_writel(unsigned int value, unsigned int offset)
{
	writel(value, SFC_BASE + offset);
}

#ifdef CONFIG_SPI_QUAD_MODE
static inline void is_end_and_clear(void)
{
	unsigned int val;
	val = jz_sfc_readl(SFC_SR);
	while (!(val & END)){
		val = jz_sfc_readl(SFC_SR);
	}
	if ((jz_sfc_readl(SFC_SR)) & END)
		jz_sfc_writel(CLR_END,SFC_SCR);
}

static void read_fifo_data(unsigned int *data, unsigned int length)
{
	unsigned int n;
	while(!(jz_sfc_readl(SFC_SR) & RECE_REQ))
		;
	jz_sfc_writel(CLR_RREQ, SFC_SCR);
	for(n =0 ; n < length; n ++) {
		*data = jz_sfc_readl(SFC_DR);
		data++;
	}
}

static void write_fifo_data(unsigned int *data, unsigned int length)
{
	unsigned int n;
	while(!(jz_sfc_readl(SFC_SR) & TRAN_REQ))
		;
	jz_sfc_writel(CLR_TREQ, SFC_SCR);
	for(n =0 ; n < length; n ++) {
		jz_sfc_writel(*data, SFC_DR);
		data++;
	}
}

static int sfc_data_ops(unsigned int *data, unsigned int size, void (*ops)(unsigned int*, unsigned int))
{
	unsigned int *end = data + (size + 3) / 4;

	while(data < end - THRESHOLD) {
		ops(data, THRESHOLD);
		data += THRESHOLD;
	}
	if(data < end) {
		ops(data, end - data);
	}
	is_end_and_clear();
	return 0;
}

static inline void sfc_set_transfer(struct jz_sfc *hw)
{
	unsigned int val;

	val = jz_sfc_readl(SFC_GLB);
	val &= ~TRAN_DIR;
	val |= hw->dir << TRAN_DIR_OFFSET;
	jz_sfc_writel(val,SFC_GLB);

	jz_sfc_writel(hw->len, SFC_TRAN_LEN);

	val = jz_sfc_readl(SFC_TRAN_CONF(hw->channel));
	val &= ~(ADDR_WIDTH_MSK | CMD_EN | DATEEN | CMD_MSK \
		 | TRAN_CONF_DMYBITS_MSK | TRAN_MODE_MSK);
	val |= (hw->addr_size << ADDR_WIDTH_OFFSET | CMD_EN \
		| hw->daten << TRAN_DATEEN_OFFSET | hw->cmd << CMD_OFFSET \
		| hw->dummy_byte << TRAN_CONF_DMYBITS_OFFSET \
		| hw->mode << TRAN_MODE_OFFSET);
	jz_sfc_writel(val,SFC_TRAN_CONF(hw->channel));

	jz_sfc_writel(hw->addr, SFC_DEV_ADDR(hw->channel));
	jz_sfc_writel(hw->addr_plus,SFC_DEV_ADDR_PLUS(hw->channel));
}

static inline void sfc_send_cmd(struct jz_sfc *sfc)
{
	sfc_set_transfer(sfc);
	jz_sfc_writel(1 << 2,SFC_TRIG);
	jz_sfc_writel(START,SFC_TRIG);

	/*this must judge the end status*/
	if(sfc->daten == 0)
		is_end_and_clear();
}

static void sfc_set_quad_mode(void)
{
	unsigned char cmd[4];
	unsigned int buf = 0;
	unsigned int tmp = 0;
	int i = 10;
	struct jz_sfc sfc;
	struct spi_quad_mode *quad_mode = &pnorflash->quad;

	if(pnorflash->quad_support == QUAD_MODE_NOT_SUPPORT)
		return;

	cmd[0] = CMD_WREN;
	cmd[1] = quad_mode->WRSR_CMD;
	cmd[2] = quad_mode->RDSR_CMD;
	cmd[3] = CMD_RDSR;

	sfc.cmd = cmd[0];
	sfc.addr = 0;
	sfc.addr_size = 0;
	sfc.addr_plus = 0;
	sfc.dummy_byte = 0;
	sfc.daten = 0;
	sfc.len = 0;
	sfc.dir = GLB_TRAN_DIR_WRITE;
	sfc.channel = 0;
	sfc.mode = TRAN_SPI_STANDARD;
	sfc_send_cmd(&sfc);

	sfc.cmd = cmd[1];
	sfc.len = quad_mode->WD_DATE_SIZE;
	sfc.daten = 1;
	sfc_send_cmd(&sfc);
	sfc_data_ops(&quad_mode->WRSR_DATE,1, write_fifo_data);

	sfc.cmd = cmd[3];
	sfc.len = 1;
	sfc.daten = 1;
	sfc.dir = GLB_TRAN_DIR_READ;
	sfc_send_cmd(&sfc);
	sfc_data_ops(&tmp, 1, read_fifo_data);

	while(tmp & CMD_SR_WIP) {
		sfc.cmd = cmd[3];
		sfc.len = 1;
		sfc.daten = 1;
		sfc.dir = GLB_TRAN_DIR_READ;
		sfc_send_cmd(&sfc);
		sfc_data_ops(&tmp, 1, read_fifo_data);
	}
	sfc.cmd = cmd[2];
	sfc.len = quad_mode->WD_DATE_SIZE;
	sfc.daten = 1;
	sfc_send_cmd(&sfc);
	sfc_data_ops(&buf, 1, read_fifo_data);
	while(!(buf & quad_mode->RDSR_DATE)&&((i--) > 0)) {
		sfc.cmd = cmd[2];
		sfc.len = quad_mode->WD_DATE_SIZE;
		sfc.daten = 1;
		sfc_send_cmd(&sfc);
		sfc_data_ops(&buf, 1, read_fifo_data);
	}
	quad_mode_is_set = 1;
	pnorflash->quad_status = QUAD_MODE_ENABLE;
}

static int sfc_read(unsigned int addr, unsigned int addr_plus,
		unsigned int addr_len, unsigned int *data, unsigned int len)
{
	unsigned char cmd;
	struct jz_sfc sfc;
	struct spi_quad_mode *quad_mode = &pnorflash->quad;

	cmd = CMD_READ;
	sfc.addr = addr;
	sfc.addr_size = addr_len;//pdata.norflash_params.addrsize;
	sfc.addr_plus = addr_plus;
	sfc.daten = 1;
	sfc.len = len;
	sfc.dir = GLB_TRAN_DIR_READ;
	sfc.channel = 0;
	sfc.mode = TRAN_SPI_STANDARD;
	sfc.dummy_byte = 0;

	if(quad_mode_is_set == 1){
		cmd  = quad_mode->cmd_read;
		sfc.mode = quad_mode->sfc_mode;
		sfc.dummy_byte = quad_mode->dummy_byte;
	}

	sfc.cmd = cmd;
	sfc_send_cmd(&sfc);
	return sfc_data_ops((unsigned int *)data, len, read_fifo_data);
}

static void reset_nor(void)
{
	struct jz_sfc sfc;
	unsigned int id;
	int i = 0;

	sfc.cmd = 0x66;
	sfc.addr = 0;
	sfc.addr_size = 0;
	sfc.addr_plus = 0;
	sfc.dummy_byte = 0;
	sfc.daten = 0;
	sfc.len = 0;
	sfc.dir = GLB_TRAN_DIR_READ;
	sfc.channel = 0;
	sfc.mode = TRAN_SPI_STANDARD;
	sfc_send_cmd(&sfc);
	sfc.cmd = 0x99;
	sfc_send_cmd(&sfc);
	udelay(60);

	/* read identification of the chip */
	sfc.cmd = CMD_RDID;
	sfc.daten = 1;
	sfc.len = 3;
	sfc_send_cmd(&sfc);
	sfc_data_ops(&id, 3, read_fifo_data);
	id = ((id & 0xff) << 16) | (((id >> 8) & 0xff) << 8) | ((id >> 16) & 0xff);
	for(i = 0; i < (sizeof(norflashs) / sizeof(norflashs[0])); i++){
		if(id == norflashs[i].id){
			pnorflash = &norflashs[i];
			break;
		}
	}
	if(pnorflash){
		sfc_set_quad_mode();
	}
}
#else /* CONFIG_SPI_QUAD_MODE */
void sfc_set_mode(int channel, int value)
{
	unsigned int tmp;
	tmp = jz_sfc_readl(SFC_TRAN_CONF(channel));
	tmp &= ~(TRAN_MODE_MSK);
	tmp |= (value << TRAN_MODE_OFFSET);
	jz_sfc_writel(tmp,SFC_TRAN_CONF(channel));
}

void sfc_dev_addr_dummy_bytes(int channel, unsigned int value)
{
	unsigned int tmp;
	tmp = jz_sfc_readl(SFC_TRAN_CONF(channel));
	tmp &= ~TRAN_CONF_DMYBITS_MSK;
	tmp |= (value << TRAN_CONF_DMYBITS_OFFSET);
	jz_sfc_writel(tmp,SFC_TRAN_CONF(channel));
}

static void sfc_set_read_reg(unsigned int cmd, unsigned int addr,
		unsigned int addr_plus, unsigned int addr_len, unsigned int data_en, unsigned int dummy_byte)
{
	unsigned int tmp;

	tmp = jz_sfc_readl(SFC_GLB);
	tmp &= ~PHASE_NUM_MSK;
	tmp |= (0x1 << PHASE_NUM_OFFSET);
	jz_sfc_writel(tmp, SFC_GLB);

	if (data_en) {
		tmp = (addr_len << ADDR_WIDTH_OFFSET) | CMD_EN |
			DATEEN | (cmd << CMD_OFFSET);
	} else {
		tmp = (addr_len << ADDR_WIDTH_OFFSET) | CMD_EN |
			(cmd << CMD_OFFSET);
	}

	jz_sfc_writel(tmp, SFC_TRAN_CONF(0));
	jz_sfc_writel(addr, SFC_DEV_ADDR(0));
	jz_sfc_writel(addr_plus, SFC_DEV_ADDR_PLUS(0));

	sfc_dev_addr_dummy_bytes(0, dummy_byte);
	sfc_set_mode(0, 0);

	jz_sfc_writel(START, SFC_TRIG);
}

static int sfc_read_data(unsigned int *data, unsigned int len)
{
	unsigned int tmp_len = 0;
	unsigned int fifo_num = 0;
	unsigned int i;
	int count = 0;
	int last_count = 0;
	while (1) {
		if (jz_sfc_readl(SFC_SR) & RECE_REQ) {
			jz_sfc_writel(CLR_RREQ, SFC_SCR);
			if ((len - tmp_len) > THRESHOLD)
				fifo_num = THRESHOLD;
			else {
				fifo_num = len - tmp_len;
			}

			for (i = 0; i < fifo_num; i++) {
				*data = jz_sfc_readl(SFC_DR);
				data++;
				tmp_len++;
			}
		}

		if (tmp_len == len)
			break;
	}

	while ((jz_sfc_readl(SFC_SR) & END) != END)
		printf("jz_sfc_readl(SFC_SR) : %x\n", jz_sfc_readl(SFC_SR));
	jz_sfc_writel(CLR_END,SFC_SCR);

	return 0;
}

static int sfc_read(unsigned int addr, unsigned int addr_plus,
		unsigned int addr_len, unsigned int *data, unsigned int len)
{
	unsigned int cmd, ret, qread[1] = {0}, tmp, dummy_byte;

	jz_sfc_writel(STOP, SFC_TRIG);
	jz_sfc_writel(FLUSH, SFC_TRIG);

	cmd  = CMD_FAST_READ;
	dummy_byte = 8;

	jz_sfc_writel((len * 4), SFC_TRAN_LEN);

	sfc_set_read_reg(cmd, addr, addr_plus, addr_len, 1, dummy_byte);

	ret = sfc_read_data(data, len);
	if (ret)
		return ret;
	else
		return 0;
}
#endif /* CONFIG_SPI_QUAD_MODE */

#define CPM_SSICDR (0xb0000000 + 0x74)
#define CPM_CLKGR0 (0xb0000000 + 0x20)
#define SSI_DIV16	15
#define SSI_DIV8	7
#define SSI_DIV6	5
void sfc_init()
{
	unsigned int tmp,i;
	unsigned int val = 0;
	unsigned int sfc_rate;

	/* the clock register is different between T20 and T31 */
	/* clock : SSICDR, CLKGR0 */
	/* the config is for T20 */
#ifdef CONFIG_T20
	writel((1 << 27), CPM_SSICDR);
	writel((SSI_DIV16 | (1 << 29)), CPM_SSICDR);
	while (readl(CPM_SSICDR) & (1 << 28));
		;
	tmp = readl(CPM_CLKGR0);
	tmp &= ~(1 << 20);
	writel(tmp, CPM_CLKGR0);
#elif defined CONFIG_T31
	sfc_rate = 160000000;
	clk_set_rate(SSI, sfc_rate);
#endif

#ifdef CONFIG_SPI_QUAD_MODE
	jz_sfc_writel(3 << 1 ,SFC_TRIG);
	val = jz_sfc_readl(SFC_GLB);
	val &= ~(TRAN_DIR | OP_MODE | THRESHOLD_MSK);
	val |= (WP_EN | THRESHOLD << THRESHOLD_OFFSET);
	jz_sfc_writel(val,SFC_GLB);

	val = jz_sfc_readl(SFC_DEV_CONF);
	val &= ~(CMD_TYPE | CPHA | CPOL | SMP_DELAY_MSK |
		 THOLD_MSK | TSETUP_MSK | TSH_MSK);
	val |= (CEDL | HOLDDL | WPDL | (1 << TSETUP_OFFSET) | (1 << SMP_DELAY_OFFSET));
	jz_sfc_writel(val,SFC_DEV_CONF);

	val = CLR_END | CLR_TREQ | CLR_RREQ | CLR_OVER | CLR_UNDER;
	jz_sfc_writel(val,SFC_INTC);
	/* low power consumption */
	jz_sfc_writel(0, SFC_CGE);

	reset_nor();
#endif /* CONFIG_SPI_QUAD_MODE */
	tmp = jz_sfc_readl(SFC_GLB);
	tmp &= ~(TRAN_DIR | OP_MODE );
	tmp |= WP_EN;
	jz_sfc_writel(tmp,SFC_GLB);

	tmp = jz_sfc_readl(SFC_DEV_CONF);
	tmp &= ~(CMD_TYPE | CPHA | CPOL | SMP_DELAY_MSK |
			THOLD_MSK | TSETUP_MSK | TSH_MSK);
	tmp |= (CEDL | HOLDDL | WPDL | 1 << SMP_DELAY_OFFSET);
	jz_sfc_writel(tmp,SFC_DEV_CONF);

	for (i = 0; i < 6; i++) {
		jz_sfc_writel((jz_sfc_readl(SFC_TRAN_CONF(i))& (~(TRAN_MODE_MSK | PHASE_FORMAT))),SFC_TRAN_CONF(i));
	}

	jz_sfc_writel((CLR_END | CLR_TREQ | CLR_RREQ | CLR_OVER | CLR_UNDR),SFC_INTC);

	jz_sfc_writel(0,SFC_CGE);

	tmp = jz_sfc_readl(SFC_GLB);
	tmp &= ~(THRESHOLD_MSK);
	tmp |= (THRESHOLD << THRESHOLD_OFFSET);
	jz_sfc_writel(tmp,SFC_GLB);
}

void sfc_nor_load(unsigned int src_addr, unsigned int count, unsigned int dst_addr)
{
	unsigned int ret, addr_len, words_of_spl;

	/* spi norflash addr len */
	addr_len = 3;

	/* count word align */
	words_of_spl = (count + 3) / 4;

	/*sfc_init();*/

#ifdef TEST
	int i;
	for(i = 0; i < 1024 * 256; i++)
		*(unsigned int *)(dst_addr + 4 * i) = i;
	printf("aceess the memrary...\n",__func__, __LINE__);
#endif

	/* reset nor flash */
	/* sfc_set_read_reg(0xe9, 0, 0, 0, 0); */
	/* udelay(100); */

#ifdef CONFIG_SPI_QUAD_MODE
	if(pnorflash == NULL){
		printf("sfc init error\n");
		return;
	}

	ret = sfc_read(src_addr, 0x0, addr_len, (unsigned int *)(dst_addr), count);
#else /* CONFIG_SPI_QUAD_MODE */
	ret = sfc_read(src_addr, 0x0, addr_len, (unsigned int *)(dst_addr), words_of_spl);
#endif /* CONFIG_SPI_QUAD_MODE */
	if (ret) {
		printf("sfc read error\n");
	}

	return ;
}

void spl_sfc_nor_load_kernel(unsigned int offset, unsigned int len, unsigned int dst_addr)
{
	sfc_nor_load(offset, len, dst_addr);
}

void spl_sfc_nor_load_update_flag(unsigned int offset, unsigned int len, unsigned int dst_addr)
{
	sfc_nor_load(offset, len, dst_addr);
}
