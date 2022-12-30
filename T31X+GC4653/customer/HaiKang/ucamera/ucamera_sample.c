/*
 * sample-Encoder-jpeg.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <dirent.h>
#include <sys/prctl.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_audio.h>
#include "imp-common.h"
#define TAG "Sample-UCamera"
#include "ucam/usbcamera.h"
#include "anticopy/AntiCopy_Verify.h"

#include "jx/Af_src.h"
#include "vcm_controler.h"
#include "tof_controler.h"
#include "hikisp/isp_inner.h"
#include "hik_config.h"
#include "pwm_controler.h"

extern struct chn_conf chn[];
static IMPISPFrontCrop fcrop_obj;
int gStreaming = 0;
int gFps = 0;
int gWidth = 0;
int gHeight = 0;

pthread_t pt_jx;
pthread_t pt_vcm;
pthread_t pt_tof;

// 是否正在聚焦的标志
static int gJXRuning = 0;
// VCM初始位置的电流
static int gVCMInitPot = 450;
// 私有属性当前值
static int gFastShutter =4;			// 快门等级，现在不再使用
static int gDenoise = 0;			// 图像降噪等级
static int gImageMode = 0;			// 图像模式
static int gHVFlip = 0;				// 镜像
static int gPN = 0;					// PN制
static int gAFGarde = 0;			// 聚焦灵敏度

struct vcm_typedef st_jx = {
	.garde = 10,
};	// 传给机芯线程的结构体
static int tof_init = 0;			// TOF是否初始化的变量
struct tof_tag st_tof = {0};		// 传给TOF线程的参数

//2020.08.24 ADD by LHY
ISP_INNER_CTRL *gp_isp_inner_ctrl;
HIK_AAA_PARAM g_aaa_param;
extern struct video_isp_class g_isp_object;

// uvc.attr相关的变量
FILE *uvc_attr_fd = NULL;
pthread_mutex_t uvc_attr_mutex;
sem_t ucam_ready_sem;
static int imp_inited = 0;

// 相机属性
static char manufact_label[LABEL_LEN] = "HIK USB Camera";
static char product_label[LABEL_LEN] = "HIK USB Camera";
static char serial_label[LABEL_LEN] = "SN0002";
static char video_device_name[LABEL_LEN] = "HIK USB Camera";
static char audio_device_name[LABEL_LEN] = "HIK USB Camera-Audio";
static int  VENDOR_ID = 0x2BDF;
static int  PRODUCT_ID = 0x0200;
static int  DEVICE_BCD = 0x6000;
// 私有属性变量
int iSelfParamFlag0 = 0;
int iSelfParamFlag1 = 0;
int iSelfParamFlag2 = 0;
int iSelfParamTemp = 0;
// 是否需要保存参数
static int iNeedSave = 1;

static struct Ucamera_Video_CT_Control auto_exposure_mode = {
	.type = UVC_AUTO_EXPOSURE_MODE_CONTROL,
	.data[UVC_MIN] = 1,
	.data[UVC_MAX] = 4,
	.data[UVC_DEF] = 2,
	.data[UVC_CUR] = 2,
};

static struct Ucamera_Video_CT_Control exposure_time = {
	.type = UVC_EXPOSURE_TIME_CONTROL,
	.data[UVC_MIN] = 5,//-11
	.data[UVC_MAX] = 2500,//-2
	.data[UVC_DEF] = 2500,//-2
	.data[UVC_CUR] = 2500,//-2
};

static struct Ucamera_Video_CT_Control focus = {
	.type = UVC_FOCUS_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_CT_Control focus_auto = {
    .type = UVC_FOCUS_AUTO_CONTROL,
    .data[UVC_MAX] = 1,
    .data[UVC_MIN] = 0,
    .data[UVC_DEF] = 1,
    .data[UVC_CUR] = 1,
};

static struct Ucamera_Video_CT_Control zoom = {
	.type = UVC_ZOOM_ABSOLUTE_CONTROL,
	.data[UVC_MAX] = 200,
	.data[UVC_MIN] = 100,
	.data[UVC_DEF] = 100,
	.data[UVC_CUR] = 100,
};

static struct Ucamera_Video_CT_Control roll = {
	.type = UVC_ROLL_ABSOLUTE_CONTROL,
	.data[UVC_MAX] = 255,
	.data[UVC_MIN] = 0,
	.data[UVC_DEF] = 0,
	.data[UVC_CUR] = 0,
};

static struct Ucamera_Video_CT_Control *Ct_ctrl[] = {
	&exposure_time,
	&auto_exposure_mode,
	&focus,
	&focus_auto,
	&zoom,
	&roll,
	NULL,
};

static struct Ucamera_Video_PU_Control backlight_compens = {
	.type = UVC_BACKLIGHT_COMPENSATION_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 8,
	.data[UVC_DEF] = 4,
	.data[UVC_CUR] = 4,
};

static struct Ucamera_Video_PU_Control brightness = {
	.type = UVC_BRIGHTNESS_CONTROL,
	.data[UVC_MIN] = 1,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_PU_Control contrast = {
	.type = UVC_CONTRAST_CONTROL,
	.data[UVC_MIN] = 1,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_PU_Control saturation = {
	.type = UVC_SATURATION_CONTROL,
	.data[UVC_MIN] = 1,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_PU_Control sharpness = {
	.type = UVC_SHARPNESS_CONTROL,
	.data[UVC_MIN] = 1,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_PU_Control hue = {
	.type = UVC_HUE_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_PU_Control pu_gamma = {
	.type = UVC_GAMMA_CONTROL,
	.data[UVC_MIN] = 1,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_PU_Control gain = {
	.type = UVC_GAIN_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 255,
	.data[UVC_CUR] = 255,
};

static struct Ucamera_Video_PU_Control whitebalance = {
	.type = UVC_WHITE_BALANCE_TEMPERATURE_CONTROL,
	.data[UVC_MIN] = 300,
	.data[UVC_MAX] = 600,
	.data[UVC_DEF] = 417,
	.data[UVC_CUR] = 417,
};

static struct Ucamera_Video_PU_Control whitebalance_auto = {
	.type = UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 1,
	.data[UVC_DEF] = 0,
	.data[UVC_CUR] = 1,
};

static struct Ucamera_Video_PU_Control powerlinefreq = {
	.type = UVC_POWER_LINE_FREQUENCY_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 2,
	.data[UVC_DEF] = 1,
};

static struct Ucamera_Video_PU_Control *Pu_ctrl[] = {
	&backlight_compens,
	&brightness,
	&contrast,
	&hue,
	&saturation,
	&sharpness,
	&whitebalance,
	&whitebalance_auto,
	&powerlinefreq,
	&gain,
	&pu_gamma,
	NULL,
};

int hik_set_image_mode(int mode)
{
	int ret = 0;
	mode = CLIP(mode, 0, 3);
	gImageMode = mode;
	switch (mode)
	{
	case 0: //标准
		sample_video_brightness_set(128);
		sample_video_contrast_set(128);
		sample_video_sharpness_set(128);
		sample_video_saturation_set(128);
		break;
	case 1: //鲜艳
		sample_video_brightness_set(128);
		sample_video_contrast_set(128);
		sample_video_sharpness_set(128);
		sample_video_saturation_set(160);
		break;
	case 2: //柔和
		sample_video_brightness_set(150);
		sample_video_contrast_set(80);
		sample_video_sharpness_set(60);
		sample_video_saturation_set(128);
		break;
	case 3: //背光
		sample_video_brightness_set(200);
		sample_video_contrast_set(100);
		sample_video_sharpness_set(128);
		sample_video_saturation_set(128);
		break;
	default:
		break;
	}
	return ret;
}

int hik_set_video_standard(int standard)
{
	int ret = 0;
	int fps_num = 0;
	uint32_t fps_num_bak, fps_den_bak;
	if(0 == standard)
	{
		// 切换到P制
		if (g_Fps_Num <= 30)
			fps_num = 25;
		else if (g_Fps_Num == 60) {
			fps_num = 50;
		} else {
			fps_num = g_Fps_Num;
		}
		ret = IMP_ISP_Tuning_GetSensorFPS(&fps_num_bak, &fps_den_bak);
		if(ret < 0)
		{
			Ucamera_ERR("isp get sensor fps error");
		}
		if(fps_num != fps_num_bak / fps_den_bak)
		{
			ret = IMP_ISP_Tuning_SetSensorFPS(fps_num, 1);
			if(ret < 0)
			{
				Ucamera_ERR("isp set sensor fps error");
				return -1;
			}
		}
	}
	else if(1 == standard)
	{
		// 切换到N制
		if (g_Fps_Num <= 30)
			fps_num = 30;
		else if (g_Fps_Num == 50) {
			fps_num = 60;
		} else {
			fps_num = g_Fps_Num;
		}
		ret = IMP_ISP_Tuning_GetSensorFPS(&fps_num_bak, &fps_den_bak);
		if(ret < 0)
		{
			Ucamera_ERR("isp get sensor fps error");
		}
		if(fps_num != fps_num_bak / fps_den_bak)
		{
			ret = IMP_ISP_Tuning_SetSensorFPS(fps_num, 1);
			if(ret < 0)
			{
				Ucamera_ERR("isp set sensor fps error");
				return -1;
			}
		}
	}
	else
	{
		// DO Nothing
	}
	return ret;
}

static int strToInt(char const *str)
{
	int val;
	if (sscanf(str, "%d", &val) == 1) 
	{
		return val;
	}
	return -1;
}

static int strToInt_Hex(char const* str) {
	int val;
	if (sscanf(str, "%x", &val) == 1)
	{
		return val;
	}
	return -1;
}

int htoi(char s[])
{
	int n = 0;
	int i = 0;
	while (s[i] != '\0' && s[i] != '\n')
	{
		if (s[i] == '0')
		{
			if (s[i + 1] == 'x' || s[i + 1] == 'X')
				i += 2;
		}
		if (s[i] >= '0' && s[i] <= '9')
		{
			n = n * 16 + (s[i] - '0');
		}
		else if (s[i] >= 'a' && s[i] <= 'f')
		{
			n = n * 16 + (s[i] - 'a') + 10;
		}
		else if (s[i] >= 'A' && s[i] <= 'F')
		{
			n = n * 16 + (s[i] - 'A') + 10;
		}
		else
		{
			return -1;
		}
		++i;
	}

	return n;
}

int sample_video_focus_set(int value)
{
    focus.data[UVC_CUR] = value;
	stop_auto_focus();
	set_value_ctl_vcm(value * 3);
	ucamera_uvc_pu_attr_save(UVC_S_FOCUS_CONTROL, value);
    return 0;
}

int sample_video_focus_get(int value)
{
    return focus.data[UVC_CUR];
}

int sample_video_focus_auto_set(int value)
{
    focus_auto.data[UVC_CUR] = value;
	ucamera_uvc_pu_attr_save(UVC_S_FOCUS_AUTO_CONTROL, value);
	if (value == 0)
	{
		/* 手动调焦 */
	}
	else
	{
		/* 自动聚焦 */
		start_auto_focus();
	}	
    return 0;
}

