#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <string.h>
#include <mtd/mtd-user.h>



int flash_region_erase(int Fd, int start, int count, int unlock)
{
    mtd_info_t meminfo;
    erase_info_t erase;

    if (ioctl(Fd,MEMGETINFO,&meminfo) == 0)
    {
        erase.start = start;
        erase.length = meminfo.erasesize;

        for (; count > 0; count--) 
         {
          //  INFO(" Flash Erase of length %u at offset 0x%x\n", erase.length, erase.start);
            fflush(stdout);
            if(unlock != 0)
            {
                //Unlock the sector first.
                printf(" Flash unlock at offset 0x%x\n",erase.start);
                if(ioctl(Fd, MEMUNLOCK, &erase) != 0)
                {
                    printf("MTD Unlock failure\n");
                    return -1;
                }
            }

            if (ioctl(Fd,MEMERASE,&erase) != 0)
            {
                printf("MTD Erase failure\n");
                return -1;
            }
            erase.start += meminfo.erasesize;
        }
        printf(" erase done\n");
    }
     return 0;
}
int mtd_flash_write(int fd,unsigned int addr_offset,unsigned char*data,int data_len)
{
		int ret=-1;
		unsigned char* pData=data;
		printf("flash program  len= %d\n",data_len);
		lseek(fd, addr_offset, SEEK_SET);//设置指针位置为0
		if((write(fd,pData,data_len))!=data_len)
		{
			printf("write mtd device error! startaddr=0x%x\n",addr_offset);
			return -1;
		}   
		 return 0;
}
int mtd_flash_write_otp(int fd,unsigned int addr_offset,unsigned char*data,int data_len)
{
		int  val;
		unsigned char* pData=data;
		int ret=-1;

		val =MTD_OTP_USER;
		ret = ioctl(fd, OTPSELECT, &val);
		if (ret < 0) {
				printf("OTP SELECT error\n");
				return ret;
		}

		printf("mtd_flash_write_otp  len= %d\n",data_len);
		ret=lseek(fd, addr_offset, SEEK_SET);//设置指针位置为0
		if (ret < 0) {
				printf("lseek() err\n");
				return ret;
		}
		if((write(fd,pData,data_len))!=data_len)
		{
				printf("write mtd device error! startaddr=0x%x\n",addr_offset);
				return -1;
		}   
		val =MTD_OTP_OFF;
		ret = ioctl(fd, OTPSELECT, &val);
		
		return ret;
}

int mtd_flash_read(int fd,unsigned int addr_offset,unsigned char*data,int data_len)
{
	int read_len;
	int i;
	lseek(fd,addr_offset, SEEK_SET);
	read_len = 0;
	read_len = read(fd, data, data_len);
	printf("r_len is %d : \n", read_len);
	for(i=0;i<data_len;i++)
	{
	printf("%x,", data[i]);
	}
	return read_len;
}
int mtd_flash_read_otp(int fd,unsigned int addr_offset,unsigned char*data,int data_len)
{
	int read_len;
	int i;
	int  val,ret;
	val =MTD_OTP_USER;
	ret = ioctl(fd, OTPSELECT, &val);
	if (ret < 0) {
			printf("OTPSELECT error");
			return ret;
	}
	lseek(fd,addr_offset, SEEK_SET);
	read_len = 0;
	read_len = read(fd, data, data_len);
	printf("r_len is %d : \n", read_len);
	for(i=0;i<data_len;i++)
	{
		printf("%x,", data[i]);
	}
	val =MTD_OTP_OFF;
	ret = ioctl(fd, OTPSELECT, &val);
	
	return ret;
}
