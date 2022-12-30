/*********************************************************
 * File Name   : serial_ak_update.c
 * Author      : Jasmine
 * Mail        : jian.dong@ingenic.com
 * Created Time: 2021-09-06 14:13
 ********************************************************/
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/completion.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/reboot.h>

#include "u_serial_update.h"
#include "usb_update.h"

#define UPGRADE_NODE_NUM		100 
#define UPGRADE_BUF_SIZE		1024 

#define HEAD_LENGTH			6
#define MD5SUM_LENGTH			6

#define FRAME_NUMBER_OFFSET 		6
#define FRAME_LENGTH_OFFSET 		7
#define FRAME_DATA_OFFSET 		9
#define DATA_FRAME_HEDA_LEN 		13	
#define DATA_FRAME_HEDA_LEN 		13	

enum frame_number
{
	START_FRAME = 1,
	INTERVAL_FRAME,
	DATA_FRAME,
	DATA_VALI_FRAME,
	FINISH_FRAME,
	UPGRADE_VALI_FRAME,
	FRAME_TYPE_NUM
};

typedef struct _update_pack_info {
	u8  EnableOTA;
	u8  TimeoutTime;
	u16 PFrameSize;
	u32 PkgSize;
	u32 FrameTotalCount;
	u16 PkgVersion;
	u16 FrameQuantityAtOnce;
} update_pack_info;

typedef struct _update_frame_info {
	u16 FrameLength;
	u8  CS;
	u8  FrameNumber;
	u32 FrameOfStar;
	u32 DataFrameIndex;
	u32 CurrentIndex;
	u32 BatchIndex;
	u32 Memcpy_count;
} update_frame_info;

typedef struct _f_ak_update {
	u8 *firmware_data_buf;
	struct completion  malloc_block;
	struct completion  finish_block;
	u32 img_offset;
	u32 img_size;
} f_ak_update;

typedef struct _firmware_head {
	unsigned int img_offset;
	unsigned int img_size;
	unsigned char md5[16];
} firmware_head_t;


#define PACK_INFO_LENGTH 		sizeof(update_pack_info)	

update_frame_info frame_info; 
f_ak_update ak_update;
update_pack_info pack_info;

static int receive_data_check(u8 *buf, u8 index)
{
	int i;
	u8 receive_data_head[HEAD_LENGTH] = {0x08, 0xEE, 0x00, 0x00, 0x00, 0x07};
	u8 data_head[HEAD_LENGTH];
	u8 *data_buf;
	u8 crc = 0;
	data_buf = buf;

	for(i = 0; i < frame_info.FrameLength - 1; i++){
		crc += data_buf[i]; 
	}

	if(crc != frame_info.CS){
		printk("#########receive frame=%d CS is error(%x),%x########\n", index, crc, frame_info.CS);
		for(i = 0; i < frame_info.FrameLength; i++){
			if(data_buf[i] != 0xaa)
				printk("data[%d]=%x\n", i, data_buf[i]);
		}
		return -1;
	}

	memcpy(data_head, buf, HEAD_LENGTH);
	for(i = 0; i < HEAD_LENGTH ; i++)
	{
		if(data_head[i] != receive_data_head[i]){
			printk("receive frame=%d, data[%d] head is error(%d)\n", index, i, data_head[i]);
			return -1;
		}
	}
	return 0;
	
};

static int send_frame(u8 index, void *data, int data_len)
{
	u8 send_data_buf[64];
	u16 length = 0;
	u8  crc = 0;
	int i, count;
	u8 send_data_head[HEAD_LENGTH] = {0x09, 0xff, 0x00, 0x00, 0x01, 0x07};

	memcpy((void *)send_data_buf, send_data_head, HEAD_LENGTH);
	send_data_buf[FRAME_NUMBER_OFFSET] = index; 
	length = HEAD_LENGTH + sizeof(index) + sizeof(length) + data_len + sizeof(crc);
	memcpy((void *)send_data_buf + FRAME_LENGTH_OFFSET,  (void *)&length, sizeof(length));
	if(data != NULL)
		memcpy(send_data_buf + FRAME_DATA_OFFSET, data, data_len);

	send_data_buf[length - 1] = 0;
	for(i = 0; i < length -1; i++)
		crc += send_data_buf[i];
	send_data_buf[length - 1] = crc;
	count = gs_write_cdc(send_data_buf, length);
	if(count != length){
		printk("send frame data is failed\n");
		return -1;
	}
	return 0;

};

