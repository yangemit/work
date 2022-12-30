/*********************************************************
 * File Name   : test.c
 * Author      : Elvis Wang
 * Mail        : huan.wang@ingenic.com
 * Created Time: 2020-06-28 21:28
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>


#include <linux/ioctl.h>

#include <stdint.h>

#define MOTOR_MOVE 	_IOW('M', 1 , int)

int main(int argc , const char * argv[])
{
	int fd = open("/dev/dw9714" , O_RDWR);
	int test_code = 0;

	while(1)
	{
		printf("please input value \n");
		scanf("%d" , &test_code);
		ioctl(fd, MOTOR_MOVE, test_code);
	}

	return 0;
}
