#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

/* 设配所对应的I2C控制器的设备节点 */
#define I2C_DEVICE   "/dev/i2c-1"
/* I2C设备地址 */
#define I2C_ADDR  0x34


int main()
{
	int fd,i,ret=0;
	unsigned char w_add=0x00;

	struct timeval time;
	static long int t1 = 0, t2 = 0;

	/* 将要读取的数据buf*/
	unsigned char rd_buf[200] = {0x00};
	/* 要写的数据buf，第0个元素是要操作eeprom的寄存器地址*/
	unsigned char wr_buf[13] = {0};

	printf("hello,this is read_write i2c test \n");

	/* 打开eeprom对应的I2C控制器文件 */

	fd =open(I2C_DEVICE, O_RDWR | O_NONBLOCK);
	if (fd< 0)
	{
		printf("open"I2C_DEVICE"failed \n");
	}

	/*设置eeprom的I2C设备地址*/
	if (ioctl(fd,I2C_SLAVE_FORCE, I2C_ADDR) < 0)
	{

		printf("set slave address failed \n");
	}

	/* 将要操作的寄存器首地址赋给wr_buf[0] */
	wr_buf[0] = w_add;

	/* 把要写入的数据写入到后面的buf中 */
	/*for(i=1;i<13;i++)*/
		/*wr_buf[i]=i;*/

	/* 通过write函数完成向eeprom写数据的功能 */
	//write(fd, wr_buf, 13);

	/* 延迟一段时间 */
	//sleep(1);a

	/*重新开始下一个操作，先写寄存器的首地址*/
	//write(fd, wr_buf, 1);
	/* 从wr_buf[0] = w_add的寄存器地址开始读取12个字节的数据 */
	while(1) {

		int flag = 0;
		ret=read(fd, rd_buf, 10);
		if( ret < 0 ) {
			printf("[ERORR] read i2c failed /n");
			continue;
		}
		//printf("ret is %d \r\n",ret);q
		for(i=0;i<10;i++)
		{	
			if ( rd_buf[i] != 0x55 ) {
				flag = 1;
				break;
			}
		}
		if ( flag == 1 ) {
		gettimeofday(&time, NULL);
		t1 = time.tv_sec * 1000 + time.tv_usec / 1000;
		printf("[INFO] time --> %ld value --> %d \n",t1-t2,rd_buf[0]);
		t2 = t1;
		flag = 0;	
		}
		/*if( rd_buf[0] != 0x55) {*/

			/*for(i=0;i<100;i++)*/
			/*{	*/
				/*printf("rd_buf is :%d\n",rd_buf[i]);*/
			/*}*/
		
		/*}*/
		memset(&rd_buf,0,sizeof(rd_buf));

	}
	/* 完成操作后，关闭eeprom对应的I2C控制器的设备文件 */
	close(fd);

	return 0;

}