static int get_firmware_head_info(void *buf)
{
	int i;
	firmware_head_t head;
	unsigned char buf_md5sum[16];

	memcpy((void *)&head, buf, sizeof(firmware_head_t));
	ak_update.img_offset = head.img_offset;
	ak_update.img_size   = head.img_size;

	buffer_md5sum_0((char *)ak_update.firmware_data_buf + HEAD_INFO_RESERVED, buf_md5sum, ak_update.img_size);

	for(i = 0; i < MD5SUM_LENGTH; i++){ 
		if(buf_md5sum[i] != head.md5[i]){ 
			printk("firmware md5 check is error,buf_md5sum[15]=%d, head.md5[15]=%d\n", buf_md5sum[15], head.md5[16]);
			return -1;
		}
	}
	return 0;

}

static void data_frame_process(void *buf, u8 index) 
{
	static int data_frame_err;
	u8	*data_buf;
	u16 VerifyStatus;
	int ret = -1, copy_len = 0;

	data_buf = (u8 *)buf;
	if (ak_update.firmware_data_buf == NULL)
		printk("firmware_data_buf=NULL\n");

	memcpy((void *)&frame_info.DataFrameIndex, data_buf + FRAME_DATA_OFFSET, sizeof(u32));

	ret = receive_data_check(data_buf, index);
	if((ret < 0) || (frame_info.DataFrameIndex != frame_info.CurrentIndex + frame_info.BatchIndex)){
		if(data_frame_err > 3){
			printk("this data more than three error frameindex=%d\n",frame_info.DataFrameIndex);
			data_frame_err = 0;
			VerifyStatus = 0;
			send_frame(FINISH_FRAME, (void *)&VerifyStatus, sizeof(VerifyStatus));
		} else {
			printk("frame data error frameindex=%d\n",frame_info.DataFrameIndex);
			data_frame_err++;
			frame_info.CurrentIndex += frame_info.BatchIndex;
			frame_info.FrameOfStar = frame_info.CurrentIndex;
			frame_info.BatchIndex = 0;
			send_frame(INTERVAL_FRAME, (void *)&frame_info.FrameOfStar, sizeof(frame_info.FrameOfStar));
		}
		return;
	}

	data_frame_err = 0;
	copy_len = frame_info.FrameLength - DATA_FRAME_HEDA_LEN - sizeof(frame_info.CS);
	memcpy(ak_update.firmware_data_buf + frame_info.Memcpy_count, data_buf + DATA_FRAME_HEDA_LEN,  copy_len);
	frame_info.Memcpy_count += copy_len;

	if(frame_info.Memcpy_count > pack_info.PkgSize)
	{
		printk("send data length %d is greater than Pkgsize=%d, frame_info.DataFrameIndex=%d, frame_info.FrameLength=%d\n", frame_info.Memcpy_count, pack_info.PkgSize, frame_info.DataFrameIndex, frame_info.FrameLength);
	};

	if(frame_info.DataFrameIndex == pack_info.FrameTotalCount - 1){
		ret = get_firmware_head_info(ak_update.firmware_data_buf);
		if(ret < 0)
			VerifyStatus = 0;
		else
			VerifyStatus = 1;
		complete(&ak_update.finish_block);
		send_frame(FINISH_FRAME, (void *)&VerifyStatus, sizeof(VerifyStatus));
	} else {
		if(frame_info.BatchIndex == pack_info.FrameQuantityAtOnce - 1){
			frame_info.BatchIndex = 0;
			frame_info.CurrentIndex += pack_info.FrameQuantityAtOnce;
			frame_info.FrameOfStar = frame_info.CurrentIndex;
			send_frame(INTERVAL_FRAME, (void *)&frame_info.FrameOfStar, sizeof(frame_info.FrameOfStar));
			printk("####Currentindex=%d######\n", frame_info.CurrentIndex);
			return;
		}
		frame_info.BatchIndex += 1;
	}
	return;
}