int sample_video_focus_auto_get(int value)
{
    return focus_auto.data[UVC_CUR];
}

int ct_ae_exp_level2time_hk(u32 level)
{
	u32 time;		
	switch(level)
	{
		case 1:
			time = 20;
			break;
		case 2:
			time = 50;
			break;
		case 5:
			time = 100; //0.1ms
			break;
		case 10:
			time = 200;
			break;
		case 20:
			time = 300;
			break;
		case 39:
			time = 500;
			break;
		case 78:
			time = 1000;//1ms
			break;
		case 156:
			time = 2000;
			break;
		case 312:
			time = 5000;
			break;
		case 625:
			time = 10000;//10ms
			break;
		case 1250:
			time = 20000;
			break;
		case 2500:
		default:
			time = 40000;//40ms
			break;
	}
	return time;
}

int sample_video_exposure_time_set(int value)
{
    int ret;
	int tar_exptime;
	int tar_explines;
	int fps,vts;
	IMPISPSENSORAttr stSensorAttr;
	memset(&stSensorAttr,0,sizeof(IMPISPSENSORAttr));
	IMP_ISP_Tuning_GetSensorAttr(&stSensorAttr);	
	tar_exptime = ct_ae_exp_level2time_hk(value);
	
	fps = (stSensorAttr.fps - 1 ) >> 16;
	if(fps > 50)
	{
		tar_exptime = (tar_exptime>>1) * 8333 / 10000; 
	}
	else if(fps > 30) //50fps
	{
		tar_exptime = tar_exptime >>1;	
	}
	else if(fps > 25) //30fps
	{
		tar_exptime = tar_exptime * 8333 / 10000;
	}
	
	vts = stSensorAttr.vts;
	tar_explines = TIME2LINE(tar_exptime,vts,(1000000/fps));
	if(value == 2500)
	{
#if((CAMERA_MODEL == TV22_2K809) || (CAMERA_MODEL == U22_2K815) || (CAMERA_MODEL == UC2_2K819) || (CAMERA_MODEL == E12A_2K822))
	tar_explines = tar_explines - 25;//sensor priciple
#elif((CAMERA_MODEL == TV24_2K810) || (CAMERA_MODEL == U24_2K816) || (CAMERA_MODEL == UC4_2K820) || (CAMERA_MODEL == E14A_2K823))
	tar_explines = tar_explines - 8;//sensor priciple
#else
	tar_explines = tar_explines - 8;//sensor priciple
#endif
	}
    ret = IMP_ISP_Tuning_SetAe_IT_MAX(tar_explines);//set ExpLines to Ae
    if(ret)
	{
        Ucamera_ERR("ERROR:set exposure time Invalid leave:%d\n",value);
	}
    exposure_time.data[UVC_CUR] = value;
	ucamera_uvc_pu_attr_save(UVC_S_EXPOSURE_TIME_CONTROL, value);
    return 0;
}

/*
* min is down span
* max is up span
* ret 返回映射值
*/
int hik_mapping(int val,int min,int max)
{
	int ret = 128;
	if(val>128)
	{
		ret = (val-128)*max/128 + 128;
	}
	else
	{
		ret = -(128-val)*min/128 + 128;
	}
	return ret;
}

/*
* min is down span
* max is up span
* ret 返回映射值
*/
int hik_mapping_invert(int val,int min,int max)
{
	int ret = 128;
	if(0 == min || 0 == max)
	{
		return ret;
	}
	if(val>128)
	{
		ret = (val-128)*128/max +128;
	}
	else
	{
		ret = -(128-val)*128/min +128;
	}
	return ret;
}

/*
* isp_video_init
* 图像初始化
* 
*/
void isp_video_init(void)
{
	unsigned char index = 128;
	// 图像初始化，图像参数不需要保存
	iNeedSave = 0;
	//bright
	sample_video_brightness_set(brightness.data[UVC_CUR]);
#if 0
	index = brightness.data[UVC_CUR] & 0xff;
	index = hik_mapping(index,BRIGTNESS_SPAN_DOWN,BRIGHTNESS_SPAN_UP);
	IMP_ISP_Tuning_SetAeComp(index);
#endif
	//contrast
	sample_video_contrast_set(contrast.data[UVC_CUR]);
#if 0
	index = contrast.data[UVC_CUR] &0xff;
	IMP_ISP_Tuning_SetContrast(128);
	IMPISPGamma gam_curve ={0};
	isp_gam_interface(&gam_curve,index);
	IMP_ISP_Tuning_SetGamma(&gam_curve);
#endif
	// hue
	sample_video_hue_set(hue.data[UVC_CUR]);
#if 0
	IMP_ISP_Tuning_SetBcshHue(hue.data[UVC_CUR]);
#endif
	//saturation
	sample_video_saturation_set(saturation.data[UVC_CUR]);
#if 0
	index = saturation.data[UVC_CUR] & 0xff;
	index = hik_mapping(index,SAT_SPAN_DOWN,SAT_SPAN_UP);
	IMP_ISP_Tuning_SetSaturation(index);
#endif
	//sharpness
	sample_video_sharpness_set(sharpness.data[UVC_CUR]);
#if 0
	index = sharpness.data[UVC_CUR] & 0xff;
	index = hik_mapping(index,SHARPNESS_SPAN_DOWN,SHARPNESS_SPAN_UP);
	IMP_ISP_Tuning_SetSharpness(index);
#endif
	// backlight
	sample_video_backlight_compens_set(backlight_compens.data[UVC_CUR]);
	// gain
	IMP_ISP_Tuning_SetMaxDgain(gain.data[UVC_CUR]*64/255);
	// 白平衡，如果是手动白平衡，则配置
	if (0 == whitebalance_auto.data[UVC_CUR])
	{
		sample_video_whitebalance_set(whitebalance.data[UVC_CUR]);
	}

	// 图像的智能降噪
	IMP_ISP_Tuning_SetTemperStrength(128 + gDenoise - 64);
	// PN制
	hik_set_video_standard(gPN);
	// 曝光（通过调用自动曝光的设置接口来实现手动或者自动曝光）
	sample_video_auto_exposure_mode_set(auto_exposure_mode.data[UVC_CUR]);
	// 镜像调节
	IMP_ISP_Tuning_SetHVFLIP(gHVFlip);
	usleep(100*1000);
	// 数字变倍
	sample_video_zoom_set(zoom.data[UVC_CUR]);
	iNeedSave = 1;
}
int sample_video_exposure_time_get(void)
{
	return exposure_time.data[UVC_CUR];
}

int sample_video_auto_exposure_mode_set(int value)
{
	IMPISPSENSORAttr stSensorAttr;
	int inttmax;
	memset(&stSensorAttr,0,sizeof(IMPISPSENSORAttr));
	IMP_ISP_Tuning_GetSensorAttr(&stSensorAttr);
#if((CAMERA_MODEL == TV22_2K809) || (CAMERA_MODEL == U22_2K815) || (CAMERA_MODEL == UC2_2K819) || (CAMERA_MODEL == E12A_2K822))
	inttmax = stSensorAttr.vts - 25;//sensor priciple
#elif((CAMERA_MODEL == TV24_2K810) || (CAMERA_MODEL == U24_2K816) || (CAMERA_MODEL == UC4_2K820) || (CAMERA_MODEL == E14A_2K823))
	inttmax = stSensorAttr.vts - 8;//sensor priciple
#else
	inttmax = stSensorAttr.vts - 8;//sensor priciple
#endif
	// TODO : YYL 君正平台的自动曝光，在Windows平台下下发的value为8，其他平台未知
	if(8 == value)
	{
		IMP_ISP_Tuning_SetAe_IT_MAX(inttmax);//set ExpLines to Ae
	}
	else
	{
		sample_video_exposure_time_set(exposure_time.data[UVC_CUR]);
	}
	auto_exposure_mode.data[UVC_CUR] = value;
	ucamera_uvc_pu_attr_save(UVC_S_AUTO_EXPOSURE_MODE_CONTROL, value);
	return 0;
}

int sample_video_auto_exposure_mode_get(void)
{
	return auto_exposure_mode.data[UVC_CUR];
}

