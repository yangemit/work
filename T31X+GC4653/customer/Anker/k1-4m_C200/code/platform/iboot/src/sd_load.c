
#include <config.h>
#include <common.h>
#include <command.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <linux/list.h>
#include <div64.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/mmc.h>
#include <asm/unaligned.h>
#include <spl.h>
#include <version.h>

#define udelay(int) do{}while(0); //for decreasing time by ykliu
/* jz_mmc_priv flags */
#define JZ_MMC_BUS_WIDTH_MASK 0x3
#define JZ_MMC_BUS_WIDTH_1    0x0
#define JZ_MMC_BUS_WIDTH_4    0x2
#define JZ_MMC_BUS_WIDTH_8    0x3
#define JZ_MMC_SENT_INIT (1 << 2)

/* SPL will only use a single MMC device (CONFIG_JZ_MMC_SPLMSC) */
#define CLK_MIN 200000
#define CLK_MAX 24000000

#define VOLTAGES (MMC_VDD_27_28 |		\
		  MMC_VDD_28_29 |		\
		  MMC_VDD_29_30 |		\
		  MMC_VDD_30_31 |		\
		  MMC_VDD_31_32 |		\
		  MMC_VDD_32_33 |		\
		  MMC_VDD_33_34 |		\
		  MMC_VDD_34_35 |		\
		  MMC_VDD_35_36)

#define HOST_CAPS (MMC_MODE_4BIT | MMC_MODE_HC)

/* Set block count limit because of 16 bit register limit on some hardware*/
#define MMC_MAX_READ_COUNT_ONCE 20

unsigned int mmc_flags = 0;
unsigned int op_cond_response = 0;
unsigned int mmc_clock = 0;
unsigned int bus_width = 0;
unsigned int ocr = 0;
unsigned short rca = 0;
unsigned int cid[4];
int high_capacity = 0;
unsigned int mmc_read_bl_len = 512;
unsigned int mmc_version = SD_VERSION_2;
unsigned int mmc_card_caps = HOST_CAPS;
#ifndef CONFIG_FPGA
unsigned int mmc_tran_speed = 24000000;
#else /* no CONFIG_FPGA */
unsigned int mmc_tran_speed = 300000;//24000000;
#endif /* no CONFIG_FPGA */

static uint16_t mmc_readw(uintptr_t off)
{
	return readw(MSC0_BASE + off);
}

static uint32_t mmc_readl(uintptr_t off)
{
	return readl(MSC0_BASE + off);
}

static void mmc_writel(uint32_t value, uintptr_t off)
{
	writel(value, MSC0_BASE + off);
}

