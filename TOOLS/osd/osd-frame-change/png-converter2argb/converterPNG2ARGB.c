#include <png.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#define PNG_BYTES_TO_CHECK 4

int load_png_image( const char *filepath)
{
        FILE *fp;
        png_structp png_ptr;
		png_infop info_ptr;
		png_bytep* row_pointers;
		char buf[PNG_BYTES_TO_CHECK];
		int w, h, x, y, temp, color_type, b;
		int i = 0,ret = 0, fd = 0, size = 0;
		unsigned char * buf_f = (unsigned char *)malloc(4);

        fp = fopen( filepath, "rb" );
        if( fp == NULL ) { 
			printf("open filepath error\n");
                return -1;
        }
		/*
			读取png 文件大小
		*/
		fd = open("ARGB", O_RDWR|O_CREAT|O_TRUNC, 0777);
		size = getFileSizeSystemCall(filepath);
		printf("file write size :%d\n", size);

		png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );
        info_ptr = png_create_info_struct( png_ptr );

        setjmp( png_jmpbuf(png_ptr) ); 

        temp = fread( buf, 1, PNG_BYTES_TO_CHECK, fp );

        if( temp < PNG_BYTES_TO_CHECK ) {
                fclose(fp);
		png_destroy_read_struct( &png_ptr, &info_ptr, 0);
                return -1;
        }

        temp = png_sig_cmp( (png_bytep)buf, (png_size_t)0, PNG_BYTES_TO_CHECK );

			printf("7\n");
        if( temp != 0 ) {
                fclose(fp);
		png_destroy_read_struct( &png_ptr, &info_ptr, 0);
                return -1;
        }
        
		rewind( fp );
		png_init_io( png_ptr, fp ); 
		png_read_png( png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0 );
		color_type = png_get_color_type( png_ptr, info_ptr );
		w = png_get_image_width( png_ptr, info_ptr );
		h = png_get_image_height( png_ptr, info_ptr );
		row_pointers = png_get_rows( png_ptr, info_ptr );
        switch( color_type ) {
			case PNG_COLOR_TYPE_RGB_ALPHA:
				for( y=0; y<h; ++y ) {
					for( x=0; x<w*4; x++) {
						buf_f[2] = /**(bufv + y*w + x-1) =*/ row_pointers[y][x++]; // red
						buf_f[1] = /**(bufv + y*w + x-1) =*/ row_pointers[y][x++]; // green
						buf_f[0] = /**(bufv + y*w + x-1) =*/ row_pointers[y][x++]; // blue
						buf_f[3] = /**(bufv + y*w + x-1) =*/ row_pointers[y][x]; // alpha
						/*
							将转换的argb 数据写到
							ARGB 文件里
						*/
						if(write(fd, buf_f, 4) != 4)
								printf("write error\n");
					}
				}
				printf("y = %d, x = %d\n", y, (x/4));
				close(fd);
				break;

			case PNG_COLOR_TYPE_RGB:
				for( y=0; y<h; ++y ) {
					for( x=0; x<w*3; ) {
					}
				}
				break;
			default:
                fclose(fp);
		png_destroy_read_struct( &png_ptr, &info_ptr, 0);
                return 0;
        }
        png_destroy_read_struct( &png_ptr, &info_ptr, 0);
        fclose(fp);
	return 0;
}

int getFileSizeSystemCall(char * strFileName) 
{
	struct stat temp;
	stat(strFileName, &temp);
	return temp.st_size;
}
/*
     功能描述
     输入一个**.png 将其转换为图像格式为argb----ARGB 文件 
*/

int main(int argc, const char *argv[])
{
	int i = 0,ret = 0, fd = 0, size = 0;
	if(argc != 2)
	{
		printf("[INFO] such as : ./converterPNG2ARGB **.png \n");
		return 0;
	}
	printf("[INFO] %s \n",argv[1]);
//	unsigned char * buf = (unsigned char *)malloc(800*480*4);
//	printf("1\n");
	ret = load_png_image( argv[1] );
//	printf("2\n");

//	fd = open("png2argb", O_RDWR|O_CREAT|O_TRUNC, 0777);
//	size = getFileSizeSystemCall(filename);
///			printf("write %d\n", size);

//	for (i = 0; i < size; i++){
//		if(write(fd, buf+i, 1) != 1)
//			printf("write error\n");
//	}
	
//	close(fd);

	return 0;
}