int sample_video_roll_set(int value)
{
	Ucamera_LOG("iSelfParamFlag1 = %d, value = %d", iSelfParamFlag1, value);
	if(SELF_PARAME_WIDEDY == iSelfParamFlag1)
	{
		// 宽动态
	}
	else if(SELF_PARAME_NOISE == iSelfParamFlag1)
	{
		// 降噪
		IMP_ISP_Tuning_SetTemperStrength(128 + value - 64);
		gDenoise = value;
		ucamera_uvc_pu_attr_save(UVC_S_DENOISE_CONTROL ,value);
	}
	else if(SELF_PARAME_FOUCE == iSelfParamFlag1)
	{
		// 自动聚焦（旧版本私有属性用，后改为待定，暂不使用）
		pthread_create(&pt_vcm, NULL, thread_vcm_check, NULL);
	}
	else if(SELF_PARAME_EC == iSelfParamFlag1)
	{
		// 回声消除
	}
	else if(SELF_PARAME_ONEKEYWH == iSelfParamFlag1)
	{
		// 一键白平衡
	}
	else if(SELF_PARAME_IMAGE == iSelfParamFlag1)
	{
		// 镜像
		IMP_ISP_Tuning_SetHVFLIP(value);
		// 解除滚动和镜像的联动
		// roll.data[UVC_CUR] = value;
		gHVFlip = value;
		ucamera_uvc_pu_attr_save(UVC_HVFLIP_CONTROL ,value);
	}
	else if(SELF_PARAME_CHECKAF == iSelfParamFlag1)
	{
		// 触发VCM自检
		pthread_create(&pt_vcm, NULL, thread_vcm_check, NULL);
	}
	else if(SELF_PARAME_FASTSHUT == iSelfParamFlag1)
	{
		// 触发快门时间调节
		#if 0
		// 现在不再使用快门时间
		if(value == 5)
		{
			hik_set_fastest_shutter(0);
		}
		else
		{
			hik_set_fastest_shutter(value);
			gFastShutter = value;
			// save参数
			ucamera_uvc_pu_attr_save(UVC_FAST_SHUTTER, value);
		}
		#endif
	}
	else if(SELF_PARAME_CHECKTOF == iSelfParamFlag1)
	{
		// TOF相关
#if(1 == STM_DTOF_EN)
		if (0 == value)
		{
			// TOF 校准
			  stop_auto_focus();
			  st_tof.tag = 2;
			  pthread_create(&pt_tof, NULL, thread_tof_run, (void *)&st_tof);
		}
		else if (1 == value)
		{
			// TOF检测
			stop_auto_focus();
			st_tof.tag = 1;
			pthread_create(&pt_tof, NULL, thread_tof_run, (void *)&st_tof);
		}
		else if (111 == value)
		{
			// TOF测距
			stop_auto_focus();
			st_tof.tag = 3;
			pthread_create(&pt_tof, NULL, thread_tof_run, (void *)&st_tof);
		}
		else
		{
		}
#endif
	}
	// 图像模式
	else if (SELF_PARAME_IMAGEMODE == iSelfParamFlag1)
	{
		hik_set_image_mode(value);
		ucamera_uvc_pu_attr_save(UVC_S_IMAGEMODE_CONTROL, value);
	}
	else if(SELF_PARAME_RESETCHN1 == iSelfParamFlag1)
	{
		// 私有属性重置通道1
		hik_set_image_mode(0);
		ucamera_uvc_pu_attr_save(UVC_S_IMAGEMODE_CONTROL, 0);
	}
	else if(SELF_PARAME_RESETCHN2 == iSelfParamFlag1)
	{
		// 私有属性重置通道2
		// 聚焦灵敏度
		gAFGarde = 2;
		st_jx.garde = af_grade[gAFGarde];
		setSensLevel(st_jx.garde);
		ucamera_uvc_pu_attr_save(UVC_S_AFGRADE_CONTROL ,gAFGarde);
		// 镜像
		IMP_ISP_Tuning_SetHVFLIP(0);
		gHVFlip = 0;
		ucamera_uvc_pu_attr_save(UVC_HVFLIP_CONTROL ,0);
	}
	else if(SELF_PARAME_CHOOSEPN == iSelfParamFlag1)
	{
		// PN制切换
		if(0 == hik_set_video_standard(value))
		{
			gPN = value;
			ucamera_uvc_pu_attr_save(UVC_S_STANDARD_CONTROL ,gPN);
		}
		else
		{
			Ucamera_ERR("set pn error");
		}
	}
	else if(SELF_PARAME_LENSCAL == iSelfParamFlag1)
	{
		// 镜头曲线老化测试
	}
	else if(SELF_PARAME_AFGRADE == iSelfParamFlag1)
	{
		gAFGarde = value;
		st_jx.garde = af_grade[gAFGarde];
		// 镜头灵敏度调节 
		ucamera_uvc_pu_attr_save(UVC_S_AFGRADE_CONTROL ,gAFGarde);
		setSensLevel(st_jx.garde);
	}
	// 数据获取
	else if (roll.data[UVC_MAX] == iSelfParamFlag1)
	{
		Ucamera_LOG("in get data iSelfParamFlag1 = %d", iSelfParamFlag1);
#if(1 == STM_DTOF_EN)
		// TOF数据上传
		if(value == 1)
		{
			roll.data[UVC_CUR] = tof_check.check_status;
			return 0;
        }
		else if(value == 2)
        {
			roll.data[UVC_CUR] = tof_check.tof1[0].milliMeter/10;
			return 0;
        }
		else if(value == 3)
		{
			roll.data[UVC_CUR] = tof_check.tof1[0].rangeStatus;
			return 0;
        }
		else if(value == 4)
        {
			roll.data[UVC_CUR] = tof_check.tof2[0].milliMeter/10;
			return 0;
		}
		else if(value == 5)
		{
			roll.data[UVC_CUR] = tof_check.tof2[0].rangeStatus;
			return 0;
        }
		else if(value == 6)
		{
			// 能量能量
			roll.data[UVC_CUR] = tof_check.tof1[0].signalRateRtnMegaCps/65536;
			return 0;
		}
		else if(value == 7)
		{
			// TOF2能量
			roll.data[UVC_CUR] = tof_check.tof2[0].signalRateRtnMegaCps/65536;
			return 0;
		}
		// 属性数据返回
		else if(SELF_PARAME_NOISE == value)
#else
		// 属性数据返回
		if(SELF_PARAME_NOISE == value)
#endif
		{
			roll.data[UVC_CUR] = gDenoise;
			return 0;
		}
		else if(SELF_PARAME_IMAGE == value)
		{
			roll.data[UVC_CUR] = gHVFlip;
			return 0;
		}
		else if (SELF_PARAME_IMAGEMODE == value)
		{
			roll.data[UVC_CUR] = gImageMode;
			return 0;
		}
		else if(SELF_PARAME_CHECKAF == value)
		{
			roll.data[UVC_CUR] = g_VcmValue/10;
			printf("%d\n", roll.data[UVC_CUR]);
			return 0;
		}
		else if(SELF_PARAME_CHOOSEPN == value)
		{
			// PN制切换
			roll.data[UVC_CUR] = gPN;
			return 0;
		}
		else if(SELF_PARAME_LENSCAL == value)
		{
			// 镜头曲线老化测试
			return 0;
		}
		else if(SELF_PARAME_AFGRADE == value)
		{
			// 镜头灵敏度调节 gAFGarde
			roll.data[UVC_CUR] = gAFGarde;
			return 0;
		}
		else if (value == roll.data[UVC_MAX])
		{
			roll.data[UVC_CUR] = gHVFlip;
		}
	}
	
	else
	{
		// TODO 滚动不再绑定镜像
		roll.data[UVC_CUR] = value;
		ucamera_uvc_pu_attr_save(UVC_HVFLIP_CONTROL, value);
	}	
	iSelfParamFlag0 = 0;
	iSelfParamFlag1 = 0;
	iSelfParamFlag2 = 0;
	return 0;
}

int sample_video_roll_get(int value)
{
	return roll.data[UVC_CUR];
}

int sample_video_zoom_set(int value)
{
#if(HIK_USB_PRODUCT == HIK_USB_OS02D20)
	// 2M 相机
#else
	// 4M相机
	if(50 <= g_Fps_Num)
	{
		// 4M的高帧率下不支持数字变倍
		Ucamera_LOG("this fps not support zoom");
		return 0;
	}
#endif
	int ret;
	int zoomwidth_cur, zoomheight_cur;
	float value_cur, value_last;
	value_cur = value;
	zoomwidth_cur = round(g_VideoWidth / sqrt(value_cur / 100));
	zoomwidth_cur = round((float)(zoomwidth_cur / 4)) * 4;
	zoomheight_cur = round(g_VideoHeight / sqrt(value_cur / 100));
	zoomheight_cur = round((float)(zoomheight_cur / 4)) * 4;

	fcrop_obj.fcrop_enable = 1;
	fcrop_obj.fcrop_width = zoomwidth_cur;
	fcrop_obj.fcrop_height = zoomheight_cur;
	fcrop_obj.fcrop_left = round((float)(g_VideoWidth - zoomwidth_cur) / 2);
	fcrop_obj.fcrop_top = round((float)(g_VideoHeight - zoomheight_cur) / 2);

	ret = IMP_ISP_Tuning_SetFrontCrop(&fcrop_obj);
	if (ret < 0)
	{
		Ucamera_ERR(TAG, "%s(%d)IMP Set Fcrop failed=%d\n", __func__, __LINE__);
	 	return -1;
	}
	zoom.data[UVC_CUR] = value;
	ucamera_uvc_pu_attr_save(UVC_S_ZOOM_ABSOLUTE_CONTROL, value);
	return 0;
}

int sample_video_zoom_get(void)
{
	return zoom.data[UVC_CUR];
}

int sample_video_backlight_compens_set(int value)
{
	int ret;
	if (value < 0 || value > 10) 
	{
		Ucamera_ERR("set BackLight Invalid leave:%d\n", value);
		return 0;
	}
	if(4 == value)
	{
		ret = IMP_ISP_Tuning_SetHiLightDepress(0);
		ret |= IMP_ISP_Tuning_SetBacklightComp(0);
		if (ret < 0) {
			Ucamera_LOG("failed to set sensor fps\n");
			return -1;
		}
	}
	else if(value < 4)
	{
		ret = IMP_ISP_Tuning_SetHiLightDepress(4-value);
	}
	else
	{
		ret = IMP_ISP_Tuning_SetBacklightComp(value-4);
	}
	if (ret)
	{
		Ucamera_ERR("ERROR: set BackLight Invalid leave:%d\n", value);
	}
	backlight_compens.data[UVC_CUR] = value;
	ucamera_uvc_pu_attr_save(UVC_BACKLIGHT_COMPENSATION_CONTROL, value);
	return 0;
}

int sample_video_backlight_compens_get(void)
{
	return backlight_compens.data[UVC_CUR];
}

int sample_video_brightness_set(int value)
{
	int ret;
	unsigned char bright = 0;
	unsigned char index = value & 0xff;

	bright = hik_mapping(index,BRIGTNESS_SPAN_DOWN,BRIGHTNESS_SPAN_UP);
	ret = IMP_ISP_Tuning_SetAeComp(bright);
	if (ret)
	{
		Ucamera_LOG("ERROR: set BrightNess failed :%d\n", ret);
	}
	brightness.data[UVC_CUR] = index;
	ucamera_uvc_pu_attr_save(UVC_BRIGHTNESS_CONTROL, index);
	return 0;
}

int sample_video_brightness_get(void)
{
	return brightness.data[UVC_CUR];
}

int sample_video_contrast_set(int value)
{
	int ret;
	unsigned char gam_index = value & 0xff;
	IMP_ISP_Tuning_SetContrast(128);
	IMPISPGamma gam_curve ={0};
	isp_gam_interface(&gam_curve,gam_index);
	ret = IMP_ISP_Tuning_SetGamma(&gam_curve);
	if (ret)
	{
		Ucamera_ERR("set Contrast failed:%d\n", ret);
	}
	contrast.data[UVC_CUR] = gam_index;
	ucamera_uvc_pu_attr_save(UVC_CONTRAST_CONTROL, gam_index);
	return 0;
}

int sample_video_contrast_get(void)
{
	return contrast.data[UVC_CUR];
}

int sample_video_saturation_set(int value)
{
	int ret;
	unsigned char tmp = 0;
	unsigned char index = value & 0xff;
	tmp = hik_mapping(index,SAT_SPAN_DOWN,SAT_SPAN_UP);
	ret = IMP_ISP_Tuning_SetSaturation(tmp);
	if (ret)
	{
		Ucamera_ERR("set Saturation failed:%d\n", ret);
	}
	saturation.data[UVC_CUR] = index;
	ucamera_uvc_pu_attr_save(UVC_SATURATION_CONTROL, index);
	return 0;
}

