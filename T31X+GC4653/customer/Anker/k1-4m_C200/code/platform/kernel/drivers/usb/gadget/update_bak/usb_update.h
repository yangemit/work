/*********************************************************
 * File Name   : usb_update.h
 * Author      : Jasmine
 * Mail        : jian.dong@ingenic.com
 * Created Time: 2021-09-13 16:44
 ********************************************************/

#ifndef __USB_UPDATE_H
#define __USB_UPDATE_H

#include <linux/mtd/direct_norflash.h>

#define NORFALSH_RESET_SIGNATURE		(0xffffffff)
#define NORFLASH_OFFSET				(0xE00020)
#define HEAD_INFO_RESERVED			128

/*
 *int read_flash(void *buf, int offset, int size);
 *int erase_flash(int offset, int size);
 *int write_flash(void *buf, int deviation, int size);
 */

int buffer_md5sum_0(char *data_buf, unsigned char md5[16], int size_len);

#endif

