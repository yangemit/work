/*
 * sample-common.h
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __SAMPLE_COMMON_H__
#define __SAMPLE_COMMON_H__

#include <imp/imp_common.h>
#include <imp/imp_osd.h>
#include <imp/imp_framesource.h>
#include <imp/imp_isp.h>
#include <imp/imp_encoder.h>
#include <unistd.h>
#include "hik_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

extern int g_Audio;
extern int g_Audio_Ns;
extern int g_Fps_Num;
extern int g_VideoWidth;
extern int g_VideoHeight;
extern int g_i2c_addr;
extern int g_wdr;
extern int g_RcMode;
extern int g_BitRate;
extern int g_gop;
extern int g_adb;
extern int g_rndis;
extern int g_Speak;
extern int g_dmic;
extern int g_led;
extern int g_touch;
extern int g_touch_old;
extern int g_HV_Flip;
extern int g_QP;
extern int g_Volume;
extern int g_Dynamic_Fps;
extern int g_Power_save;
extern char g_Sensor_Name[16];

#define SENSOR_FRAME_RATE_NUM_25	25
#define SENSOR_FRAME_RATE_NUM_30	30
#define SENSOR_FRAME_RATE_DEN		1

//#define SENSOR_OS05A10
//#define SENSOR_IMX335
//#define SENSOR_GC2053
//#define SENSOR_GC4C33
//#define SENSOR_IMX327
//#define SENSOR_IMX307
#define CHN0_EN                 1
#define CROP_EN					1

#if defined SENSOR_OS05A10
#define SENSOR_NAME				"os05a10"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x36
#define SENSOR_WIDTH			2592
#define SENSOR_HEIGHT			1944
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					0
#elif defined SENSOR_IMX335
#define SENSOR_NAME				"imx335"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x1a
#define SENSOR_WIDTH			2592
#define SENSOR_HEIGHT			1944
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					0
#elif defined SENSOR_GC2053
#define SENSOR_NAME				"gc2053"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x37
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 1
#define CROP_EN					0
#elif defined SENSOR_GC4C33
#define SENSOR_NAME				"gc4c33"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x10
#define SENSOR_WIDTH			2560
#define SENSOR_HEIGHT			1440
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN					0
#elif defined SENSOR_IMX307
#define SENSOR_NAME				"imx307"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x1a
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN					0
#elif defined SENSOR_IMX327
#define SENSOR_NAME				"imx327"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x36
#define SENSOR_WIDTH			1920
#define SENSOR_HEIGHT			1080
#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0
#define CHN3_EN                 0
#define CROP_EN					0
#endif

#define SENSOR_WIDTH_SECOND		640
#define SENSOR_HEIGHT_SECOND	360

#define SENSOR_WIDTH_THIRD		1280
#define SENSOR_HEIGHT_THIRD		720

#define BITRATE_720P_Kbs        (1000 * 5)

#define NR_FRAMES_TO_SAVE		200
#define STREAM_BUFFER_SIZE		(1 * 1024 * 1024)

#define ENC_VIDEO_CHANNEL		0
#define ENC_JPEG_CHANNEL		1

#define STREAM_FILE_PATH_PREFIX		"/tmp"
#define SNAP_FILE_PATH_PREFIX		"/tmp"

#define OSD_REGION_WIDTH		16
#define OSD_REGION_HEIGHT		34
#define OSD_REGION_WIDTH_SEC		8
#define OSD_REGION_HEIGHT_SEC   	18


#define SLEEP_TIME			1

#define FS_CHN_NUM			1  //MIN 1,MAX 3
#define IVS_CHN_ID          2

#define CH0_INDEX  0
#define CH1_INDEX  1
#define CH2_INDEX  2
#define CH3_INDEX  3
#define CHN_ENABLE 1
#define CHN_DISABLE 0

#define HIK_USB_OS02D20 1
#define HIK_USB_OS04C10 2
#define HIK_USB_JXF37 	3

#if((CAMERA_MODEL == TV22_2K809) || (CAMERA_MODEL == U22_2K815) || (CAMERA_MODEL == UC2_2K819) || (CAMERA_MODEL == E12A_2K822))
#define HIK_USB_PRODUCT HIK_USB_OS02D20
#elif((CAMERA_MODEL == TV24_2K810) || (CAMERA_MODEL == U24_2K816) || (CAMERA_MODEL == UC4_2K820) || (CAMERA_MODEL == E14A_2K823))
#define HIK_USB_PRODUCT HIK_USB_OS04C10
#else
#define HIK_USB_PRODUCT HIK_USB_OS04C10
#endif

/*#define SUPPORT_RGB555LE*/

struct chn_conf{
	unsigned int index;//0 for main channel ,1 for second channel
	unsigned int enable;
  	IMPEncoderProfile payloadType;
	IMPFSChnAttr fs_chn_attr;
	IMPCell framesource_chn;
	IMPCell imp_encoder;
};

#define  CHN_NUM  ARRAY_SIZE(chn)

#define LED_GPIO 53

#define UVC_BUF_NUM 	2

// 分配太大之后，创建线程会失败
#if((CAMERA_MODEL == TV22_2K809) || (CAMERA_MODEL == U22_2K815) || (CAMERA_MODEL == UC2_2K819) || (CAMERA_MODEL == E12A_2K822))
#define UVC_BUF_SIZE	(unsigned int)(2*1024*1024)
#elif((CAMERA_MODEL == TV24_2K810) || (CAMERA_MODEL == U24_2K816))
#define UVC_BUF_SIZE	(unsigned int)(4*1024*1024)
#elif((CAMERA_MODEL == UC4_2K820) || (CAMERA_MODEL == E14A_2K823))
#define UVC_BUF_SIZE	(unsigned int)(2*1024*1024)
#else
#define UVC_BUF_SIZE	(unsigned int)(4*1024*1024)
#endif

int sample_system_init();
int sample_system_exit();

int sample_framesource_streamon();
int sample_framesource_streamoff();

int sample_framesource_init();
int sample_framesource_exit();

int sample_encoder_init();
int sample_jpeg_init();
int sample_jpeg_exit(void);
int sample_encoder_exit(void);


int sample_get_frame();
int sample_get_jpeg_snap(char *img_buf);
int sample_get_h264_snap(char *img_buf);
int sample_get_yuv_snap(char *img_buf);
int sample_get_nv12_snap(char *img_buf);

void sample_audio_dmic_init(void);
void sample_audio_dmic_exit(void);
int sample_audio_dmic_pcm_get(short *pcm);

void sample_audio_amic_init(void);
void sample_audio_amic_exit(void);
int sample_audio_amic_pcm_get(short *pcm);

int sample_get_h264_start(void);
int sample_get_h264_stop(void);

int imp_sdk_init(int format, int width, int height);
int imp_sdk_deinit(int format);
int imp_system_init(void);
int imp_system_exit(void);
int sample_reload_sensor_init(char *sensor_name_del, char *sensor_name_add);

int sample_set_mic_volume(int vol);
int sample_set_spk_volume(int vol);
int sample_set_spk_mute(int mute);
int sample_set_mic_mute(int mute);
int sample_audio_play_start(void);
int sample_video_isOnStream(void);

int sample_ucamera_gpio_init(int gpio, int direction);
int sample_ucamera_gpio_ctl(int gpio, int value);
int sample_ucamera_gpio_read(int gpio);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SAMPLE_COMMON_H__ */