int sample_video_saturation_get(void)
{
	return saturation.data[UVC_CUR];
}

int sample_video_sharpness_set(int value)
{
	int ret;
	unsigned char tmp = 0;
	unsigned char index = value & 0xff;

	tmp = hik_mapping(index,SHARPNESS_SPAN_DOWN,SHARPNESS_SPAN_UP);
	ret = IMP_ISP_Tuning_SetSharpness(tmp);
	if (ret)
	{
		Ucamera_LOG("set Sharpness failed:%d\n", ret);
	}
	sharpness.data[UVC_CUR] = index;
	ucamera_uvc_pu_attr_save(UVC_SHARPNESS_CONTROL, index);
	return 0;
}

int sample_video_sharpness_get(void)
{
	return sharpness.data[UVC_CUR];
}

int sample_video_hue_set(int value)
{
	int ret;
	unsigned char hue_value = value & 0xff;
	ret = IMP_ISP_Tuning_SetBcshHue(hue_value);
	if (ret)
	{
		Ucamera_LOG("set Hue failed:%d\n", ret);
	}
	hue.data[UVC_CUR] = hue_value;
	ucamera_uvc_pu_attr_save(UVC_HUE_CONTROL, hue_value);
	return 0;
}

int sample_video_hue_get(void)
{
	return hue.data[UVC_CUR];
}

// YYL : TODO 没在使用，图像确认是否删除
int sample_video_t_compens_set(int value)
{
	int ret;
	if (value < 0 || value > 10) 
	{
		Ucamera_ERR("set BackLight Invalid leave:%d\n", value);
		return 0;
	}
	ret = IMP_ISP_Tuning_SetHiLightDepress(value);
	if (ret)
	{
		Ucamera_ERR("ERROR: set BackLight Invalid leave:%d\n", value);
	}
	backlight_compens.data[UVC_CUR] = value;
	ucamera_uvc_pu_attr_save(UVC_BACKLIGHT_COMPENSATION_CONTROL, value);
	return 0;
}

int sample_video_gamma_set(int value)
{
	int ret;
	unsigned char tmp = 0;
	tmp = value & 0xff;
	ret = IMP_ISP_Tuning_SetContrast(tmp);
	if (ret)
	{
		Ucamera_ERR("set Contrast failed:%d\n", ret);
	}
	pu_gamma.data[UVC_CUR] = tmp;
	ucamera_uvc_pu_attr_save(UVC_GAMMA_CONTROL, tmp);
	return 0;
}

int sample_video_gamma_get(void)
{
	return pu_gamma.data[UVC_CUR];
}

/*
* value高16位代表模式（0:自动/ 1:手动
* value 底16位为白平衡实际值
*/
int sample_video_whitebalance_set(int value)
{
	int ret;
	unsigned short gain;
	IMPISPWB wb = {0};
	
	gain = value & 0xffff;
	wb.rgain = gain;
	wb.bgain = 160000/gain;
	wb.mode = ISP_CORE_WB_MODE_MANUAL;
	g_aaa_param.awb_param.auto_enable = 1;//manual for hik awb
	ret = IMP_ISP_Tuning_SetWB(&wb);
	if (ret)
	{
		Ucamera_ERR("set WhiteBalance failed:%d\n", ret);
	}
	whitebalance.data[UVC_CUR] = wb.rgain;
	ucamera_uvc_pu_attr_save(UVC_WHITE_BALANCE_TEMPERATURE_CONTROL, wb.rgain);
	return 0;
}

int sample_video_whitebalance_get(void)
{
	return whitebalance.data[UVC_CUR];
}

int sample_video_whitebalance_auto_set(int value)
{
	//g_aaa_param.awb_param.auto_enable = 0;//auto for hik awb
	if(value == 1)//勾上实现海康算法
	{	
		g_aaa_param.awb_param.auto_enable = 0;//auto for hik awb
	}
	else//取消实现客户手动配置
	{
		g_aaa_param.awb_param.auto_enable = 1;//manual for hik awb
	}
	whitebalance_auto.data[UVC_CUR] = value;	
	ucamera_uvc_pu_attr_save(UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL, value);
    return 0;
}

int sample_video_whitebalance_auto_get()
{
    return whitebalance_auto.data[UVC_CUR];
}

int sample_video_powerlinefreq_set(int value)
{
	// 白平衡已不使用，通过直接改变Sensor的输出和抽插帧的方式
	int ret;
	IMPISPAntiflickerAttr attr;

	attr = value;
	if (attr < IMPISP_ANTIFLICKER_DISABLE || attr >= IMPISP_ANTIFLICKER_BUTT) 
	{
		Ucamera_ERR("[ERROR]Sample set PowerLine Freq Invalid level:%d\n", value);
		return 0;
	}
	ret = IMP_ISP_Tuning_SetAntiFlickerAttr(attr);
	if (ret)
	{
		Ucamera_ERR("set PowerLine Freq failed:%d\n", ret);
		return 0;
	}
	powerlinefreq.data[UVC_CUR] = attr;
	ucamera_uvc_pu_attr_save(UVC_POWER_LINE_FREQUENCY_CONTROL, attr);
	return 0;
}

int sample_video_powerlinefreq_get(void)
{
	return powerlinefreq.data[UVC_CUR];
}

int sample_video_gain_set(int value)
{
	if(0 == value)
	{
		iSelfParamFlag0 = 1;
		iSelfParamFlag1 = 0;
		iSelfParamFlag2 = 0;
		value = iSelfParamTemp;
	}
	else
	{
		if(1 == iSelfParamFlag0 && 0 == iSelfParamFlag2)
		{
			iSelfParamFlag1 = value;
			iSelfParamFlag2 = 1;
			value = iSelfParamTemp;
		}
		else
		{
			uint32_t tmp = value;
			gain.data[UVC_CUR] = value;
			tmp = tmp*64/255;
			IMP_ISP_Tuning_SetMaxDgain(tmp);
			iSelfParamFlag0 = 0;
			iSelfParamFlag1 = 0;
			iSelfParamFlag2 = 0;
			iSelfParamTemp = value;
			ucamera_uvc_pu_attr_save(UVC_GAIN_CONTROL, value);
		}
	}
	return 0;
}

int sample_video_gain_get(int value)
{
	return gain.data[UVC_CUR];
}

int uvc_pu_attr_setcur(int type, int value)
{
	int ret = 0;
	struct Ucamera_Video_PU_Control *pu_attr = NULL;

	switch (type) 
	{
	case UVC_BACKLIGHT_COMPENSATION_CONTROL:
		pu_attr = &backlight_compens;
		break;
	case UVC_BRIGHTNESS_CONTROL:
		pu_attr = &brightness;
		break;
	case UVC_CONTRAST_CONTROL:
		pu_attr = &contrast;
		break;
	case UVC_GAIN_CONTROL:
		pu_attr = &gain;
		break;
	case UVC_POWER_LINE_FREQUENCY_CONTROL:
		pu_attr = &powerlinefreq;
		break;
	case UVC_HUE_CONTROL:
		pu_attr = &hue;
		break;
	case UVC_SATURATION_CONTROL:
		pu_attr = &saturation;
		break;
	case UVC_SHARPNESS_CONTROL:
		pu_attr = &sharpness;
		break;
	case UVC_GAMMA_CONTROL:
		pu_attr = &pu_gamma;
		break;
	case UVC_WHITE_BALANCE_TEMPERATURE_CONTROL:
		pu_attr = &whitebalance;
		break;
	case UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
		pu_attr = &whitebalance_auto;
		break;
	case UVC_FAST_SHUTTER:
		gFastShutter = value;
		break;
	case UVC_HVFLIP_CONTROL:
		roll.data[UVC_CUR] = value;
		gHVFlip = value;
		break;
	case UVC_S_EXPOSURE_TIME_CONTROL:
		exposure_time.data[UVC_CUR] = value;
		break;
	case UVC_S_AUTO_EXPOSURE_MODE_CONTROL:
		auto_exposure_mode.data[UVC_CUR] = value;
		break;
	case UVC_S_FOCUS_CONTROL:
		focus.data[UVC_CUR] = value;
		break;
	case UVC_S_FOCUS_AUTO_CONTROL:
		focus_auto.data[UVC_CUR] = value;
		break;
	case UVC_S_DENOISE_CONTROL:
		gDenoise = value;
		break;
	case UVC_S_ZOOM_ABSOLUTE_CONTROL:
		zoom.data[UVC_CUR] = value;
		break;
	case UVC_S_IMAGEMODE_CONTROL:
		gImageMode = value;
		break;
	case UVC_S_STANDARD_CONTROL:
	    gPN = value;
	    break;
	case UVC_S_AFGRADE_CONTROL:
	    gAFGarde = value;
	    break;
	default:
		Ucamera_LOG("Unkown uvc pu type:%d\n", type);
		ret = -1;
		break;
	}
	if (pu_attr)
	{
		pu_attr->data[UVC_CUR] = value;
	}

	return ret;
}


char *uvc_pu_type_to_string(int type)
{
	return pu_string[type].s;
}

int uvc_pu_string_to_type(char *string)
{
	int i, index;
	index = sizeof(pu_string)/sizeof(struct uvc_pu_string);

	for (i = 0; i < index; i++) 
	{
		if (strcmp(string, pu_string[i].s) == 0)
		{
			return pu_string[i].id;
		}
	}
	return 0;
}

int ucamera_uvc_pu_attr_load(void)
{
	char key[64] = {0};
	char value[16] = {0};
	char line_str[128] = {0};
	int type;


	if ((uvc_attr_fd = fopen("/media/uvc.attr", "r+")) == NULL) 
	{
		Ucamera_ERR("%s open config file failed!\n", __func__);
		return -1;
	}
	if (pthread_mutex_init(&uvc_attr_mutex, NULL) != 0)
	{
		Ucamera_ERR("%s init mutex failed\n", __func__);
		return -1;
	}
	while (!feof(uvc_attr_fd)) 
	{
		if (fscanf(uvc_attr_fd, "%[^\n]", line_str) < 0)
		{
			break;
		}
		if (sscanf(line_str, "%[^:]:%[^\n]", key, value) != 2) 
		{
			Ucamera_LOG("warning: skip config %s\n", line_str);
			fseek(uvc_attr_fd , 1L, SEEK_CUR);
			continue;
		}
		if ((type = uvc_pu_string_to_type(key))) 
		{
			uvc_pu_attr_setcur(type, strToInt(value));
		}
		fseek(uvc_attr_fd , 1L, SEEK_CUR);
	}
	return 0;
}

