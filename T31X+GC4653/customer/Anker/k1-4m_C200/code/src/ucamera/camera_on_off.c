/*********************************************************
 * File Name   : camera_on_off.c
 * Author      : Jasmine
 * Mail        : jian.dong@ingenic.com
 * Created Time: 2021-10-08 19:51
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>

#include <camera_on_off.h>
#include <jpeg_control.h>

#include <imp/imp_log.h>
#include <imp/imp_framesource.h>
#include <imp/imp_common.h>
#include <imp/imp_isp.h>
#include <imp-common.h>

#define TAG "Sample-UCamera"
typedef struct image_info {
	int fmt;
	int width;
	int height;
	int size;
	void *data;
} image_info_t;

typedef struct _camera_off_info {
	int enable;                 /* 使能 开始保存图像*/
	int fd;                     /* 按键控制句柄 */
	int fmt;                    /* 当前视频流格式*/
	unsigned int width;
	unsigned int height;               /* 720/1080/1440*/
	image_info_t src_image;     /* 原始文件大小 */
	image_info_t dst_image;     /* resize 后大小 */
} camera_off_info_t;

extern struct chn_conf chn[];
static camera_off_info_t g_camera_off_info;
extern IMPRgnHandle *prHander;

/*
 *g_osdState:
 *        0:osd处于不显示状态
 *        1:osd处于显示状态
 */
static int g_osdState = 0;


/*
 *g_imageDateState :
 *        0:NV12数据还没准备好
 *        1:nv12数据已经准备好了
 */
static int g_imageDateState = 0;

int sample_enable_camera_off_pic(int fmt)
{
	/*TODO: fmt for h264 */
	g_camera_off_info.enable = 1;
	g_camera_off_info.fmt = fmt;

	return 0;
}

void sample_disable_camera_off_pic(int fmt)
{
	/*TODO: fmt for h264 */
	g_camera_off_info.enable = 0;
	g_camera_off_info.fmt = fmt;
}
void sample_update_resution(unsigned int width,unsigned int height)
{
	g_camera_off_info.width = width;
	g_camera_off_info.height = height;
}
static void camera_off_pic_read_file_data(char *file_name)
{
	/*Step1: get file size */
	int file_size = 0;
	struct stat statbuf;
	stat(file_name,&statbuf);
	file_size = statbuf.st_size;

	int rsize = 0;
	FILE *fp = NULL;

	fp = fopen(file_name, "r+");
	if (!fp) {
		printf("ERROR(%s): open %s failed \n", __func__, file_name);
		return;
	}

	memset(g_camera_off_info.src_image.data, 0 , MAX_JPEG_SIZE);
	rsize = fread(g_camera_off_info.src_image.data, 1, file_size, fp);
	if(rsize != file_size) {
		printf("ERROR(%s): read %d btye, actual %d byte \n", __func__, rsize, file_size);
		return;
		fclose(fp);
	}
	
	g_camera_off_info.src_image.size = file_size;
	fclose(fp);
}