int __mmc_send_cmd(struct mmc_cmd *cmd, struct mmc_data *data)
{
	int ret;

	uint32_t stat, cmdat = 0;

	/* setup command */
	mmc_writel(cmd->cmdidx,MSC_CMD);
	mmc_writel(cmd->cmdarg,MSC_ARG);

	if (data) {
		/* setup data */
		cmdat |= MSC_CMDAT_DATA_EN;
		mmc_writel(data->blocks, MSC_NOB);
		mmc_writel(data->blocksize, MSC_BLKLEN);
	}

	/* setup response */
	switch (cmd->resp_type) {
	case MMC_RSP_R1:
	case MMC_RSP_R1b:
		cmdat |= MSC_CMDAT_RESPONSE_R1;
		break;
	case MMC_RSP_R2:
		cmdat |= MSC_CMDAT_RESPONSE_R2;
		break;
	case MMC_RSP_R3:
		cmdat |= MSC_CMDAT_RESPONSE_R3;
		break;
	default:
		break;
	}

	if (cmd->resp_type & MMC_RSP_BUSY)
		cmdat |= MSC_CMDAT_BUSY;

	/* set init for the first command only */
	if (!(mmc_flags & JZ_MMC_SENT_INIT)) {
		cmdat |= MSC_CMDAT_INIT;
		mmc_flags |= JZ_MMC_SENT_INIT;
	}

	cmdat |= (mmc_flags & JZ_MMC_BUS_WIDTH_MASK) << 9;

	/* write the data setup */
	mmc_writel(cmdat, MSC_CMDAT);

	/* start the command (& the clock) */
	mmc_writel(MSC_STRPCL_START_OP, MSC_STRPCL);

	/* wait for completion */
	while (!(stat = (mmc_readl(MSC_IREG) & (MSC_IREG_END_CMD_RES | MSC_IREG_TIME_OUT_RES))))
		udelay(10000);

	mmc_writel(stat, MSC_IREG);
	if (stat & MSC_IREG_TIME_OUT_RES)
		return TIMEOUT;

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		/* read the response */
		if (cmd->resp_type & MMC_RSP_136) {
			uint16_t a, b, c, i;
			a = mmc_readw(MSC_RES);
			for (i = 0; i < 4; i++) {
				b = mmc_readw(MSC_RES);
				c = mmc_readw(MSC_RES);
				cmd->response[i] = (a << 24) | (b << 8) | (c >> 8);
				a = c;
			}
		} else {
			cmd->response[0] = mmc_readw(MSC_RES) << 24;
			cmd->response[0] |= mmc_readw(MSC_RES) << 8;
			cmd->response[0] |= mmc_readw(MSC_RES) & 0xff;
		}
	}

	if (cmd->resp_type == MMC_RSP_R1b) {
		while (!(mmc_readl(MSC_STAT) & MSC_STAT_PRG_DONE));
		mmc_writel(MSC_IREG_PRG_DONE, MSC_IREG);
	}

	if (data && (data->flags & MMC_DATA_READ)) {
		/* read the data */
		int sz = data->blocks * data->blocksize;
		void *buf = data->dest;
		do {
			stat = mmc_readl(MSC_STAT);
			if (stat & MSC_STAT_TIME_OUT_READ)
				return TIMEOUT;
			if (stat & MSC_STAT_CRC_READ_ERROR)
				return COMM_ERR;
			if (stat & MSC_STAT_DATA_FIFO_EMPTY) {
				udelay(100);
				continue;
			}
			do {
				uint32_t val = mmc_readl(MSC_RXFIFO);
				if (sz == 1)
					*(uint8_t *)buf = (uint8_t)val;
				else if (sz == 2)
					put_unaligned_le16(val, buf);
				else if (sz >= 4)
					put_unaligned_le32(val, buf);
				buf += 4;
				sz -= 4;
				stat = mmc_readl(MSC_STAT);
			} while (!(stat & MSC_STAT_DATA_FIFO_EMPTY));
		} while (!(stat & MSC_STAT_DATA_TRAN_DONE));

		while (!(mmc_readl(MSC_IREG) & MSC_IREG_DATA_TRAN_DONE));

		mmc_writel(MSC_IREG_DATA_TRAN_DONE, MSC_IREG);
	}

	return 0;
}

int __mmc_send_status(int timeout)
{
	struct mmc_cmd cmd;
	int err, retries = 5;

	cmd.cmdidx = MMC_CMD_SEND_STATUS;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = rca << 16;

	do {
		err = __mmc_send_cmd(&cmd, NULL);
		if (!err) {
			if ((cmd.response[0] & MMC_STATUS_RDY_FOR_DATA) &&
			    (cmd.response[0] & MMC_STATUS_CURR_STATE) !=
			     MMC_STATE_PRG)
				break;
			else if (cmd.response[0] & MMC_STATUS_MASK) {
				return COMM_ERR;
			}
		} else if (--retries < 0)
			return err;

		udelay(1000);

	} while (timeout--);

	if (timeout <= 0) {
		return TIMEOUT;
	}

	return 0;
}

int __mmc_set_blocklen(int len)
{
	struct mmc_cmd cmd;

	cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = len;

	return __mmc_send_cmd(&cmd, NULL);
}

static int mmc_read_blocks(void *dst, lbaint_t start,
			   lbaint_t blkcnt)
{
	struct mmc_cmd cmd;
	struct mmc_data data;

	if (blkcnt > 1)
		cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
	else
		cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;

	if (high_capacity)
		cmd.cmdarg = start;
	else {
		cmd.cmdarg = start * mmc_read_bl_len;
	}

	cmd.resp_type = MMC_RSP_R1;

	data.dest = dst;
	data.blocks = blkcnt;
	data.blocksize = mmc_read_bl_len;
	data.flags = MMC_DATA_READ;

	if (__mmc_send_cmd(&cmd, &data))
		return 0;

	if (blkcnt > 1) {
		cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1b;
		if (__mmc_send_cmd(&cmd, NULL)) {
			return 0;
		}
	}

	return blkcnt;
}

static ulong mmc_bread(int dev_num, lbaint_t start, lbaint_t blkcnt, void *dst)
{
	int i = 0;
	lbaint_t cur, blocks_todo = blkcnt;

	if (blkcnt == 0)
		return 0;

	if (__mmc_set_blocklen(mmc_read_bl_len))
		return 0;

	do {
		cur = (blocks_todo > MMC_MAX_READ_COUNT_ONCE) ? MMC_MAX_READ_COUNT_ONCE : blocks_todo;
		if(mmc_read_blocks(dst, start, cur) != cur)
			return 0;
		blocks_todo -= cur;
		start += cur;
		dst += cur * mmc_read_bl_len;
		if (i++ % 64 == 0)
			puts("\n");
		puts(".");
	} while (blocks_todo > 0);
	puts("\n");

	return blkcnt;
}

