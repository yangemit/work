#ifndef __HIK_CONFIG_H__
#define __HIK_CONFIG_H__
#include "ucam/usbcamera.h"

// HIK私有属性变量
#define SELF_PARAME_BASE		128
#define SELF_PARAME_WIDEDY		SELF_PARAME_BASE+0	// 宽动态
#define SELF_PARAME_NOISE		SELF_PARAME_BASE+1	// 降噪
#define SELF_PARAME_FOUCE		SELF_PARAME_BASE+2	// 聚焦
#define SELF_PARAME_EC			SELF_PARAME_BASE+3	// 回声消除
#define SELF_PARAME_ONEKEYWH	SELF_PARAME_BASE+4	// 一键白平衡
#define SELF_PARAME_IMAGE		SELF_PARAME_BASE+5	// 镜像
#define SELF_PARAME_CHECKAF		SELF_PARAME_BASE+6	// VCM标定
#define SELF_PARAME_FASTSHUT	SELF_PARAME_BASE+7	// 快门时间
#define SELF_PARAME_CHECKTOF	SELF_PARAME_BASE+8	// TOF检测
#define SELF_PARAME_IMAGEMODE	SELF_PARAME_BASE+9	// 图像模式
#define SELF_PARAME_CHECKPWM	SELF_PARAME_BASE+10	// PWM检测
#define SELF_PARAME_FACEAE		SELF_PARAME_BASE+11	// face ae 0-关闭 1-开启 默认0
#define SELF_PARAME_LOGO		SELF_PARAME_BASE+12	// 水印开关 0-关闭 1-开启 默认0
#define SELF_PARAME_COLORRANGE	SELF_PARAME_BASE+13	// 色域范围 0-127 默认0
#define SELF_PARAME_RESETCHN1	SELF_PARAME_BASE+14	// 重置通道1，重置一键白平衡、色域范围、图像模式和宽动态
#define SELF_PARAME_RESETCHN2	SELF_PARAME_BASE+15	// 重置通道2，重置降噪、faceae、回声消除、水印
#define SELF_PARAME_CHECKITOF	SELF_PARAME_BASE+16	// ITOF相关
#define SELF_PARAME_CHOOSEPN	SELF_PARAME_BASE+17	// P/N制切换
#define SELF_PARAME_LENSCAL	    SELF_PARAME_BASE+18	// 镜头曲线校验 0-初始化，2-开始校验，4-保存曲线，9-置老化测试标志，10-获取Pan老化率，11-获取Titl老化率
#define SELF_PARAME_AFGRADE     SELF_PARAME_BASE+19	// 聚焦灵敏度等级0-4

struct uvc_pu_string {
	char id;
	const char *s;
};

static struct uvc_pu_string pu_string[] = {
	{0, "null"},
	{UVC_BACKLIGHT_COMPENSATION_CONTROL, "backlight"},
	{UVC_BRIGHTNESS_CONTROL, "brightness"},
	{UVC_CONTRAST_CONTROL, "contrast"},
	{UVC_GAIN_CONTROL, "gain"},
	{UVC_POWER_LINE_FREQUENCY_CONTROL, "powerline_freq"},
	{UVC_HUE_CONTROL, "hue"},
	{UVC_SATURATION_CONTROL, "saturation"},
	{UVC_SHARPNESS_CONTROL, "sharpness"},
	{UVC_GAMMA_CONTROL, "pu_gamma"},
	{UVC_WHITE_BALANCE_TEMPERATURE_CONTROL, "white_balance"},
	{UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL, "white_balance_auto"},
	{UVC_FAST_SHUTTER, "fast_shutter"},
	{UVC_HVFLIP_CONTROL, "hvflip"},
	{0xe, "null"},
	{UVC_S_EXPOSURE_TIME_CONTROL, "exposure"},
	{UVC_S_AUTO_EXPOSURE_MODE_CONTROL, "exposure_auto"},
	{UVC_S_FOCUS_CONTROL, "focus"},
	{UVC_S_FOCUS_AUTO_CONTROL, "focus_auto"},
	{UVC_S_DENOISE_CONTROL, "denoise"},
	{UVC_S_ZOOM_ABSOLUTE_CONTROL, "zoom"},
	{UVC_S_IMAGEMODE_CONTROL, "image_mode"},
	{UVC_S_STANDARD_CONTROL, "video_standard"},
	{UVC_S_AFGRADE_CONTROL, "af_grade"}
};

#define LABEL_LEN 64

// 图像参数映射
#define SAT_SPAN_DOWN	64
#define SAT_SPAN_UP	32
#define BRIGTNESS_SPAN_DOWN	88
#define BRIGHTNESS_SPAN_UP	48
#define SHARPNESS_SPAN_DOWN	48
#define SHARPNESS_SPAN_UP	20

extern int Ucam_Stream_On;
extern int gFcc;
extern int gWidth;
extern int gHeight;

// LED 的控制，PWM或者GPIO
#define GPIO_CTRL_LED   1
#define PWM_CTRL_LED    2

// VCM 类型
#define VCM_TYPE_BU     0
#define VCM_TYPE_AW     1
#define VCM_TYPE VCM_TYPE_BU

