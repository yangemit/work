#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

	unsigned int width , height;

	if ( argc > 3 || argc < 3 ) {
		printf("such as: ./a.out width height\n");
		return 0;
	} else if ( argc == 3 ) {
		width = (unsigned int)atoi(argv[1]);
		height = (unsigned int)atoi(argv[2]);
		printf("yuyv frame size %d \n", width * height *2);
	}
	
	int usb_packet[3] = {1 * 8 * 1024,2 * 8 * 1024,3 * 8 * 1024};
	
	int i;
	
	int fps;
	for ( i = 0; i < 3; i++) {
		fps = ( usb_packet[i] * 1024 ) / ( width * height * 2 );
		printf("[INFO] usb_packet %d to fps\n",usb_packet[i]/8);
		printf("[INFO] %d \n",fps);
	}

	return 0;
}