static int mmc_go_idle()
{
	struct mmc_cmd cmd;
	int err;

	udelay(1000);

	cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_NONE;

	err = __mmc_send_cmd(&cmd, NULL);

	if (err)
		return err;

	udelay(2000);

	return 0;
}

static int sd_send_op_cond()
{
	int timeout = 1000;
	int err;
	struct mmc_cmd cmd;

	do {
		cmd.cmdidx = MMC_CMD_APP_CMD;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = 0;

		err = __mmc_send_cmd(&cmd, NULL);

		if (err)
			return err;

		cmd.cmdidx = SD_CMD_APP_SEND_OP_COND;
		cmd.resp_type = MMC_RSP_R3;

		/*
		 * Most cards do not answer if some reserved bits
		 * in the ocr are set. However, Some controller
		 * can set bit 7 (reserved for low voltages), but
		 * how to manage low voltages SD card is not yet
		 * specified.
		 */
		cmd.cmdarg = VOLTAGES & 0xff8000;

		if (mmc_version == SD_VERSION_2)
			cmd.cmdarg |= OCR_HCS;

		err = __mmc_send_cmd(&cmd, NULL);

		if (err)
			return err;

		udelay(1000);
	} while ((!(cmd.response[0] & OCR_BUSY)) && timeout--);

	if (timeout <= 0)
		return UNUSABLE_ERR;

	if (mmc_version != SD_VERSION_2) {
		mmc_version = SD_VERSION_1_0;
	}

	ocr = cmd.response[0];

	high_capacity = ((ocr & OCR_HCS) == OCR_HCS);
	rca = 0;

	return 0;
}

/* We pass in the cmd since otherwise the init seems to fail */
static int mmc_send_op_cond_iter(struct mmc_cmd *cmd, int use_arg)
{
	int err;

	cmd->cmdidx = MMC_CMD_SEND_OP_COND;
	cmd->resp_type = MMC_RSP_R3;
	cmd->cmdarg = 0;
	if (use_arg) {
		cmd->cmdarg =
			(VOLTAGES &
			(op_cond_response & OCR_VOLTAGE_MASK)) |
			(op_cond_response & OCR_ACCESS_MODE);

		if (HOST_CAPS & MMC_MODE_HC)
			cmd->cmdarg |= OCR_HCS;
	}
	err = __mmc_send_cmd(cmd, NULL);
	if (err)
		return err;
	op_cond_response = cmd->response[0];

	return 0;
}

int __mmc_switch(u8 set, u8 index, u8 value)
{
	struct mmc_cmd cmd;
	int timeout = 1000;
	int ret;

	cmd.cmdidx = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
				 (index << 16) |
				 (value << 8);

	ret = __mmc_send_cmd(&cmd, NULL);

	/* Waiting for the ready status */
	if (!ret)
		ret = __mmc_send_status(timeout);

	return ret;

}

static void mmc_set_ios()
{
#ifndef CONFIG_FPGA
	uint32_t real_rate = 0;
	uint32_t lpm = LPM_LPM;
	uint8_t clk_div = 0;

	if (mmc_clock > 1000000) {
		clk_set_rate(MSC0, mmc_clock);
	} else {
		clk_set_rate(MSC0, 24000000);
	}

	real_rate = clk_get_rate(MSC0);

	/* calculate clock divide */
	while ((real_rate > mmc_clock) && (clk_div < 7)) {
		real_rate >>= 1;
		clk_div++;
	}

	mmc_writel(clk_div, MSC_CLKRT);

	if (real_rate > 25000000)
		lpm |= (0x2 << LPM_DRV_SEL_SHF) | LPM_SMP_SEL;

	mmc_writel(lpm, MSC_LPM);
#else
	if(mmc_clock < 400000) {
		/* 1/32 devclk, 384KHz, for init */
		mmc_writel(5, MSC_CLKRT);
	} else {
#ifdef CONFIG_JZ_MMC_MSC1
		mmc_writel(3, MSC_CLKRT);
#else
		/* 1/2 devclk, 12Mhz, for data transfer */
		mmc_writel(1, MSC_CLKRT);
#endif
	}
#endif
	/* set the bus width for the next command */
	mmc_flags &= ~JZ_MMC_BUS_WIDTH_MASK;

	if (bus_width == 8)
		mmc_flags |= JZ_MMC_BUS_WIDTH_8;
	else if (bus_width == 4)
		mmc_flags |= JZ_MMC_BUS_WIDTH_4;
	else
		mmc_flags |= JZ_MMC_BUS_WIDTH_1;
}

