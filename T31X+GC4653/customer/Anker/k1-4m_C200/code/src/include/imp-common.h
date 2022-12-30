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
#include <imp/imp_audio.h>
#include <imp/imp_isp.h>
#include <imp/imp_encoder.h>
#include <unistd.h>

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
extern int g_VideoWidth_bk;
extern int g_VideoHeight_bk;
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
extern int g_led_b;
extern int g_led_g;
extern int g_led_r;
extern int g_HV_Flip;
extern int g_QP;
extern int Mic_Volume;
extern int Spk_Volume;
extern int g_Dynamic_Fps;
extern int g_Power_save;
extern char g_Sensor_Name[16];
extern IMPAudioIOAttr amic_attr;

#define SENSOR_FRAME_RATE_NUM_25	25
#define SENSOR_FRAME_RATE_NUM_30	30
#define SENSOR_FRAME_RATE_DEN		1
#define SENSOR_RELOAD                   0

#define CHN0_EN                         1
#define CROP_EN                         1

#define SENSOR_WIDTH		        2560
#define SENSOR_HEIGHT        		1440

#define BITRATE_720P_Kbs                (1000 * 5)

#define FS_CHN_NUM			1  //MIN 1,MAX 3
#define IVS_CHN_ID                      2

#define CH0_INDEX  0
#define CH1_INDEX  1
#define CH2_INDEX  2
#define CH3_INDEX  3
#define CHN_ENABLE 1
#define CHN_DISABLE 0

struct chn_conf{
	unsigned int index;//0 for main channel ,1 for second channel
	unsigned int enable;
#ifdef T31
  	IMPEncoderProfile payloadType;
#else
	IMPPayloadType payloadType;
#endif
	IMPFSChnAttr fs_chn_attr;
	IMPCell framesource_chn;
	IMPCell imp_encoder;
};

typedef struct _crop_obj {
	int left;
	int top;
	int w;
	int h;
} crop_obj_t;

#define PANTILT_MODE                  1
#define FOV_MODE                      0

#define  CHN_NUM  ARRAY_SIZE(chn)

#define UVC_BUF_NUM                   2
#define UVC_BUF_SIZE                  1.5*1024*1024
#define UVC_ATTR_PATH                 "/media/uvc.attr"
#define UVC_CONFIG_PATH                "/media/uvc.config"
#define UVC_ATTR_BAK_PATH              "/system/config/uvc.attr"
#define UVC_CONFIG_BAK_PATH            "/system/config/uvc.config"
#define KIVA_APP_VERSION               "V5.3"

#define SENSOR_DEFAULT_FOV             95
#define SENSOR_FOV_65                  65
#define SENSOR_FOV_78                  78
#define SENSOR_FOV_95                  95

#define AUDIO_ANGLE_0                  0
#define AUDIO_ANGLE_60                 60
#define AUDIO_ANGLE_90                 90
#define AUDIO_ANGLE_180                180

#define UCAMERA_DEBUG                  0

#define UVC_PAN_ATTR_CONTROL           0x1
#define UVC_TILT_ATTR_CONTROL          0x2
#define EU_FOV_ATTR_CONTROL            0x3
#define EU_AUDIO_ANGLE_ATTR_CONTROL    0x4
#define EU_PANTILT_MODE_CONTROL        0x5
#define EU_HDR_MODE_CONTROL	       0x6

#if UCAMERA_DEBUG
#define Ucamera_DEBUG(format, arg...)                                            \
	printf("%s: " format "\n" , "[Ucamera]", ## arg)
#else
#define Ucamera_DEBUG(format, arg...)
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
int ucamera_uvc_pu_attr_save(int type, int value);
int ucamera_uvc_ct_attr_save(int type, int value);

#ifdef T31
void sample_audio_dmic_init(void);
void sample_audio_dmic_exit(void);
int sample_audio_dmic_pcm_get(short *pcm);
#endif

void sample_audio_amic_init(void);
void sample_audio_amic_exit(void);
int sample_audio_amic_pcm_get(short *pcm);

int sample_get_h264_start(void);
int sample_get_h264_stop(void);

int imp_sdk_init(int format, int width, int height);
int imp_sdk_deinit(int format);
int imp_system_init(void);
int imp_system_exit(void);

int sample_set_mic_volume(int vol);
int sample_set_spk_volume(int vol);
int sample_set_spk_mute(int mute);
int sample_set_mic_mute(int mute);
int sample_get_record_off();
int sample_get_record_on();
int sample_get_speak_off();
int sample_get_speak_on();
int sample_audio_play_start(void);
int sample_video_zoom_set(int value);

int sample_ucamera_led_init(int gpio, int value);
int sample_ucamera_led_ctl(int gpio, int value);
int sample_ucamera_gpio_in_init(int gpio);
int sample_ucamera_gpio_read(int gpio);

int sample_video_zoom_move_set(int zoom_value, int move_value);
#if SENSOR_RELOAD
int sensor_cutover(int format, int height, int interval);
#endif
int sensor_tuning(int cmd, int height);
int speak_volume_write_config(int val);
void nv12toyuy2(char * image_in, char* image_out, int width, int height);
int sample_get_ucamera_streamonoff();
int sample_get_ucamera_af_onoff();
int sample_get_ucamera_af_cur();
int sample_get_ucamera_ev_cur();

IMPRgnHandle *sample_osd_init(int grpNum);
int sample_osd_exit(IMPRgnHandle *prHander,int grpNum);
int sample_OSD_SetRgnAttr(int w, int h, IMPRgnHandle rHanderLogo, unsigned char * nv12_data);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SAMPLE_COMMON_H__ */
