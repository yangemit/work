/*
 * sample-common.h
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __SAMPLE_COMMON_H__
#define __SAMPLE_COMMON_H__

#include <imp/imp_log.h>
#include <imp/imp_system.h>
#include <imp/imp_common.h>
#include <imp/imp_osd.h>
#include <imp/imp_framesource.h>
#include <imp/imp_isp.h>
#include <imp/imp_encoder.h>


#include <imp/imp_audio.h>
#include <imp/imp_dmic.h>

#include <unistd.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

extern int g_Power_save;

#define UVC_VIDEO_CH			0
#define ALGORITHM_VIDEO_CH		1

#define OSD_HANDLE_NUM    2 

#define SENSOR_FRAME_RATE_NUM_25	25
#define SENSOR_FRAME_RATE_NUM_30	30
#define SENSOR_FRAME_RATE_DEN		1
/* #define SENSOR_SC4335 */

//#define SENSOR_OS05A10
//#define SENSOR_IMX335
//#define SENSOR_GC2053
//#define SENSOR_GC4C33
//#define SENSOR_IMX327
//#define SENSOR_IMX307
#define CHN0_EN                 1
#define CHN1_EN                 1
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
#elif defined SENSOR_SC4335
#define SENSOR_NAME				"sc4335"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define SENSOR_I2C_ADDR			0x30
#define SENSOR_WIDTH			2560
#define SENSOR_HEIGHT			1440
#define CHN0_EN                 1
#define CHN1_EN                 1
#define CHN2_EN                 0
#define CHN3_EN                 0
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

#if (defined MODULE_FACEAE_ENABLE)
#define SENSOR_WIDTH_SECOND		320
#define SENSOR_HEIGHT_SECOND	180
#elif (defined MODULE_TRACK_ENABLE)
#define SENSOR_WIDTH_SECOND		640
#define SENSOR_HEIGHT_SECOND	360
#else
#define SENSOR_WIDTH_SECOND		640
#define SENSOR_HEIGHT_SECOND	360
#endif



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

#define FS_CHN_NUM			2  //MIN 1,MAX 3
#define IVS_CHN_ID          1

#define IVS_MODE_BIND			1
#define IVS_MODE_UNBIND			2

#define CH0_INDEX  0
#define CH1_INDEX  1
#define CH2_INDEX  2
#define CH3_INDEX  3
#define CHN_ENABLE 1
#define CHN_DISABLE 0

#define UCAMERA_DEBUG                    0

#if UCAMERA_DEBUG
#define Ucamera_DEBUG(format, arg...)                                            \
		printf("%s: " format "\n" , "[Ucamera]", ## arg)
#else
#define Ucamera_DEBUG(format, arg...)
#endif

struct chn_conf{
	unsigned int index;//0 for main channel ,1 for second channel
	unsigned int enable;
  	IMPEncoderProfile payloadType;
	IMPFSChnAttr fs_chn_attr;
	IMPCell framesource_chn;
	IMPCell imp_encoder;
	IMPCell imp_osd;
};

#define  CHN_NUM  ARRAY_SIZE(chn)

#define LED_GPIO 53

#define MJPEG_ENCODER_SIZE      2 * 1024 * 1024 //size 2M

#define OSD_CONTROL_GROUP       0
/* #define IVS_TRACK_GROUP         0 */
#define MAX_IVS_OSD_REGION	5
int sample_system_init(void *param);
int sample_system_exit();

int sample_framesource_streamon(int chn_num);
int sample_framesource_streamoff(int chn_num);

int sample_framesource_init(int chn_num);
int sample_framesource_exit(int chn_num);

int sample_encoder_init(int chn_num, int enc_type);
int sample_jpeg_init(int chn_num);
int sample_jpeg_exit(int chn_num);
int sample_encoder_exit(int chn_num);

int osd_init(int grpNum, int ch);
int osd_deinit(int grpNum);

int module_uvc_control_get_status(void);
int algorithm_video_init(int chn_num, int ivs_mode);
int algorithm_video_deinit(int chn_num, int ivs_mode);
int algo_isp_tuning_init(const void *param);

int sample_get_frame();
int sample_get_jpeg_snap(int chn_num, char *img_buf);
int sample_get_h264_snap(int chn_num, char *img_buf);
int sample_get_yuv_snap(int chn_num, char *img_buf);

void sample_audio_dmic_init(void);
void sample_audio_dmic_exit(void);

void sample_audio_amic_init(int vol, int audio_ns);
void sample_audio_amic_exit(void);

int sample_get_h264_start(void);
int sample_get_h264_stop(void);

int imp_system_exit(void);

int sample_audio_play_start(void *param);
int speak_volume_write_config(int val);

int sample_get_ucamera_streamonoff();
int sample_get_ucamera_af_onoff();
int sample_get_ucamera_af_cur();

int sample_uvc_framesource_init(int ch, int fps, int format, int width, int height);

//osd
IMPRgnHandle *sample_osd_init(int grpNum);
int sample_osd_exit(IMPRgnHandle *prHander,int grpNum);
int osd_show(IMPRgnHandle *prHander,int grpNum);

//osd-ivs
int module_osd_control_init(void);
int module_osd_control_deinit(void);
int IVS_OSD_Init(int grp_num);
int IVS_OSD_Start(void);
int IVS_OSD_Stop(void);
int set_osd_ivs_switch(int on_off);

//yuyv
void set_yuyv_switch(int on_off);

//png-argb
//int getFileSizeSystemCall(const char *strFileName);
int load_png_image(const char *filepath);
int resize_png(char const *str_dir,int nDestWidth, int nDestHeight);

void Stretch_pic(int srcWidth, int srcHeight, int picWidth, int picHeight, int *need_Width, int *need_Height);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SAMPLE_COMMON_H__ */
