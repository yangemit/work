#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PNG_BYTES_TO_CHECK 4

/*
	将png -> argb 
*/
int load_png_image( const char *filepath)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep* row_pointers;
	char buf[PNG_BYTES_TO_CHECK];
	int w, h, x = 0, y, temp, color_type;
	int  fd = 0, size = 0;
	unsigned char * buf_f = (unsigned char *)malloc(4);
	
	printf("[INFO] png_argb %s \n",filepath);
	fp = fopen( filepath, "rb" );
	if( fp == NULL ) { 
		printf("open filepath error\n");
		return -1;
	}
	/*
	   读取png 文件大小                                             
	   */
	fd = open("/tmp/ARGB", O_RDWR|O_CREAT|O_TRUNC, 0777);
	printf("[INFO] argb_file %s \n","/tmp/ARGB");
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
						printf("write        error\n");
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
/*
	将图像缩放成对应比例
	str_dir：需要处理的 png图像
	strout : 处理后生成的图像 （/tmp/tmp.png）
*/
int resize_png(char const *str_dir,int nDestWidth, int nDestHeight)
{
	char const *str = NULL;
	char const *strout = "/tmp/tmp.png";

	int iw=0, ih=0, n=0;
	int ow=0, oh=0;
	unsigned char *odata = NULL;
	unsigned char *idata = NULL;

	str = (char const *)str_dir;

	idata = stbi_load(str, &iw, &ih, &n, 0);
	printf("iw = %d ih = %d n = %d \n", iw,ih,n);

	ow = nDestWidth;
	oh = nDestHeight;
	printf("ow = %d oh = %d \n", ow,oh);
	odata = malloc(ow * oh * n);
	stbir_resize(idata, iw, ih, 0, odata, ow, oh, 0,
			STBIR_TYPE_UINT8, n, 0, 0,
			STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
			STBIR_FILTER_BOX, STBIR_FILTER_BOX,
			STBIR_COLORSPACE_SRGB, NULL
		    );

	stbi_write_png(strout, ow, oh, n, odata, 0);

	stbi_image_free(idata);
	stbi_image_free(odata);
	printf("[INFO] str %s \n",str);
	printf("[INFO] strout %s \n",strout);
	printf("Resize success!!! \n");
	return 0;
}


