#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <stdbool.h>

#include <jpeg_control.h>

static int get_file_size(char* filename)
{
	struct stat statbuf;
	stat(filename,&statbuf);
	int size = statbuf.st_size;

	return size;
}

static int read_jpeg_file(const char*file_name, int file_size, void *out_data)
{

	if (!file_name || !out_data) {
		printf("ERROR(%s): input data is invalid \n", __func__);
		return -1;
	}

	int rsize = 0;
	FILE *fp = NULL;

	fp = fopen(file_name, "r+");
	if (!fp) {
		printf("ERROR(%s): open %s failed \n", __func__, file_name);
		return -1;
	}

	rsize = fread(out_data, 1, file_size, fp);
	if(rsize != file_size) {
		printf("ERROR(%s): read %d btye, actual %d byte \n", __func__, rsize, file_size);
		return -1;
		fclose(fp);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret = -1;
	if(argc != 3){
		printf("please input ./jpeg_decode input.jpeg output.nv12!");
		return -1;
	}

	int file_size = get_file_size(argv[1]);

	unsigned char *jpeg_data = NULL;
	jpeg_data = (unsigned char*)malloc(file_size);
	if (!jpeg_data) {
		printf("ERROR(%s): malloc jpeg data error\n", __func__);
		return -1;
	}

	ret = read_jpeg_file(argv[1], file_size, jpeg_data);
	if (ret < 0) {
		printf("ERROR(%s): read jpeg file error\n", __func__);
		return -1;
	}
#define RESIZE_JPEG
	unsigned char *jpeg2_data = NULL;
#ifdef RESIZE_JPEG
	jpeg2_data = resize_jpeg_file(jpeg_data, file_size, 640, 360, 24, (unsigned long *)&file_size);
#else
	jpeg2_data = jpeg_data;

#endif

	if (!jpeg2_data) {
		printf("ERROR(%s) : resize_jpeg_file error\n", __func__);
		return -1;
	}

	int nv12_w = 0, nv12_h = 0;
	unsigned char *nv12_data = NULL;
	nv12_data = convert_jpeg_to_nv12(jpeg2_data, file_size, &nv12_w, &nv12_h);
	if (!nv12_data) {
		printf("ERROR(%s): convert nv12  error\n", __func__);
		return -1;
	}

#if 1
	/* write_jpeg_file(argv[2], rgb2_data, 100, 3, 640, 360); */
	FILE *fp = NULL;
	fp = fopen(argv[2], "w+");
	fwrite(nv12_data,  nv12_w * nv12_h * 3 / 2, 1, fp);
	/* fwrite(jpeg2_data,  file_size, 1, fp); */
	fclose(fp);
#endif

	free(nv12_data);
	free(jpeg_data);

	return 0;
}