int ucamera_uvc_pu_attr_save(int type, int value)
{
	char key[64] = {0};
	char data[16] = {0};
	char line_str[128] = {0};
	char *attr_string = NULL;
	if (0 == iNeedSave)
	{
		return 0;
	}
	if (uvc_attr_fd == NULL) 
	{
		Ucamera_LOG("%s can not open uvc config file!\n", __func__);
		return -1;
	}
	attr_string = uvc_pu_type_to_string(type);

	if (attr_string == NULL)
	{
		return -1;
	}
	pthread_mutex_lock(&uvc_attr_mutex);
	fseek(uvc_attr_fd, 0L, SEEK_SET);

	while (!feof(uvc_attr_fd)) 
	{
		if (fscanf(uvc_attr_fd, "%[^\n]", line_str) < 0)
		{
			break;
		}
		if (sscanf(line_str, "%[^:]:%[^\n]", key, data) != 2) 
		{
			Ucamera_LOG("warning: Invalid param:%s\n", line_str);
			fseek(uvc_attr_fd , 1L, SEEK_CUR);
			continue;
		}
		if (strcmp(key, attr_string) == 0) 
		{
			fseek(uvc_attr_fd, -strlen(line_str), SEEK_CUR);
			fprintf(uvc_attr_fd, "%s:%04d", key, value);
		}
		fseek(uvc_attr_fd , 1L, SEEK_CUR);
	}
	pthread_mutex_unlock(&uvc_attr_mutex);
	return 0;
}

int imp_system_init(void)
{
	int ret;
	/* Step.1 System init */
	ret = sample_system_init();
	if (ret < 0) 
	{
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
		return -1;
	}
	return 0;
}

int imp_isp_tuning_init(void)
{
	int ret, fps_num;
	IMPISPAntiflickerAttr attr;

	/* enable turning, to debug graphics */
	ret = IMP_ISP_EnableTuning();
	if(ret < 0)
	{
		IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
		return -1;
	}
	ret = IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "failed to set running mode\n");
		return -1;
	}
	return 0;
}

int imp_sdk_init(int format, int width, int height)
{
	int i, ret, tmp = 0;
	IMPFSChnAttr *imp_chn_attr_tmp;
	Ucamera_LOG("IMP SDK reinit!\n");

	imp_chn_attr_tmp = &chn[0].fs_chn_attr;
	/*imp_chn_attr_tmp->outFrmRateNum = SENSOR_FRAME_RATE_NUM_30;*/
	if (0 == gPN) 
	{
		if (g_Fps_Num == 30)
		{
			imp_chn_attr_tmp->outFrmRateNum = 25;
		}
		else if (g_Fps_Num == 60) 
		{
			imp_chn_attr_tmp->outFrmRateNum = 50;
		} else 
		{
			imp_chn_attr_tmp->outFrmRateNum = g_Fps_Num;
		}

	} else if (1 == gPN) 
	{
		if (g_Fps_Num == 25)
		{
			imp_chn_attr_tmp->outFrmRateNum = 30;
		}
		else if (g_Fps_Num == 50) 
		{
			imp_chn_attr_tmp->outFrmRateNum = 60;
		} else 
		{
			imp_chn_attr_tmp->outFrmRateNum = g_Fps_Num;
		}

	} else
	{
		imp_chn_attr_tmp->outFrmRateNum = g_Fps_Num;
	}
	imp_chn_attr_tmp->picWidth = width;
	imp_chn_attr_tmp->picHeight = height;
	imp_chn_attr_tmp->scaler.outwidth = width;
	imp_chn_attr_tmp->scaler.outheight = height;

	if (width == g_VideoWidth && height == g_VideoHeight) 
	{
		imp_chn_attr_tmp->scaler.enable = 0;
	} 
	else
	{
		imp_chn_attr_tmp->scaler.enable = 1;
	}
	if (width > 2560 && height > 1440)
	{
		imp_chn_attr_tmp->nrVBs = 1;
	}
	else
	{
		imp_chn_attr_tmp->nrVBs = 3;
	}
	imp_chn_attr_tmp->crop.enable = 0;

	if ((g_VideoWidth*height) != (g_VideoHeight*width))
	{
		imp_chn_attr_tmp->crop.enable = 1;
		tmp =  g_VideoWidth*height/g_VideoHeight;
		if (tmp > width) 
		{
			if (tmp%16)
			{
				tmp -= tmp%16;
			}
			imp_chn_attr_tmp->scaler.outwidth = tmp;
			imp_chn_attr_tmp->scaler.outheight = height;
			imp_chn_attr_tmp->crop.left = (tmp - width)/2;
			imp_chn_attr_tmp->crop.top = 0;
			imp_chn_attr_tmp->crop.width = width;
			imp_chn_attr_tmp->crop.height = height;
		}
		else 
		{
			tmp = g_VideoHeight*width/g_VideoWidth;
			imp_chn_attr_tmp->scaler.outwidth = width;
			imp_chn_attr_tmp->scaler.outheight = tmp;
			imp_chn_attr_tmp->crop.left = 0;
			imp_chn_attr_tmp->crop.top = (tmp-height)/2;
			imp_chn_attr_tmp->crop.width = width;
			imp_chn_attr_tmp->crop.height = height;
		}
		Ucamera_LOG("IMP: Crop enable w:%d h:%d left:%d top:%d\n",
							imp_chn_attr_tmp->crop.width,
							imp_chn_attr_tmp->crop.height,
							imp_chn_attr_tmp->crop.left,
							imp_chn_attr_tmp->crop.top);
	}
	Ucamera_LOG("IMP: Scaler enable w:%d h:%d\n",
						imp_chn_attr_tmp->scaler.outwidth,
						imp_chn_attr_tmp->scaler.outheight);
	if (g_Power_save) 
	{
		ret = IMP_ISP_EnableSensor();
		if(ret < 0)
		{
			IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
			return -1;
		}
		imp_isp_tuning_init();
	}
	fcrop_obj.fcrop_enable = 0;

	/* Step.2 FrameSource init */
	ret = sample_framesource_init();
	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

	if (format == V4L2_PIX_FMT_YUYV || format == V4L2_PIX_FMT_NV12) 
	{
		IMPFSChnAttr fs_chn_attr[2];
		/* Step.3 Snap raw config */
		ret = IMP_FrameSource_GetChnAttr(0, &fs_chn_attr[0]);
		if (ret < 0) 
		{
			IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_GetChnAttr failed\n", __func__, __LINE__);
			return -1;
		}

		fs_chn_attr[0].pixFmt = PIX_FMT_NV12;//PIX_FMT_YUYV422;
		ret = IMP_FrameSource_SetChnAttr(0, &fs_chn_attr[0]);
		if (ret < 0)
		{
			IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_SetChnAttr failed\n", __func__, __LINE__);
			return -1;
		}

		/* Step.3 config sensor reg to output colrbar raw data*/
		/* to do */

		/* Step.4 Stream On */
		if (chn[0].enable)
		{
			ret = IMP_FrameSource_EnableChn(chn[0].index);
			if (ret < 0)
			{
				IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn[0].index);
				return -1;
			}
		}

		/* Step.4 Snap raw */
		ret = IMP_FrameSource_SetFrameDepth(0, 1);
		if (ret < 0)
		{
			IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_SetFrameDepth failed\n", __func__, __LINE__);
			return -1;
		}
		return 0;
	}

	for (i = 0; i < FS_CHN_NUM; i++) 
	{
		if (chn[i].enable) 
		{
			ret = IMP_Encoder_CreateGroup(chn[i].index);
			if (ret < 0) 
			{
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
				return -1;
			}
		}
	}

	/* Step.3 Encoder init */
	switch (format) 
	{
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_NV12:
			break;
		case V4L2_PIX_FMT_MJPEG:
			ret = sample_jpeg_init();
			break;
		case V4L2_PIX_FMT_H264:
			ret = sample_encoder_init();
			break;
		default:
			break;
	}
	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "Encoder init failed\n");
		return -1;
	}

	/* Step.4 Bind */
	for (i = 0; i < FS_CHN_NUM; i++)
	{
		if (chn[i].enable) 
		{
			ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			if (ret < 0) 
			{
				IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed\n",i);
				return -1;
			}
		}
	}

	/* Step.5 Stream On */
	ret = sample_framesource_streamon();
	if (ret < 0) 
	{
		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		return -1;
	}

	if (format == V4L2_PIX_FMT_H264) 
	{
#if 1
		ret = IMP_Encoder_StartRecvPic(chn[0].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", chn[0].index);
			return -1;
		}
#else
		sample_get_h264_start();
#endif
	}
	else if (format == V4L2_PIX_FMT_MJPEG) 
	{
		ret = IMP_Encoder_StartRecvPic(chn[0].index + 3);
		if (ret < 0)
		{
			IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", chn[0].index + 3);
			return -1;
		}

	}
	return 0;
}

int imp_sdk_deinit(int format)
{
	int i, ret;

	if (format == V4L2_PIX_FMT_YUYV || format == V4L2_PIX_FMT_NV12)
	{
		/* Step.5 Stream Off */
		if (chn[0].enable)
		{
			ret = IMP_FrameSource_DisableChn(chn[0].index);
			if (ret < 0) 
			{
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[0].index);
				return -1;
			}
		}

		/* Step.6 FrameSource exit */
		if (chn[0].enable) 
		{
			/*Destroy channel i*/
			ret = IMP_FrameSource_DestroyChn(0);
			if (ret < 0)
			{
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn() error: %d\n", ret);
				return -1;
			}
		}
	} else 
	{
		if (format == V4L2_PIX_FMT_H264)
		{
		#if 1
			ret = IMP_Encoder_StopRecvPic(chn[0].index);
			if (ret < 0)
			{
				IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic(%d) failed\n", chn[0].index);
				return -1;
			}
		#else
			sample_get_h264_stop();
#endif
		} 
		else if (format == V4L2_PIX_FMT_MJPEG)
		{
			ret = IMP_Encoder_StopRecvPic(chn[0].index + 3);
			if (ret < 0)
			{
				IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic(%d) failed\n", chn[0].index + 3);
				return -1;
			}

		}

		/* Step.a Stream Off */
		ret = sample_framesource_streamoff();
		if (ret < 0)
		{
			IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
			return -1;
		}

		/* Step.b UnBind */
		for (i = 0; i < FS_CHN_NUM; i++)
		{
			if (chn[i].enable)
			{
				ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
				if (ret < 0)
				{
					IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
					return -1;
				}
			}
		}

		/* Step.c Encoder exit */
		switch (format)
		{
			case V4L2_PIX_FMT_YUYV:
			case V4L2_PIX_FMT_NV12:
				break;
			case V4L2_PIX_FMT_MJPEG:
				ret = sample_jpeg_exit();
				break;
			case V4L2_PIX_FMT_H264:
				ret = sample_encoder_exit();
				break;
		}
		if (ret < 0)
		{
			IMP_LOG_ERR(TAG, "Encoder init failed\n");
			return -1;
		}

		for (i = 0; i < FS_CHN_NUM; i++)
		{
			if (chn[i].enable)
			{
				ret = IMP_Encoder_DestroyGroup(chn[i].index);
				if (ret < 0)
				{
					IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
					return -1;
				}
			}
		}

		/* Step.d FrameSource exit */
		ret = sample_framesource_exit();
		if (ret < 0)
		{
			IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
			return -1;
		}
	}

	if (g_Power_save)
	{
		ret = IMP_ISP_DisableTuning();
		if(ret < 0)
		{
			IMP_LOG_ERR(TAG, "IMP_ISP_DisableTuning failed\n");
			return -1;
		}

		ret = IMP_ISP_DisableSensor();
		if(ret < 0)
		{
			IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
			return -1;
		}
	}
	return 0;
}

