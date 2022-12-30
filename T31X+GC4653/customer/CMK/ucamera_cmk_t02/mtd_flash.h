#ifndef __MTD_FLASH_H__
#define __MTD_FLASH_H__
/* OTP mode selection */
#define MTD_OTP_OFF		0
#define MTD_OTP_FACTORY		1
#define MTD_OTP_USER		2
int flash_region_erase(int Fd, int start, int count, int unlock);
int mtd_flash_write(int fd,unsigned int addr_offset,unsigned char*data,int data_len);
int mtd_flash_read(int fd,unsigned int addr_offset,unsigned char*data,int data_len);
int mtd_flash_write_otp(int fd,unsigned int addr_offset,unsigned char*data,int data_len);
int mtd_flash_read_otp(int fd,unsigned int addr_offset,unsigned char*data,int data_len);

#endif