#define TV22_2K809				0x0100
#define TV24_2K810				0x0200
#define U22_2K815				0x0300
#define U24_2K816				0x0400
#define UC2_2K819				0x0500
#define UC4_2K820				0x0600
#define E12A_2K822				0x0700
#define E14A_2K823				0x0800
#define CAMERA_MODEL			E12A_2K822

#if(CAMERA_MODEL == TV22_2K809)
#define TIME2LINE(exptime,vts,fps)  MAX((5),(((exptime)*(vts)) / (fps)))
// 私有属性及加密信息头
static char self_parame_support[17] = "46435000_P030100_";
// 机芯灵敏度等级
static int af_grade[5] = {1, 6, 13, 16, 19};
// LED的控制方式
#define LED_CTRL_PIN    GPIO_CTRL_LED
// STM 的DTOF开关
#define STM_DTOF_EN     0
#elif(CAMERA_MODEL == TV24_2K810)
#define TIME2LINE(exptime,vts,fps)  MAX((16),(((exptime)*(vts)) / (fps)))
// 私有属性及加密信息头
static char self_parame_support[17] = "46435000_P020300_";
// 机芯灵敏度等级
static int af_grade[5] = {1, 6, 13, 16, 19};
// LED的控制方式
#define LED_CTRL_PIN    GPIO_CTRL_LED
// STM 的DTOF开关
#define STM_DTOF_EN     0
#elif(CAMERA_MODEL == U22_2K815)
#define TIME2LINE(exptime,vts,fps)  MAX((5),(((exptime)*(vts)) / (fps)))
// 私有属性及加密信息头
static char self_parame_support[17] = "46C35000_P030100_";
// 机芯灵敏度等级
static int af_grade[5] = {1, 6, 11, 16, 19};
// LED的控制方式
#define LED_CTRL_PIN    GPIO_CTRL_LED
// STM 的DTOF开关
#define STM_DTOF_EN     1
#elif(CAMERA_MODEL == U24_2K816)
#define TIME2LINE(exptime,vts,fps)  MAX((16),(((exptime)*(vts)) / (fps)))
// 私有属性及加密信息头
static char self_parame_support[17] = "46C35000_P020300_";
// 机芯灵敏度等级
static int af_grade[5] = {1, 6, 11, 16, 19};
// LED的控制方式
#define LED_CTRL_PIN    GPIO_CTRL_LED
// STM 的DTOF开关
#define STM_DTOF_EN     0
#elif(CAMERA_MODEL == UC2_2K819)
#define TIME2LINE(exptime,vts,fps)  MAX((5),(((exptime)*(vts)) / (fps)))
// 私有属性及加密信息头
static char self_parame_support[17] = "46435000_P030100_";
// 机芯灵敏度等级
static int af_grade[5] = {1, 6, 13, 16, 19};
// LED的控制方式
#define LED_CTRL_PIN    PWM_CTRL_LED
// STM 的DTOF开关
#define STM_DTOF_EN     0
#elif(CAMERA_MODEL == UC4_2K820)
#define TIME2LINE(exptime,vts,fps)  MAX((16),(((exptime)*(vts)) / (fps)))
// 私有属性及加密信息头
static char self_parame_support[17] = "46435000_P030300_";
// 机芯灵敏度等级
static int af_grade[5] = {1, 6, 13, 16, 19};
// LED的控制方式
#define LED_CTRL_PIN    PWM_CTRL_LED
// STM 的DTOF开关
#define STM_DTOF_EN     0
#elif(CAMERA_MODEL == E12A_2K822)
#define TIME2LINE(exptime,vts,fps)  MAX((5),(((exptime)*(vts)) / (fps)))
#if (VCM_TYPE ==  VCM_TYPE_AW)
// 私有属性及加密信息头（艾为的VCM芯片）
static char self_parame_support[17] = "46435000_P030101_";
#else
// 私有属性及加密信息头（罗姆的VCM芯片）
static char self_parame_support[17] = "46431000_P030100_"; // bit[16:19]=0001,关闭PN制
#endif
// 机芯灵敏度等级
static int af_grade[5] = {1, 6, 13, 16, 19};
// LED的控制方式
#define LED_CTRL_PIN    PWM_CTRL_LED
// STM 的DTOF开关
#define STM_DTOF_EN     0
#elif(CAMERA_MODEL == E14A_2K823)
#define TIME2LINE(exptime,vts,fps)  MAX((16),(((exptime)*(vts)) / (fps)))
// 私有属性及加密信息头
// static char self_parame_support[17] = "46435000_P020300_";
// T31ZX替换成T31L
static char self_parame_support[17] = "46435000_P030300_";
// 机芯灵敏度等级
static int af_grade[5] = {1, 6, 13, 16, 19};
// LED的控制方式
#define LED_CTRL_PIN    PWM_CTRL_LED
// STM 的DTOF开关
#define STM_DTOF_EN     0
#else
#define TIME2LINE(exptime,vts,fps)  MAX((16),(((exptime)*(vts)) / (fps)))
// 私有属性及加密信息头
static char self_parame_support[17] = "46C35000_P020300_";
// 机芯灵敏度等级
static int af_grade[5] = {1, 6, 11, 16, 19};
// LED的控制方式
#define LED_CTRL_PIN    PWM_CTRL_LED
// STM 的DTOF开关
#define STM_DTOF_EN     0
#endif


#endif