int imp_system_exit(void)
{
	int ret;
	/* Step.e System exit */
	ret = sample_system_exit();
	if (ret < 0)
	{
		IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
	}
	return ret;
}

int imp_SensorFPS_Adapture(void)
{
	int ret, ev_cur = 0;
	int sensor_FPS = 0;
	static int  ev_last = 0;
	IMPISPEVAttr evattr = {0};

	if (gStreaming == 0)
	{
		return 0;
	}
	ret = IMP_ISP_Tuning_GetEVAttr(&evattr);
	if (ret < 0) 
	{
		Ucamera_ERR("failed to get evattr\n");
		return -1;
	}
	Ucamera_LOG("IMP get Ev:%d\n", evattr.ev);

	if (evattr.ev > 8000)
	{
		ev_cur = 2;
	} 
	else if (evattr.ev > 4000)
	{
		ev_cur = 1;
	}
	else
	{
		ev_cur = 0;
	}
	if (ev_cur == ev_last)
	{
		return 0;
	}
	ev_last = ev_cur;

	switch (ev_cur) 
	{
		case 2:
			sensor_FPS = 12;
			break;
		case 1:
			sensor_FPS = 15;
			break;
		case 0:
			sensor_FPS = g_Fps_Num;
			break;
		default:
			return 0;
	}

	ret = IMP_ISP_Tuning_SetSensorFPS(sensor_FPS, 1);
	if (ret < 0)
	{
		Ucamera_ERR("failed to set sensor fps\n");
		return -1;
	}
	return 0;
}


int ucamera_load_config(struct Ucamera_Cfg *ucfg )
{
	int nframes = 0;
	unsigned int i, width, height, j;
	char key[64] = {0};
	char value[64] = {0};
	char *line_str = NULL;
	FILE *fp = NULL;
	struct Ucamera_Video_Cfg *vcfg;
	int intervals = 0;

	vcfg = &(ucfg->vcfg);
	if ((fp = fopen("/media/uvc.config", "r")) == NULL)
	{
		Ucamera_LOG("%s open config file failed!\n", __func__);
		return -1;
	}
	line_str = malloc(256*sizeof(char));

	while (!feof(fp))
	{
		if (fscanf(fp, "%[^\n]", line_str) < 0)
			break;
		fseek(fp , 1, SEEK_CUR);

		if (sscanf(line_str, "%[^:]:%[^\n]", key, value) != 2) 
		{
			fseek(fp , 1, SEEK_CUR);
			continue;
		}

		char *ch = strchr(key, ' ');
		if (ch) 
		{
			*ch = 0;
		}
		if (strcmp(key, "sensor_name") == 0)
		{
			strncpy(g_Sensor_Name, value, sizeof(g_Sensor_Name));
		}
		else if (strcmp(key, "i2c_addr") == 0) 
		{
			g_i2c_addr = htoi(value);
		} 
		else if (strcmp(key, "fps_num") == 0) 
		{
			g_Fps_Num = strToInt(value);
		} 
		else if (strcmp(key, "width") == 0) 
		{
			g_VideoWidth = strToInt(value);
		} 
		else if (strcmp(key, "height") == 0) 
		{
			g_VideoHeight = strToInt(value);
		} 
		else if (strcmp(key, "wdr_en") == 0) 
		{
			g_wdr = strToInt(value);
		} 
		else if (strcmp(key, "bitrate") == 0) 
		{
			g_BitRate = strToInt(value);
		} 
		else if (strcmp(key, "audio_en") == 0) 
		{
			g_Audio = strToInt(value);
			ucfg->audio_en = g_Audio;
		} 
		else if (strcmp(key, "gop") == 0) 
		{
			g_gop = strToInt(value);
		} 
		else if (strcmp(key, "qp_value") == 0) 
		{
			g_QP = strToInt(value);
		} 
		else if (strcmp(key, "adb_en") == 0) 
		{
			g_adb = strToInt(value);
			ucfg->adb_en = g_adb;
		} 
		else if (strcmp(key, "stillcap_en") == 0) 
		{
			ucfg->stillcap = strToInt(value);
		} 
		else if (strcmp(key, "rndis_en") == 0) 
		{
			g_rndis = strToInt(value);
		} 
		else if (strcmp(key, "dmic_en") == 0) 
		{
			g_dmic = strToInt(value);
		} 
		else if (strcmp(key, "speak_en") == 0) 
		{
			g_Speak = strToInt(value);
		} 
		else if (strcmp(key, "hid_en") == 0) 
		{
			ucfg->hid_en = strToInt(value);
		} 
		else if (strcmp(key, "h264_en") == 0) 
		{
			ucfg->h264_en = strToInt(value);
		} 
		else if (strcmp(key, "uvc_led") == 0) 
		{
			g_led = strToInt(value);
		} 
		else if (strcmp(key, "uvc_touch") == 0) 
		{
			g_touch = strToInt(value);
		} 
		else if (strcmp(key, "uvc_touch_old") == 0) 
		{
			g_touch_old = strToInt(value);
		} 
		else if (strcmp(key, "mic_volume") == 0) 
		{
			g_Volume = strToInt(value);
		} 
		else if (strcmp(key, "audio_ns") == 0) 
		{
			g_Audio_Ns = strToInt(value);
		} 
		else if (strcmp(key, "hvflip") == 0) 
		{
			gHVFlip = strToInt(value);
		} 
		else if (strcmp(key, "dynamic_fps") == 0) 
		{
			g_Dynamic_Fps = strToInt(value);
		} 
		else if (strcmp(key, "device_bcd") == 0) 
		{
			DEVICE_BCD = strToInt_Hex(value);
		} 
		else if (strcmp(key, "product_id") == 0) 
		{
			PRODUCT_ID = strToInt_Hex(value);
		} 
		else if (strcmp(key, "vendor_id") == 0) 
		{
			VENDOR_ID = strToInt_Hex(value);
		} 
		else if (strcmp(key, "serial_lab") == 0) 
		{
			strncpy(serial_label, self_parame_support, 18);
			strncat(serial_label, value, LABEL_LEN-17);
		} 
		else if (strcmp(key, "product_lab") == 0) 
		{
			strncpy(product_label, value, LABEL_LEN);
		} 
		else if (strcmp(key, "manufact_lab") == 0) 
		{
			strncpy(manufact_label, value, LABEL_LEN);
		} 
		else if (strcmp(key, "video_name") == 0) 
		{
			strncpy(video_device_name, value, LABEL_LEN);
		} 
		else if (strcmp(key, "audio_name") == 0) 
		{
			strncpy(audio_device_name, value, LABEL_LEN);
		} 
		else if (strcmp(key, "rcmode") == 0) 
		{
			if (strcmp(value, "vbr") == 0) 
			{
				g_RcMode = IMP_ENC_RC_MODE_VBR;
			} 
			else if (strcmp(value, "cbr") == 0) 
			{
				g_RcMode = IMP_ENC_RC_MODE_CBR;
			} 
			else if (strcmp(value, "fixqp") == 0) 
			{
				g_RcMode = IMP_ENC_RC_MODE_FIXQP;
			} 
			else if (strcmp(value, "cappedvbr") == 0) 
			{
				g_RcMode = IMP_ENC_RC_MODE_CAPPED_VBR;
			} 
			else {
				printf("Invalid RC method: %s\n", value);
			}
		} 
		else if (strcmp(key, "nframes") == 0) 
		{
			i = 0;
			j = 0;
			struct Ucamera_YUYV_Param *yuyvl;
			struct Ucamera_JPEG_Param *jpegl;
			struct Ucamera_H264_Param *h264l;
			nframes = strToInt(value);
			vcfg->yuyvnum = nframes;
			vcfg->jpegnum = nframes;
			vcfg->h264num = nframes;
			yuyvl = malloc(vcfg->yuyvnum*sizeof(struct Ucamera_YUYV_Param));
			jpegl = malloc(vcfg->jpegnum*sizeof(struct Ucamera_JPEG_Param));
			h264l = malloc(vcfg->h264num*sizeof(struct Ucamera_H264_Param));
			intervals = 10000000 / g_Fps_Num;
			ucfg->interval = intervals;
			while (i < nframes) 
			{
				if (fscanf(fp, "%[^\n]", line_str) < 0)
					break;
				sscanf(line_str, "{%d, %d}", &width, &height);

				if (width > 0 && height > 0 && (width%16 == 0))
				{
					yuyvl[i].width = width;
					yuyvl[i].height = height;
					yuyvl[i].fps_num = intervals;
					jpegl[i].width = width;
					jpegl[i].height = height;
					jpegl[i].fps_num = intervals;
					h264l[i].width = width;
					h264l[i].height = height;
					h264l[i].fps_num = intervals;
					if (yuyvl[i].height >= 1440)
					{
						j = i;
						vcfg->yuyvnum = nframes - 1;
					}
				} 
				else 
				{
					Ucamera_ERR("error(%s %d)Invalid width or height(%d %d)\n", __func__, __LINE__, width, height);
				}
				i++;
				fseek(fp , 1, SEEK_CUR);
			}
			if (vcfg->yuyvnum < nframes)
			{
				memcpy(yuyvl + j, yuyvl + j + 1, (nframes - j) * sizeof(struct Ucamera_YUYV_Param));
			}
			vcfg->yuyvlist = yuyvl;
			vcfg->jpeglist = jpegl;
			vcfg->h264list = h264l;
		} 
		else if (strcmp(key, "vcm_grade") == 0) 
		{
			st_jx.garde = strToInt(value);
			printf("===>> set vcm garde = %d <<===\r\n", st_jx.garde);
		} 
		else 
		{
			printf("Invalid config param: %s\n", key);
		}
	}
	free(line_str);
	return 0;
}

