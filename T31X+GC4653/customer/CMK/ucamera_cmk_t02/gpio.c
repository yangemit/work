#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "gpio.h"

#define MSG(args...) printf(args) 

//º¯ÊýÉùÃ÷


int gpio_export(int pin)  
{  
    FILE *fd;  
  
    fd = fopen("/sys/class/gpio/export", "w");  
    if (fd < 0) {  
        MSG("Failed to open export for writing!\n");  
        return(-1);  
    }  
  
    fprintf(fd,"%d",pin);
	fclose(fd);

    return 0;  
}  

 int gpio_unexport(int pin)  
{  
    char buffer[64];  
    int len;  
    int fd;  
  
    fd = fopen("/sys/class/gpio/unexport", "w");  
    if (fd < 0) {  
        MSG("Failed to open unexport for writing!\n");  
        return -1;  
    }  
  
    fprintf(fd,"%d",pin);
	fclose(fd);
    return 0;  
} 

//dir: 0-->IN, 1-->OUT
 int gpio_direction(int pin, int dir)  
{  
    char path[64];  
    FILE *fd;  
  
    sprintf(path, "/sys/class/gpio/gpio%d/direction", pin);  
    fd = fopen(path, "w");  
    if (fd < 0) {  
        MSG("Failed to open gpio direction for writing!\n");  
        return -1;  
    }  
   if(dir==1)//output
   	{	
		fprintf(fd, "out");
	 }
	else
	{
		fprintf(fd, "in");
	 }
   	fclose(fd);
    return 0;  
}  

//value: 0-->LOW, 1-->HIGH
 int gpio_write(int pin, int value)  
{  
    char path[64];  
    FILE *fd;  
  
    sprintf(path,"/sys/class/gpio/gpio%d/value", pin);  
    fd = fopen(path, "w");  
    if (fd < 0) {  
        MSG("Failed to open gpio value for writing!\n");  
        return -1;  
    }  
    if(value==0){
		    fprintf(fd, "%d", 0);
    	}
		else{
		    fprintf(fd, "%d", 1);
		}
	fclose(fd);
    return 0;  
}

 int gpio_read(int pin)  
{  
    char path[64];  
    char i;
    FILE *fd;  

    sprintf(path,  "/sys/class/gpio/gpio%d/value", pin);  
    fd = fopen(path, "r");  
    if (fd < 0) {  
        MSG("Failed to open gpio value for reading!\n");  
        return -1;  
    }  
  
    fseek(fd , 0 , 0);
    fread(&i , 1, 1 ,fd);
    fclose(fd);  
    


		
/*
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);

    if ((fd = open(path, O_RDONLY)) == -1)
        return -1;

    if (read(fd, &i, sizeof(i)) != sizeof(i)) {
        close(fd);
        return -1;
    }

    if (close(fd) == -1)
        return -1;
  */
    return (i - '0');
  
}
  