static void *camera_off_pic_prepare_process(void *arg)
{
	int stream_on = 0;
	int resize_flag = 0;
	int save_end = 0;
	int ret;
	prctl(PR_SET_NAME, "camera_off_pic_prepare_process");

	while (1) {
		stream_on = sample_get_ucamera_streamonoff();

		/* 默认挂起 */
		while(!stream_on) {
			/*if ucamera_stream off, release buffer */
			if (g_camera_off_info.fmt == V4L2_PIX_FMT_MJPEG) {
				/*resize 后, jpeg data 需要释放 */
				if (resize_flag) {
					if (g_camera_off_info.dst_image.data) {
						free(g_camera_off_info.dst_image.data);
						g_camera_off_info.dst_image.data = NULL;
					}
				}
			} else if (g_camera_off_info.fmt == V4L2_PIX_FMT_NV12 || g_camera_off_info.fmt == V4L2_PIX_FMT_YUYV || g_camera_off_info.fmt == V4L2_PIX_FMT_H264) {
				if (save_end) {
					/*yuv 转换过程中有申请外部buffer, 需要直接释放*/
					if (g_camera_off_info.dst_image.data) {
						free(g_camera_off_info.dst_image.data);
						g_camera_off_info.dst_image.data = NULL;
					}
				}
			}
			stream_on = sample_get_ucamera_streamonoff();
			resize_flag = 0;
			save_end = 0;
			g_imageDateState = 0;
			g_osdState = 0;
			usleep(1 * 1000);
		}

		if (!save_end) {

			/*MJPEG and YUV */
			char file_name[64] = {0};

			/*Step1: read jpeg data */
			unsigned char *src_data = NULL;
			unsigned char *jpeg_data = NULL;
			unsigned char *nv12_data = NULL;
			int file_size = 0;

			if (g_camera_off_info.width < 1280 && g_camera_off_info.height < 720) {
				/*need resize */
				strcpy(file_name, CAMERA_OFF_PIC_720P);
				camera_off_pic_read_file_data(file_name);
				file_size = g_camera_off_info.src_image.size;
				src_data = g_camera_off_info.src_image.data;
				if (!file_size || !src_data) {
					printf("WARNNING(%s): need to wait g_camera_off_info read data \n", __func__);
					continue;
				}

				resize_flag = 1;
			} else {

				if (g_camera_off_info.width == 1280 && g_camera_off_info.height == 720) {
					strcpy(file_name, CAMERA_OFF_PIC_720P);
				} else if ( g_camera_off_info.width == 1920 && g_camera_off_info.height == 1080) {
					strcpy(file_name, CAMERA_OFF_PIC_1080P);
				} else {
					strcpy(file_name, CAMERA_OFF_PIC_2k);
				}

				camera_off_pic_read_file_data(file_name);

				file_size = g_camera_off_info.src_image.size;
				src_data = g_camera_off_info.src_image.data;
				if (!file_size || !src_data) {
					printf("WARNNING(%s): need to wait g_camera_off_info read data \n", __func__);
					continue;
				}
				resize_flag = 0;

			}

			/*Step2: resize jpeg data */
			if (resize_flag) {
				jpeg_data = resize_jpeg_file(src_data, file_size, g_camera_off_info.width, g_camera_off_info.height, 24, (unsigned long*)&file_size);
				if (!jpeg_data) {
					printf("WARNNING(%s): resize jpeg_file \n", __func__);
					continue;
				}
				g_camera_off_info.src_image.size = file_size;
			} else {
				jpeg_data = g_camera_off_info.src_image.data;
				file_size = g_camera_off_info.src_image.size;
			}

			/*Step3:MJPEG save data  */
			if (g_camera_off_info.fmt == V4L2_PIX_FMT_MJPEG) {
				if (resize_flag) {
					g_camera_off_info.dst_image.data = jpeg_data;
					g_camera_off_info.dst_image.size = g_camera_off_info.src_image.size;
				} else {
					g_camera_off_info.dst_image.data = g_camera_off_info.src_image.data;
					g_camera_off_info.dst_image.size = g_camera_off_info.src_image.size;
				}
				save_end = 1;
			}

			/*YUV save data  */
			if (g_camera_off_info.fmt == V4L2_PIX_FMT_YUYV || g_camera_off_info.fmt == V4L2_PIX_FMT_NV12 || g_camera_off_info.fmt == V4L2_PIX_FMT_H264) {
				nv12_data = convert_jpeg_to_nv12(jpeg_data, file_size, &g_camera_off_info.width, &g_camera_off_info.height);
				if (!nv12_data) {
					if (resize_flag) {
						free(jpeg_data);
					}
					printf("ERROR(%s): convert nv12  error\n", __func__);
					continue;
				}

				if (resize_flag) {
					free(jpeg_data);
					jpeg_data = NULL;
				}

				g_camera_off_info.dst_image.data = nv12_data;
				g_camera_off_info.dst_image.size = g_camera_off_info.width * g_camera_off_info.height * 3 / 2;

				save_end = 1;

				if (g_camera_off_info.fmt == V4L2_PIX_FMT_H264) {
					ret = sample_OSD_SetRgnAttr(g_camera_off_info.width,g_camera_off_info.height,prHander[0],nv12_data);
					if (ret < 0) {
						IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Logo error !\n");
						continue;
					}
				}
			}
			g_imageDateState = 1;
			printf("INFO(%s): prepare camera_off_pic data ok\n", __func__);
		} else {
			if (g_camera_off_info.enable) {
				sample_ucamera_led_ctl(g_led, 0);

			} else {
				sample_ucamera_led_ctl(g_led, 1);
			}
			usleep(100 * 1000);
		}

	}

	return NULL;
}