// TODO : YYL  对比4M的时候需要注意
#if(HIK_USB_PRODUCT == HIK_USB_OS04C10)
static char sensor_last[16] = "os04c10";
static int sensor_cutover(int format, int height, int interval)
{
	int ret = -1;
	char sensor_name_1[16] = "os04c10";
	char sensor_name_2[16] = "os04c2m";
	char sensor_name_3[16] = "os04cbi";

	if(format == V4L2_PIX_FMT_NV12 || format == V4L2_PIX_FMT_YUYV || interval <= 30){
		ret = strcmp(sensor_last, sensor_name_1);	
		if(ret != 0){
			g_VideoWidth = 2560;
			g_VideoHeight = 1440;
			sample_reload_sensor_init(sensor_last, sensor_name_1);				
			strcpy(sensor_last, sensor_name_1);
		}
	} else {
		if(height >= 960){
			ret = strcmp(sensor_last, sensor_name_2);	
			if(ret != 0){
				g_VideoWidth = 1920;
				g_VideoHeight = 1080;
				sample_reload_sensor_init(sensor_last, sensor_name_2);				
				strcpy(sensor_last, sensor_name_2);
			}
		} else {
			ret = strcmp(sensor_last, sensor_name_3);	
			if(ret != 0){
				g_VideoWidth = 1280;
				g_VideoHeight = 720;
				sample_reload_sensor_init(sensor_last, sensor_name_3);				
				strcpy(sensor_last, sensor_name_3);
			}
		}
	}
	return 0;
}
#endif

static int uvc_event_process(int event_id, void *data)
{
	/**
	 * 如果是第一次出30/25fps的，则先出一次1080P60fps的
	 */
	int retry_cnt = 0;
	int interval_cur = 0;

#if(HIK_USB_PRODUCT == HIK_USB_OS02D20)
	char sensor_name_1[16] = "os04c10";
	char sensor_name_2[16] = "os04c2m";
#endif

	switch (event_id) 
	{
	case UCAMERA_EVENT_STREAMON: 
	{
		// zoom.data[UVC_CUR] = 100;
		struct Ucamera_Video_Frame *frame = (struct Ucamera_Video_Frame *) data;
		gStreaming = 1;
		interval_cur = 10000000 / frame->interval;
#if (LED_CTRL_PIN == GPIO_CTRL_LED)
		if (g_led) 
		{
			sample_ucamera_gpio_ctl(g_led, 1);
		}
#else
	led_on(0, DUTY);
#endif
imp_init_check:
		if (!imp_inited) 
		{
			if (retry_cnt++ > 40) 
			{
				Ucamera_LOG("[error]imp system init failed.\n");
				return 0;
			}
			Ucamera_LOG("imp sys not ready, wait and retry:%d", retry_cnt);
			usleep(50*1000);
			goto imp_init_check;
		}

#if(HIK_USB_PRODUCT == HIK_USB_OS02D20)
		// 2M 流程
		if(g_VideoHeight > 1080 && frame->height <= 1080){
			sample_reload_sensor_init(sensor_name_1, sensor_name_2);				
			g_VideoWidth = 1920;
			g_VideoHeight = 1080;
		}
		if(g_VideoHeight <= 1080 && frame->height > 1080){
			sample_reload_sensor_init(sensor_name_2, sensor_name_1);				
			g_VideoWidth = 2560;
			g_VideoHeight = 1440;
		}
#elif(HIK_USB_PRODUCT == HIK_USB_OS04C10)
		// 4M流程
		sensor_cutover(frame->fcc, frame->height, interval_cur);
#endif

		g_Fps_Num = interval_cur;
		gFps = 10000000 / frame->interval;
		gWidth = frame->width;
		gHeight = frame->height;
		imp_sdk_init(frame->fcc, frame->width, frame->height);
		sample_get_stream_process_init(frame->fcc);
		IMPISPAFHist af_hist = {0};
		int index = 0;
		// 校验获取AF相关是否成功，若不成功则重新初始化ISP
		while((IMP_ISP_Tuning_GetAfHist(&af_hist) < 0) && index < 5)
		{
			index++;
			usleep(index*100*10000);
			if(IMP_ISP_EnableTuning() < 0)
			{
				printf("=====>> IMP_ISP_EnableTuning failed <<=====\n");
			}
		}
		// 图像参数在开图前重新配置
		isp_video_init();
		return start_auto_focus();
	}
	case UCAMERA_EVENT_STREAMOFF: {
		struct Ucamera_Video_Frame *frame = (struct Ucamera_Video_Frame *) data;
		gStreaming = 0;
	
#if (LED_CTRL_PIN == GPIO_CTRL_LED)
		if (g_led) 
		{
			sample_ucamera_gpio_ctl(g_led, 0);
		}
#else
		led_off(0);
#endif
		gFps = 0;
		gWidth = 0;
		gHeight = 0;
		// TODO，关闭自动聚焦和AF
		stop_auto_focus();
		sample_get_stream_process_deinit();
		return imp_sdk_deinit(frame->fcc);
	}

	default:
		Ucamera_LOG("%s(ERROR): unknown message ->[%d]!\n", TAG, event_id);
		return -1;
	};

	return 0;
}

FILE *pcm_fd = NULL;
FILE *ref_fd = NULL;
short ref_pcm[640] = {0};
int sample_audio_pcm_get(short *pcm)
{
	int len;
	len = sample_audio_amic_pcm_get(ref_pcm);
	len = sample_audio_dmic_pcm_get(pcm);
	return len;
}

void signal_handler(int signum) 
{
	Ucamera_LOG("catch signal %d\n", signum);
	if (signum == SIGUSR1)
	{
		sem_post(&ucam_ready_sem);
	}
	else 
	{
    	sample_deprepare_pal_buf();
	    Ucamera_LOG("Ucamera Exit Now. \n");
		sample_audio_amic_exit();
		if (g_dmic)
			sample_audio_dmic_exit();
		imp_system_exit();
		Ucamera_DeInit();
	}
}

void *ucam_impsdk_init_entry(void *res)
{
	sem_wait(&ucam_ready_sem);

	sample_system_init();

	imp_inited = 1;
	if (g_Audio == 1) {
		if (g_dmic)
			sample_audio_dmic_init();
		sample_audio_amic_init();

		int ret = 0;
		IMPAudioIOAttr attr  = {0};
		IMPAudioAgcConfig agcConfig = {0};
		attr.samplerate = AUDIO_SAMPLE_RATE_16000;
		attr.bitwidth = AUDIO_BIT_WIDTH_16;
		attr.soundmode = AUDIO_SOUND_MODE_MONO;
		attr.frmNum = 4;
		attr.numPerFrm = 960;
		attr.chnCnt = 1;
		agcConfig.TargetLevelDbfs = 9;
		agcConfig.CompressionGaindB = 20;
		// TODO : YYL
		// E 系列上使用了高通滤波，需黄工确认一下直播上是否也需要
		ret = IMP_AI_SetHpfCoFrequency(250);
		ret = IMP_AI_EnableHpf(&attr);
		ret = IMP_AI_EnableAgc(&attr, agcConfig);
		Ucamera_Audio_Start();
	}

	if (g_Speak)
		sample_audio_play_start();
	ISP_Create(&g_isp_object.create_param,&g_isp_object.handle);
	return NULL;
}

/* ---------------------------------------------------------------------------
 * main
 */
static void usb_uart_init(void)
{
#if 1
	int fd= -1;
	fflush(stdout);
	setvbuf(stdout,NULL,_IONBF,0);
	fd = open("/dev/ttyGS0",O_RDWR | O_NDELAY);
	if(fd < 0) {
		printf("open /dev/ttyGS0 failed\n");
		return;
	}
	dup2(fd,0);
	dup2(fd,1);
	dup2(fd,2);
	close(fd);
#endif
}

