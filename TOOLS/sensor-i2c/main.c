#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, const char *argv[])
{
	unsigned  int tmp = 0;

	if(argc > 2)
	{
		printf("such as: ./i2c 0x29\n");
		return 0;
	}else if(argc == 2)
	{
		tmp = strtol(argv[1],NULL,16);
		tmp = tmp >> 1;
		printf("want get I2C:0x%x \n", tmp);
	}
	return 0;
}