void __mmc_set_clock(uint clock)
{
	if (clock > CLK_MAX)
		clock = CLK_MAX;

	if (clock < CLK_MIN)
		clock = CLK_MIN;

	mmc_clock = clock;

	mmc_set_ios();
}

void __mmc_set_bus_width(uint width)
{
	bus_width = width;

	mmc_set_ios();
}

static int mmc_startup()
{
	int err;
	struct mmc_cmd cmd;

	/* Put the Card in Identify Mode */
	cmd.cmdidx = MMC_CMD_ALL_SEND_CID; /* cmd not supported in spi */
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = 0;

	err = __mmc_send_cmd(&cmd, NULL);
	if (err)
		return err;

	/*
	 * For MMC cards, set the Relative Address.
	 * For SD cards, get the Relatvie Address.
	 * This also puts the cards into Standby State
	 */
	cmd.cmdidx = SD_CMD_SEND_RELATIVE_ADDR;
	cmd.cmdarg = rca << 16;
	cmd.resp_type = MMC_RSP_R6;

	err = __mmc_send_cmd(&cmd, NULL);
	if (err)
		return err;

	if (mmc_version & SD_VERSION_SD)
		rca = (cmd.response[0] >> 16) & 0xffff;

	/* Select the card, and put it into Transfer Mode */
	cmd.cmdidx = MMC_CMD_SELECT_CARD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = rca << 16;

	err = __mmc_send_cmd(&cmd, NULL);
	if (err)
		return err;

	/* Restrict card's capabilities by what the host can do */
	mmc_card_caps = HOST_CAPS;

	if (mmc_version & SD_VERSION_SD) {
		if (mmc_card_caps & MMC_MODE_4BIT) {
			cmd.cmdidx = MMC_CMD_APP_CMD;
			cmd.resp_type = MMC_RSP_R1;
			cmd.cmdarg = rca << 16;

			err = __mmc_send_cmd(&cmd, NULL);
			if (err)
				return err;

			cmd.cmdidx = SD_CMD_APP_SET_BUS_WIDTH;
			cmd.resp_type = MMC_RSP_R1;
			cmd.cmdarg = 2;
			err = __mmc_send_cmd(&cmd, NULL);
			if (err)
				return err;

			__mmc_set_bus_width(4);
		}
	} else {
		return -1;
	}

	__mmc_set_clock(mmc_tran_speed);

	return 0;
}

static int mmc_send_if_cond()
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = SD_CMD_SEND_IF_COND;
	/* We set the bit if the host supports voltages between 2.7 and 3.6 V */
	cmd.cmdarg = ((VOLTAGES & 0xff8000) != 0) << 8 | 0xaa;
	cmd.resp_type = MMC_RSP_R7;

	err = __mmc_send_cmd(&cmd, NULL);
	if (err)
		return err;

	if ((cmd.response[0] & 0xff) != 0xaa)
		return UNUSABLE_ERR;
	else
		mmc_version = SD_VERSION_2;

	return 0;
}

int __mmc_init()
{
	int err;

	__mmc_set_bus_width(1);

	__mmc_set_clock(1);

	/* Reset the Card */
	err = mmc_go_idle();
	if (err)
		return err;

	/* Test for SD version 2 */
	mmc_send_if_cond();

	/* Now try to get the SD card's operating condition */
	err = sd_send_op_cond();
	if (!err)
		err = mmc_startup();

	return err;
}

static int mmc_load_image_raw(unsigned int offset, unsigned int len, unsigned int dst_addr)
{
	unsigned long ret = 0;
	u32 start_sector;
	u32 total_sectors;

	ret = __mmc_init();
	if (ret) {
		puts("ERROR: mmc init failed!\n");
		return ret;
	}

	start_sector = offset / mmc_read_bl_len;
	total_sectors = (len + mmc_read_bl_len - 1) / mmc_read_bl_len;
	printf("MMC: start = [%d], sectors = [%d], dst = [0x%x]\n",
	       start_sector, total_sectors, dst_addr);

	ret = mmc_bread(0, start_sector, total_sectors, (void *)dst_addr);
	if (ret == 0) {
		puts("ERROR: sd load failed!\n");
		return -1;
	}

	return 0;
}

void sd_load_kernel(unsigned int offset, unsigned int len, unsigned int dst_addr)
{
	mmc_load_image_raw(offset, len, dst_addr);
}