int get_update_data(void *buf, int len)
{
	int ret = -1;
	u8      index;
	u8	pack_info_data[PACK_INFO_LENGTH];	
	u8	*data_buf;
	data_buf = (u8 *)buf;
	
	memcpy(&frame_info.FrameLength, data_buf + FRAME_LENGTH_OFFSET, sizeof(u16));
	frame_info.FrameNumber = data_buf[FRAME_NUMBER_OFFSET];
	frame_info.CS = data_buf[frame_info.FrameLength - 1];

	index = frame_info.FrameNumber;

	switch(index){
		case START_FRAME:
			ret = receive_data_check(data_buf, index); 
			if(ret){
				printk("START FRAME is check failed\n");
				break;
			}

			memcpy(pack_info_data, data_buf + FRAME_DATA_OFFSET , PACK_INFO_LENGTH);
			pack_info = *(update_pack_info *)pack_info_data;
			printk("EnableOTA=%d, Timeouttime=%d, PFrameSize=%d, PkgSize=%d, FrameTotalCount=%d, Pkgversion=%d, FrameQuantityAtOnce=%d\n",pack_info.EnableOTA, pack_info.TimeoutTime, pack_info.PFrameSize, pack_info.PkgSize, pack_info.FrameTotalCount, pack_info.PkgVersion, pack_info.FrameQuantityAtOnce);

			if(ak_update.firmware_data_buf !=NULL)
				kfree(ak_update.firmware_data_buf);
			memset(&frame_info, 0, sizeof(update_frame_info));

			complete(&ak_update.malloc_block);

			send_frame(INTERVAL_FRAME, (void *)&frame_info.FrameOfStar, sizeof(frame_info.FrameOfStar));
			break;

		case DATA_FRAME: 
			data_frame_process(buf, DATA_FRAME);
			break;
		default:
			printk("FrameNumber=%d, FrameLength=%d, CS=%d, Currentindex=%d, Batchindex=%d\n",frame_info.FrameNumber, frame_info.FrameLength, frame_info.CS, frame_info.CurrentIndex, frame_info.BatchIndex);
			break;
	}
	return 0;
};

static int ak_update_thread(void *data)
{
	int flash_flag = 0;

	wait_for_completion(&ak_update.malloc_block);
	if(ak_update.firmware_data_buf == NULL){
		ak_update.firmware_data_buf = kmalloc(pack_info.PkgSize, GFP_KERNEL);
		if(ak_update.firmware_data_buf == NULL){
			printk("kmalloc cdc data buf is failed\n");
		}
	}

	wait_for_completion(&ak_update.finish_block);
	/*写入升级固件*/
	direct_erase_norflash(ak_update.img_offset, ak_update.img_size);
	direct_write_norflash(ak_update.firmware_data_buf + HEAD_INFO_RESERVED, ak_update.img_offset, ak_update.img_size);

/*重置flash中的标志位*/
	flash_flag = NORFALSH_RESET_SIGNATURE; 
	direct_erase_norflash(NORFLASH_OFFSET, sizeof(int));
	direct_write_norflash(&flash_flag, NORFLASH_OFFSET, sizeof(int));

	machine_restart("Restart the system after the upgrade is complete");
	
	return 0;
}

int cdc_serial_update(void)
{
	struct task_struct *update_thread;

	update_thread = kthread_create(ak_update_thread, NULL, "usb-serial-update");	
	if(IS_ERR(update_thread))
	{
		return PTR_ERR(update_thread);
	}
	
	init_completion(&ak_update.malloc_block);
	init_completion(&ak_update.finish_block);
	wake_up_process(update_thread);

	return 0;
}



