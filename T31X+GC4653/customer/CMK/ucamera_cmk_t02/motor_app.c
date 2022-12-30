#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "motor_app.h"

#define	MOTOR_IOC_MAGIC	'm'
#define	MOTOR_IOCGETSPEED	_IOR(MOTOR_IOC_MAGIC,1,int)
#define	MOTOR_IOCSETSPEED	_IOW(MOTOR_IOC_MAGIC,2,int)
#define	MOTOR_IOCSETLEFT	_IOW(MOTOR_IOC_MAGIC,3,int)
#define	MOTOR_IOCSETRIGHT	_IOW(MOTOR_IOC_MAGIC,4,int)
#define    MOTOR_IOCSETSTOP          _IOR(MOTOR_IOC_MAGIC,5,int)
#define    MOTOR_IOCGET_STATUS _IOR(MOTOR_IOC_MAGIC,6,int)
#define	MOTOR_IOCGET_POS	 _IOR(MOTOR_IOC_MAGIC,7,int)
#define MOTOR_IOCGET_STEP_CNT _IOR(MOTOR_IOC_MAGIC,8,int)
#define	MOTOR_IOC_MAXNR		8
 

 
#define	MOTOR_DEV	"/dev/motor"
int motor_drv_fd=0;

int open_motor_dev(void)
{
	motor_drv_fd = open(MOTOR_DEV,O_RDWR);
	if(motor_drv_fd < 0)
	{
		perror("Open /dev/pwm Fail");
	}	
	return motor_drv_fd;
}
int motor_get_speed(void)
{
	unsigned int arg;
	int ret = ioctl(motor_drv_fd,MOTOR_IOCGETSPEED,&arg);
	if(ret < 0)
	{
		perror("Get the frequency of motor fail");
		return ret;
	}
	else
	{
		printf("Get the frequency of motor is %dHZ\n",arg);
	}
	return arg;
}
int motor_get_pos(void)
{
	unsigned int arg;
	int ret = ioctl(motor_drv_fd,MOTOR_IOCGET_POS,&arg);
	if(ret < 0)
	{
		perror("Get the position of motor fail");
		return ret;
	}
	else
	{
		printf("Get the pos of motor is %d\n",arg);
	}
	return arg;
}
int motor_set_speed(unsigned int speed)
{
	unsigned int arg=speed;

	int ret = ioctl(motor_drv_fd,MOTOR_IOCSETSPEED,&arg);
	if(ret < 0)
	{
		perror("Set the frequency of motor fail");		
	}
	else
	{
		printf("Set the frequency of motor is %dHZ\n",arg);
	}
	return ret;
}
int motor_close_door(void)
{  
	int ret=ioctl(motor_drv_fd,MOTOR_IOCSETRIGHT,NULL);	
	return ret;
}
int motor_open_door(void)
{  
	int ret=ioctl(motor_drv_fd,MOTOR_IOCSETLEFT,NULL);	
	return ret;
}
int motor_stop(void)
{  
	int ret=ioctl(motor_drv_fd,MOTOR_IOCSETSTOP,NULL);	
	return ret;
}
int motor_get_status(void)
{
	unsigned int arg;
	int ret = ioctl(motor_drv_fd,MOTOR_IOCGET_STATUS,&arg);
	if(ret < 0)
	{
		perror("Get the status of motor fail");
		return ret;
	}
	else
	{
		printf("Get the status of motor is %d\n",arg);
	}
	return arg;
}
#define MOTOR_MAX_STEP 250
char motor_door_opened(void)
{
	unsigned int steps;
	int run_status;
	while(motor_get_status()!=eMOTOR_ACTION_IDLE)
	{
      usleep(5*1000);
	}
	int ret = ioctl(motor_drv_fd,MOTOR_IOCGET_STEP_CNT,&steps);
	if(ret < 0)
	{
		perror("ioctl MOTOR_IOCGETSPEED fail");
		return 0;
	}
	else
	{
		printf("motor steps cnt %d\n",steps);
	}
  return(steps<MOTOR_MAX_STEP);
}
char motor_door_closed(void)
{
	unsigned int steps;
	int run_status;
	while(motor_get_status()!=eMOTOR_ACTION_IDLE)
	{
      usleep(5*1000);
	}
	int ret = ioctl(motor_drv_fd,MOTOR_IOCGET_STEP_CNT,&steps);
	if(ret < 0)
	{
		perror("ioctl MOTOR_IOCGETSPEED fail");
		return 0;
	}
	else
	{
		printf("motor steps cnt %d\n",steps);
	}
  return(steps<MOTOR_MAX_STEP);
}