int main(int argc, char *argv[])
{
	signal(SIGUSR1, signal_handler); /*set signal handler*/
	sem_init(&ucam_ready_sem, 0, 0);
	int ret;

	struct Ucamera_Cfg ucfg = {0};
	struct Ucamera_Video_CB_Func v_func;
	struct Ucamera_Audio_CB_Func a_func;
	pthread_t ucam_impsdk_init_id;

	signal(SIGKILL, signal_handler); /*set signal handler*/
	signal(SIGTERM, signal_handler); /*set signal handler*/
#if 0
	if (AntiCopy_Verify())
	{
		Ucamera_ERR("AntiCopy Verified failed!!!\n");
		return 0;
	}
#endif
     sample_prepare_pal_buf();
	if (ucamera_load_config(&ucfg)) {
		Ucamera_ERR("ucamera load config failed!\n");
		return 0;
	}

	if (ucamera_uvc_pu_attr_load()) {
		Ucamera_ERR("[ERROR] load uvc PU attr failed.\n");
		return 0;
	}
	g_VcmValue = read_vcm_value();
	
	if (g_Speak)
	{
		ucfg.pcfg.Pid = PRODUCT_ID + 1;
	}
	else
	{
		ucfg.pcfg.Pid = PRODUCT_ID;
	}
	ucfg.pcfg.Vid = VENDOR_ID;
	ucfg.pcfg.version = DEVICE_BCD;
	strcpy(ucfg.pcfg.manufacturer, manufact_label);
	strcpy(ucfg.pcfg.product, product_label);
	strcpy(ucfg.pcfg.serial, serial_label);
	strcpy(ucfg.pcfg.video_name, video_device_name);
	strcpy(ucfg.pcfg.audio_name, audio_device_name);
	ucfg.acfg.mic_volume = 0x0600;
	ucfg.acfg.spk_volume = 0x0;
	if (g_Speak)
		ucfg.acfg.speak_enable = 1;
	else
		ucfg.acfg.speak_enable = 0;

	Ucamera_Config(&ucfg);
	Ucamera_Init(UVC_BUF_NUM, UVC_BUF_SIZE);

	v_func.get_YuvFrame = sample_get_yuv_snap;
	v_func.get_Nv12Frame = sample_get_nv12_snap;
	v_func.get_JpegFrame = sample_get_jpeg_snap;
	v_func.get_H264Frame = sample_get_h264_snap;
	Ucamera_Video_Regesit_CB(&v_func);

	backlight_compens.set = sample_video_backlight_compens_set;
	backlight_compens.get = sample_video_backlight_compens_get;
	brightness.set = sample_video_brightness_set;
	brightness.get = sample_video_brightness_get;
	contrast.set = sample_video_contrast_set;
	contrast.get = sample_video_contrast_get;
	saturation.set = sample_video_saturation_set;
	saturation.get = sample_video_saturation_get;
	sharpness.set = sample_video_sharpness_set;
	sharpness.get = sample_video_sharpness_get;
	hue.set = sample_video_hue_set;
	hue.get = sample_video_hue_get;
	whitebalance.set = sample_video_whitebalance_set;
	whitebalance.get = sample_video_whitebalance_get;
	whitebalance_auto.set = sample_video_whitebalance_auto_set;
	whitebalance_auto.get = sample_video_whitebalance_auto_get;
	powerlinefreq.set = sample_video_powerlinefreq_set;
	powerlinefreq.get = sample_video_powerlinefreq_get;
	pu_gamma.set = sample_video_gamma_set;
	pu_gamma.get = sample_video_gamma_get;
	gain.set = sample_video_gain_set;
	gain.get = sample_video_gain_get;
	focus.set = sample_video_focus_set;
	focus.get = sample_video_focus_get;
	focus_auto.set = sample_video_focus_auto_set;
	focus_auto.get = sample_video_focus_auto_get;
	exposure_time.set = sample_video_exposure_time_set;
	exposure_time.get = sample_video_exposure_time_get;
	auto_exposure_mode.set = sample_video_auto_exposure_mode_set;
	auto_exposure_mode.get = sample_video_auto_exposure_mode_get;
	zoom.set = sample_video_zoom_set;
	zoom.get = sample_video_zoom_get;
	roll.set = sample_video_roll_set;
	roll.get = sample_video_roll_get;

	Ucamera_Video_Regesit_Process_Unit_CB(Pu_ctrl);
	Ucamera_Video_Regesit_Camera_Terminal_CB(Ct_ctrl);
	register_xu_cb();

	UCamera_Registe_Event_Process_CB(uvc_event_process);
	//2020.08.24 ADD by LHY
	ISP_GetMemSize(&g_isp_object.create_param);
	printf("ISP_GetMemSize  =  %d  \n",g_isp_object.create_param.buf_size);
	g_isp_object.create_param.buf = malloc(g_isp_object.create_param.buf_size);
	if (g_isp_object.create_param.buf == NULL)
	{
		printf("g_isp_object_Mem_Alloc failed\n");
		return ISP_LIB_S_FAIL;
	}
	else
	{
		memset(g_isp_object.create_param.buf,0,g_isp_object.create_param.buf_size);
	}
	UCamera_Video_Start();
	if (g_Audio == 1) 
	{
		if (g_dmic)
		{
			a_func.get_AudioPcm = sample_audio_dmic_pcm_get;
		}
		else
		{
			a_func.get_AudioPcm = sample_audio_amic_pcm_get;
		}
		a_func.set_Mic_Volume = sample_set_mic_volume;
		a_func.set_Spk_Volume = sample_set_spk_volume;
		a_func.set_Mic_Mute = sample_set_mic_mute;
		a_func.set_Spk_Mute = sample_set_spk_mute;
		Ucamera_Audio_Regesit_CB(&a_func);
	}
#if (LED_CTRL_PIN == GPIO_CTRL_LED)
	if (g_led) {
		sample_ucamera_gpio_init(g_led, 0);
		sample_ucamera_gpio_ctl(g_led, 0);
	}
#else
	SU_PWM_Init();
	led_twinkle(2, 0);
#endif
	ret = pthread_create(&ucam_impsdk_init_id, NULL, ucam_impsdk_init_entry, NULL);
	if (ret != 0) 
	{
		Ucamera_LOG("pthread_create failed \n");
		return -1;
	}
	
	int flag = 0;
	while (1) 
	{
		if (g_Dynamic_Fps)
		{
			imp_SensorFPS_Adapture();
		}
		usleep(1000*1000*2);
		/*UART Change to usb-com*/

#if 1
		if(!access("/dev/ttyGS0",F_OK)) {
			if(!flag) {
				printf("freopen !\n");
				usb_uart_init();
				flag = 1;
			}
			if(flag)
			{
				sleep(1);
			}	
		} else {
				printf("please wait minitue\n");
				sleep(1);
		}
#endif
	}
	return 0;
}

int start_auto_focus()
{
	int ret = 0;
	int tof_status = 0;
	pthread_attr_t attr;
#if (1 == STM_DTOF_EN)
	if((0 == tof_init) && (0 == Tof_Init()))
	{
		tof_init = 1;
		Ucamera_LOG("===>>>tof init success<<<====\n");
	}
#endif
	if(0 == g_VcmValue)
	{
		Ucamera_LOG("vcm not check, set vcm to initial position : %d", gVCMInitPot);
		ret = set_value_ctl_vcm(gVCMInitPot);
		if(ret < 0)
		{
			Ucamera_ERR("vcm ctl err");
		}
		return 0;
	}
	if(0 == focus_auto.data[UVC_CUR])
	{
		ret = set_value_ctl_vcm(3*focus.data[UVC_CUR]);
		if(ret < 0)
		{
			Ucamera_ERR("vcm ctl err");
		}
		return 0;
	}
	if(0 == gJXRuning)
	{	
		af_thread_cancel = 0;
		// TODO 根据返回值，给机芯传递tof的状态
		st_jx.nearCoord = (g_VcmValue == 0) ? 0 : g_VcmValue + 70;
		st_jx.calibrationCoord = g_VcmValue;
		st_jx.farCoord = (g_VcmValue - 22 > 0) ? g_VcmValue - 22 : 0;
		st_jx.slope = 3.52;
		st_jx.tof1_status = 0;
		st_jx.tof2_status = 0;
		st_jx.garde = af_grade[gAFGarde];
#if (1 == STM_DTOF_EN)
		tof_status = Tof_Start();
		if (tof_status > -2)
		{
			st_jx.tof2_status = 1;
		}
		if(tof_status > -1)
		{
			st_jx.tof1_status = 1;
		}
#endif
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		ret = pthread_create(&pt_jx, &attr, autoFocus, (void*)&st_jx);
		if (ret != 0) 
		{
			Ucamera_ERR("create jx_thread error:%d", __LINE__);
			return -1;
		}
		Ucamera_LOG("start auto foucus");
		gJXRuning = 1;
		// 根据机芯需求，开始聚焦前，将vcm置于初始位置
		set_value_ctl_vcm(gVCMInitPot);
	}
	else
	{
		Ucamera_ERR("jx_thread is runing");
	}
	return 0;
}

int stop_auto_focus()
{
	if(1 == gJXRuning)
	{
		// 将取消机芯线程的标志置为1，通知机芯要结束自动聚焦
		af_thread_cancel = 1;
		pthread_cancel(pt_jx);
#if (1 == STM_DTOF_EN)
		Tof_Stop();
#endif
		Ucamera_LOG("stop auto foucus");
		gJXRuning = 0;
	}
	else
	{
		Ucamera_ERR("jx_thread is stoped");
	}
}

int sample_video_isOnStream(void)
{
	return gStreaming;
}


int AF_GetResolvFPS(int chan, struct whf_typedef * whf)
{
#if(HIK_USB_PRODUCT == HIK_USB_OS02D20)
	if(sample_video_isOnStream())
	{	
		if(2560 == gWidth && 1440 == gHeight)
		{
			whf->sensorW = 2560;
			whf->sensorH = 1440;
			whf->sensorF = gFps;
			whf->ispW = 2560;
			whf->ispH = 1440;
			whf->cropX = 0;
			whf->cropY = 0;
			whf->outputW = gWidth;
			whf->outputH = gHeight;
			whf->productType = HIK_USB_OS04C10;
		}
		else
		{
			whf->sensorW = 1920;
			whf->sensorH = 1080;
			whf->sensorF = gFps;
			if(480 == gHeight || 960 == gHeight)
			{
				whf->ispW = 1440;
				whf->cropX = 240;
			}
			else
			{
				whf->ispW = 1920;
				whf->cropX = 0;
			}
			whf->ispH = 1080;
			whf->cropY = 0;
			whf->outputW = gWidth;
			whf->outputH = gHeight;
			whf->productType = HIK_USB_OS02D20;
		}
		
	}
	else
	{
		whf->sensorW = 0;
		whf->sensorH = 0;
		whf->sensorF = 0;
		whf->ispW = 0;
		whf->ispH = 0;
		whf->cropX = 0;
		whf->cropY = 0;
		whf->outputW = 0;
		whf->outputH = 0;
		whf->productType = 0;
	}
#elif(HIK_USB_PRODUCT == HIK_USB_OS04C10)
	if(sample_video_isOnStream())
	{	
		if(HIK_USB_OS04C10 == 2)
		{
			whf->productType = HIK_USB_OS04C10;
			// 帧率为30帧、25帧、1280*720、640*480
			if(gFps < 50 || gHeight < 960)
			{
				whf->sensorW = 2560;
				whf->sensorH = 1440;
				whf->sensorF = gFps;
				whf->outputW = gWidth;
				whf->outputH = gHeight;
				whf->ispH = 1440;		
				whf->cropY = 0;
				if(480 == gHeight || 960 == gHeight)
				{
					whf->ispW = 1920;
					whf->cropX = 320;
				}
				else
				{
					whf->ispW = 2560;
					whf->cropX = 0;
				}
			}
			// 1920*1080和1280*960的50 60fps
			else
			{
				whf->sensorW = 1920;
				whf->sensorH = 1080;
				whf->sensorF = gFps;
				whf->outputW = gWidth;
				whf->outputH = gHeight;
				whf->ispH = 1080;		
				whf->cropY = 0;
				if(480 == gHeight || 960 == gHeight)
				{
					whf->ispW = 1440;
					whf->cropX = 240;
				}
				else
				{
					whf->ispW = 1920;
					whf->cropX = 0;
				}
			}
		}
		else
		{
			whf->sensorW = 0;
			whf->sensorH = 0;
			whf->sensorF = 0;
			whf->ispW = 0;
			whf->ispH = 0;
			whf->cropX = 0;
			whf->cropY = 0;
			whf->outputW = 0;
			whf->outputH = 0;
			whf->productType = 0;
		}
	}
	else
	{
		whf->sensorW = 0;
		whf->sensorH = 0;
		whf->sensorF = 0;
		whf->ispW = 0;
		whf->ispH = 0;
		whf->cropX = 0;
		whf->cropY = 0;
		whf->outputW = 0;
		whf->outputH = 0;
		whf->productType = 0;
	}
#endif
	return 0;
}