static void* camera_off_pic_key_process(void *arg)
{
	int fd = -1;
	int val = 0;

	prctl(PR_SET_NAME, "camera_off_pic_key_process");

	fd = open(KEY_DEV, O_RDWR);
	if (fd == -1)
	{
		printf("can not open file %s\n", KEY_DEV);
		return NULL;
	}

	g_camera_off_info.fd = fd;

	while (1) {
		read(fd, &val, 4);
		printf("get button : 0x%x\n", val);
		if (val == 0) {
			sample_enable_camera_off_pic(g_camera_off_info.fmt);
		} else {
			sample_disable_camera_off_pic(g_camera_off_info.fmt);
		}
	}

	close(fd);

	return NULL;
}
int sample_camera_off_pic_key_process(void)
{
	int ret = -1;
	pthread_t pid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
	ret = pthread_create(&pid, &attr, &camera_off_pic_key_process, NULL);
	if (ret) {
		printf("ERROR(%s): camera_off_pic_key_process!\n", __func__);
		return -1;
	}

	return 0;
}

unsigned long sample_camera_off_pic_get_key_val()
{
	int ret = -1;
	int fd = -1;
	unsigned long val = 0;

	fd = g_camera_off_info.fd;
	ret = ioctl(fd, CMD_GET_VAL, (unsigned long *)&val);
	if (ret < 0) {
		printf("ERROR(%s): get key val failed !\n", __func__);
		return -1;
	}

	printf(" key val = %ld \n", val);

	return val;
}

int sample_get_jpeg_snap_2(char *img_buf)
{
	int len = 0;
	if (g_camera_off_info.enable) {
		while (g_imageDateState == 0) {
			usleep(1 * 1000);
		}
		len = g_camera_off_info.dst_image.size;
		memcpy(img_buf, g_camera_off_info.dst_image.data, len);
		return len;
	} else {
		return sample_get_jpeg_snap(img_buf);
	}
}

int sample_get_yuv_snap_2(char *img_buf)
{
	int len = 0;

	if (g_camera_off_info.enable) {
		while (g_imageDateState == 0) {
			usleep(1 * 1000);
		}
		unsigned char *nv12_data = g_camera_off_info.dst_image.data;

		if (nv12_data) {
			len = g_camera_off_info.width *g_camera_off_info.height *2;
			nv12toyuy2((char *)nv12_data, img_buf, g_camera_off_info.width, g_camera_off_info.height);
		}

		return len;
	} else {
		return sample_get_yuv_snap(img_buf);
	}
}

int sample_get_h264_snap_2(char *img_buf)
{
	int ret;
	if (g_camera_off_info.enable) {
		while (g_imageDateState == 0) {
			usleep(1 * 1000);
		}
		if (g_osdState != 1) {
			ret = IMP_OSD_ShowRgn(prHander[0], 0, 1);
			if (ret != 0) {
				IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn() timeStamp error\n");
				return -1;
			}
			g_osdState = 1;
		}
	} else {
		if (g_osdState != 0) {
			ret = IMP_OSD_ShowRgn(prHander[0], 0, 0);
			if (ret != 0) {
				IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn() timeStamp error\n");
				return -1;
			}
			g_osdState = 0;
		}
	}
	return sample_get_h264_snap(img_buf);
}

void sample_camera_off_pic_init()
{
	/*Step1: malloc src_image data*/
	memset(&g_camera_off_info, 0, sizeof(camera_off_info_t));
	g_camera_off_info.src_image.data  = NULL;
	g_camera_off_info.dst_image.data  = NULL;

	g_camera_off_info.src_image.data = (unsigned char *)malloc(MAX_JPEG_SIZE);
	if (!g_camera_off_info.src_image.data) {
		printf("ERROR(%s): malloc data error!\n", __func__);
		return;
	}
	memset(g_camera_off_info.src_image.data, 0, MAX_JPEG_SIZE);

	/*Step2: created thread*/
	int ret;
	pthread_t pid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
	ret = pthread_create(&pid, &attr, &camera_off_pic_prepare_process, NULL);
	if (ret) {
		printf("ERROR(%s): camera_off_pic_prepare_process!\n", __func__);
		return;
	}

	return;
}

void sample_camera_off_pic_deinit()
{
	if (g_camera_off_info.src_image.data) {
		free(g_camera_off_info.src_image.data);
		g_camera_off_info.src_image.data = NULL;
	}
}
