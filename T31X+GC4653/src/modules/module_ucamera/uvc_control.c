/*
 * uvc_control.c
 *
 * Copyright (C) 2022 Ingenic Semiconductor Co.,Ltd
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <semaphore.h>

#include <usbcamera.h>
#include <imp-common.h>
#include "uvc_control.h"
#include <global_config.h>

#define MODULE_TAG "UVC_CONTROL"

#define GET_CUR			0x81
#define GET_LEN			0x85

//osd句柄
static IMPRgnHandle *prHander = NULL;
static int frame_fmt = -1;
int osd_width = 0;
int osd_height = 0;
int frame_stream_switch = 0;

IMPRgnHandle *prHander_interface(void)
{
	return prHander;
}
int get_frame_fmt(void)
{
		return frame_fmt;
}
int get_osd_width(void)
{
		return osd_width;
}

int get_osd_height(void)
{
		return osd_height;
}

int get_frame_switch(void){
	return frame_stream_switch;
}

typedef struct _uvc_control {
	uint8_t hvflip;
	uint8_t stream_on;
	uint8_t imp_inited;
	uint8_t dynamic_fps;
	uint32_t sensor_fps;
	uint32_t sensor_width;
	uint32_t sensor_height;
	pthread_mutex_t uvc_attr_mutex;
	FILE *uvc_attr_fd;
	sem_t ucam_fs_sem;
	uint8_t bcsh_en;
	uint8_t bcsh_low;
	uint8_t bcsh_high;

} uvc_control_t;

static uvc_control_t uvc_ctx;

static struct Ucamera_Video_CB_Func v_func;
extern struct chn_conf chn[];

IMPISPGamma gamma_cur;
int getGammaOnlyOnce = 1;

static int ucamera_uvc_pu_attr_save(int type, int value);


struct uvc_attr_string {
	char id;
	const char *s;
};

/*
 * UVC.attr config
 */
struct uvc_attr_string pu_string[] = {
	{0, "null"},
	{UVC_BACKLIGHT_COMPENSATION_CONTROL, "backlight"},
	{UVC_BRIGHTNESS_CONTROL, "brightness"},
	{UVC_CONTRAST_CONTROL, "contrast"},
	{UVC_GAIN_CONTROL, "gain"},
	{UVC_POWER_LINE_FREQUENCY_CONTROL, "powerline_freq"},
	{UVC_HUE_CONTROL, "hue"},
	{UVC_SATURATION_CONTROL, "saturation"},
	{UVC_SHARPNESS_CONTROL, "sharpness"},
	{UVC_GAMMA_CONTROL, "gamma"},
	{UVC_WHITE_BALANCE_TEMPERATURE_CONTROL, "white_balance"},
	{UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL, "white_balance_auto"}
};



/*
 * Set UVC Process Uinit
 *
 **/

/*
 * cs : UVC_BACKLIGHT_COMPENSATION_CONTROL
 * set:
 * get:
 */
static struct Ucamera_Video_PU_Control backlight_compens = {
	.type = UVC_BACKLIGHT_COMPENSATION_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 4,
	.data[UVC_DEF] = 0,
	.data[UVC_CUR] = 0,
};

static int sample_video_backlight_compens_set(int value)
{
	int ret;
	if (value < 0 || value > 10) {
		printf("WARNNING(%s): %s failed \n", MODULE_TAG, __func__);
		return -1;
	}
	ret = IMP_ISP_Tuning_SetHiLightDepress(value);
	if (ret)
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
	backlight_compens.data[UVC_CUR] = value;
	ucamera_uvc_pu_attr_save(UVC_BACKLIGHT_COMPENSATION_CONTROL, value);

	return 0;
}

static int sample_video_backlight_compens_get(void)
{
	uint32_t value = 0;

	if (!uvc_ctx.stream_on)
		return backlight_compens.data[UVC_CUR];
	int ret = -1;
	ret = IMP_ISP_Tuning_GetHiLightDepress(&value);
	if (ret) {
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
	}
	return value;
}

/*
 * cs : UVC_BRIGHTNESS_CONTROL
 * set:
 * get:
 */
static struct Ucamera_Video_PU_Control brightness = {
	.type = UVC_BRIGHTNESS_CONTROL,
	.data[UVC_MIN] = 50,
	.data[UVC_MAX] = 160,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static int sample_video_brightness_set(int value)
{
	int ret;
	unsigned char bright = 0;

	bright = value & 0xff;
	ret = IMP_ISP_Tuning_SetBrightness(bright);
	if (ret)
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
	brightness.data[UVC_CUR] = bright;
	ucamera_uvc_pu_attr_save(UVC_BRIGHTNESS_CONTROL, bright);
	return 0;
}

static int sample_video_brightness_get(void)
{
	int ret;
	unsigned char bright = 0;

	if (!uvc_ctx.stream_on)
		return brightness.data[UVC_CUR];

	ret = IMP_ISP_Tuning_GetBrightness(&bright);
	if (ret) {
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
		bright = 128;
	}
	return bright;
}

/*
 * cs : UVC_CONTRAST_CONTROL
 * set:
 * get:
 */
static struct Ucamera_Video_PU_Control contrast = {
	.type = UVC_CONTRAST_CONTROL,
	.data[UVC_MIN] = 50,
	.data[UVC_MAX] = 160,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static int sample_video_contrast_set(int value)
{
	int ret;
	unsigned char tmp = 0;

	tmp = value & 0xff;
	ret = IMP_ISP_Tuning_SetContrast(tmp);
	if (ret)
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);

	contrast.data[UVC_CUR] = tmp;
	ucamera_uvc_pu_attr_save(UVC_CONTRAST_CONTROL, tmp);
	return 0;
}

static int sample_video_contrast_get(void)
{
	int ret;
	unsigned char cost = 0;

	if (!uvc_ctx.stream_on)
	return contrast.data[UVC_CUR];

	ret = IMP_ISP_Tuning_GetContrast(&cost);
	if (ret) {
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
		cost = 128;
	}
	return cost;
}

/*
 * cs : UVC_SATURATION_CONTROL
 * set:
 * get:
 */
static struct Ucamera_Video_PU_Control saturation = {
	.type = UVC_SATURATION_CONTROL,
	.data[UVC_MIN] = 50,
	.data[UVC_MAX] = 160,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static int sample_video_saturation_set(int value)
{
	int ret;
	unsigned char tmp = 0;
	tmp = value & 0xff;

	ret = IMP_ISP_Tuning_SetSaturation(tmp);
	if (ret)
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
	saturation.data[UVC_CUR] = tmp;
	ucamera_uvc_pu_attr_save(UVC_SATURATION_CONTROL, tmp);
	return 0;
}

static int sample_video_saturation_get(void)
{
	int ret;
	unsigned char tmp = 0;

	if (!uvc_ctx.stream_on)
		return saturation.data[UVC_CUR];

	ret = IMP_ISP_Tuning_GetSaturation(&tmp);
	if (ret) {
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
		tmp = 128;
	}
	return tmp;
}

/*
 * cs : UVC_SHARPNESS_CONTROL
 * set:
 * get:
 */
static struct Ucamera_Video_PU_Control sharpness = {
	.type = UVC_SHARPNESS_CONTROL,
	.data[UVC_MIN] = 50,
	.data[UVC_MAX] = 160,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static int sample_video_sharpness_set(int value)
{
	int ret;
	unsigned char tmp = 0;

	tmp = value & 0xff;
	ret = IMP_ISP_Tuning_SetSharpness(tmp);
	if (ret)
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
	sharpness.data[UVC_CUR] = tmp;
	ucamera_uvc_pu_attr_save(UVC_SHARPNESS_CONTROL, tmp);
	return 0;
}

static int sample_video_sharpness_get(void)
{
	int ret;
	unsigned char tmp = 0;

	if (!uvc_ctx.stream_on)
		return sharpness.data[UVC_CUR];

	ret = IMP_ISP_Tuning_GetSharpness(&tmp);
	if (ret) {
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
		tmp = 128;
	}
	return tmp;
}

/*
 * cs : UVC_GAMMA_CONTROL
 * set:
 * get:
 */
static struct Ucamera_Video_PU_Control gamma_pu = {
	.type = UVC_GAMMA_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 800,
	.data[UVC_DEF] = 400,
	.data[UVC_CUR] = 400,
};

static int sample_video_gamma_set(int value)
{
	IMPISPGamma gamma;
	memset(&gamma, 0, sizeof(IMPISPGamma));

	memcpy(&gamma, &gamma_cur, sizeof(IMPISPGamma));
	gamma.gamma[64] = gamma_cur.gamma[64] + 3*(value - 400);
	IMP_ISP_Tuning_SetGamma(&gamma);

	gamma_pu.data[UVC_CUR] = value;
	ucamera_uvc_pu_attr_save(UVC_GAMMA_CONTROL, value);
	return 0;
}

static int sample_video_gamma_get(void)
{
	return gamma_pu.data[UVC_CUR];
}

/*
 * cs : UVC_HUE_CONTROL
 * set:
 * get:
 */
static struct Ucamera_Video_PU_Control hue = {
	.type = UVC_HUE_CONTROL,
	.data[UVC_MIN] = 50,
	.data[UVC_MAX] = 160,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static int sample_video_hue_set(int value)
{
	int ret;
	unsigned char hue_value = value & 0xff;
	ret = IMP_ISP_Tuning_SetBcshHue(hue_value);
	if (ret)
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
	hue.data[UVC_CUR] = hue_value;
	ucamera_uvc_pu_attr_save(UVC_HUE_CONTROL, hue_value);
	return 0;
}

static int sample_video_hue_get(void)
{
	int ret;
	unsigned char hue_value = 0;

	if (!uvc_ctx.stream_on)
		return hue.data[UVC_CUR];
	ret = IMP_ISP_Tuning_GetBcshHue(&hue_value);
	if (ret) {
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
		hue_value = 128;
	}
	return hue_value;
}

/*
 * cs : UVC_WHITE_BALANCE_TEMPERATURE_CONTROL
 * set:
 * get:
 */
static struct Ucamera_Video_PU_Control whitebalance = {
	.type = UVC_WHITE_BALANCE_TEMPERATURE_CONTROL,
	.data[UVC_MIN] = 300,
	.data[UVC_MAX] = 600,
	.data[UVC_DEF] = 417,
};

/*
* value高16位代表模式（0:自动/ 1:手动
* value 底16位为白平衡实际值
*/
static int sample_video_whitebalance_set(int value)
{
	int ret;
	unsigned short gain, mode = 0;
	IMPISPWB wb = {0};

	mode = value >> 16;
	gain = value & 0xffff;

	if (mode == ISP_CORE_WB_MODE_AUTO)
		wb.mode = mode;
	else{
		wb.mode = ISP_CORE_WB_MODE_MANUAL;
		wb.bgain = gain;
		wb.rgain = 160000/gain;
	}
	ret = IMP_ISP_Tuning_SetWB(&wb);
	if (ret)
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);

	whitebalance.data[UVC_CUR] = wb.bgain;
	ucamera_uvc_pu_attr_save(UVC_WHITE_BALANCE_TEMPERATURE_CONTROL, wb.bgain);
	return 0;
}

static int sample_video_whitebalance_get(void)
{
	int ret, tmp = 0;
	IMPISPWB wb = {0};

	ret = IMP_ISP_Tuning_GetWB(&wb);
	if (ret)
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
	if (wb.mode == ISP_CORE_WB_MODE_AUTO)
		wb.bgain = 417;
	tmp = (wb.mode << 16) | wb.bgain;
	return tmp;
}

static struct Ucamera_Video_PU_Control powerlinefreq = {
	.type = UVC_POWER_LINE_FREQUENCY_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 2,
	.data[UVC_DEF] = 0,
};

static int sample_video_powerlinefreq_set(int value)
{
	int ret;
	uint32_t fps_num_bak,fps_den_bak, fps_num;
	/* IMPISPAntiflickerAttr attr; */
	int attr;

	attr = value;
	if (attr < IMPISP_ANTIFLICKER_DISABLE || attr >= IMPISP_ANTIFLICKER_BUTT) {
		printf("ERROR(%s) Sample set PowerLine Freq Invalid level:%d\n",MODULE_TAG,  value);
		return 0;
	}
	
	printf("[%s]attr:%d\n",__func__,attr);
	ret = IMP_ISP_Tuning_SetAntiFlickerAttr(attr);
	if (ret)
		IMP_LOG_WARN(MODULE_TAG, "SetAntiFlickerAttr failed \n");
	if (attr == IMPISP_ANTIFLICKER_50HZ)
		fps_num = SENSOR_FRAME_RATE_NUM_25;
	else if(attr == IMPISP_ANTIFLICKER_60HZ)
		fps_num = SENSOR_FRAME_RATE_NUM_30;
	else
		fps_num = uvc_ctx.sensor_fps;
	printf("[%s]attr:%d\n",__func__,attr);
	ret = IMP_ISP_Tuning_GetSensorFPS(&fps_num_bak, &fps_den_bak);
	if (ret < 0) {
		IMP_LOG_WARN(MODULE_TAG, "GetSensorFPS failed \n");
		return -1;
	}
	if(fps_num != fps_num_bak/fps_den_bak){
		ret = IMP_ISP_Tuning_SetSensorFPS(fps_num, 1);
		if (ret < 0) {
			IMP_LOG_WARN(MODULE_TAG, "SetSensorFPS failed \n");
			return -1;
		}
	}

	powerlinefreq.data[UVC_CUR] = attr;
	ucamera_uvc_pu_attr_save(UVC_POWER_LINE_FREQUENCY_CONTROL, attr);
	return 0;
}

static int sample_video_powerlinefreq_get(void)
{
	int ret;
	IMPISPAntiflickerAttr attr;

	if (!uvc_ctx.stream_on)
		return powerlinefreq.data[UVC_CUR];

	ret = IMP_ISP_Tuning_GetAntiFlickerAttr(&attr);
	if (ret)
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
	return attr;
}

static struct Ucamera_Video_PU_Control *Pu_ctrl[] = {
	&backlight_compens,
	&brightness,
	&contrast,
	&hue,
	&saturation,
	&sharpness,
	&gamma_pu,
	&whitebalance,
	&powerlinefreq,
	NULL,
};

/*
 * SET UVC Camera Terminal
 */

/*
 * cs : UVC_AUTO_EXPOSURE_MODE_CONTROL
 * set:
 * get:
 */
static struct Ucamera_Video_CT_Control auto_exposure_mode = {
	.type = UVC_AUTO_EXPOSURE_MODE_CONTROL,
	.data[UVC_MIN] = 1,
	.data[UVC_MAX] = 4,
	.data[UVC_DEF] = 2,
	.data[UVC_CUR] = 2,
};

static int sample_video_auto_exposure_mode_set(int value)
{
    int ret;
    if(value == 2){
	    ret = IMP_ISP_Tuning_SetAeComp(128);
	    if(ret)
		    IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
    }
    auto_exposure_mode.data[UVC_CUR] = value;
    return 0;
}

static int sample_video_auto_exposure_mode_get(void)
{
	return auto_exposure_mode.data[UVC_CUR];
}

/*
 * cs : UVC_EXPOSURE_TIME_CONTROL
 * set:
 * get:
 */
static struct Ucamera_Video_CT_Control exposure_time = {
	.type = UVC_EXPOSURE_TIME_CONTROL,
	.data[UVC_MIN] = 1,
	.data[UVC_MAX] = 1000,
	.data[UVC_DEF] = 156,
	.data[UVC_CUR] = 156,
};

static int sample_video_exposure_time_set(int value)
{
    int ret, value_time=0;
    if(value > 900)
	    value_time = 255;
    else if(value > 600)
	    value_time = 220;
    else if(value > 300)
	    value_time = 190;
    else
	    value_time = value;

    ret = IMP_ISP_Tuning_SetAeComp(value_time);
    if(ret)
	    IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);

    exposure_time.data[UVC_CUR] = value;
    return 0;
}

static int sample_video_exposure_time_get(void)
{
	return exposure_time.data[UVC_CUR];
}

/*
 * cs : UVC_FOCUS_CONTROL
 * set:
 * get:
 */
static struct Ucamera_Video_CT_Control focus = {
	.type = UVC_FOCUS_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 1000,
	.data[UVC_DEF] = 500,
	.data[UVC_CUR] = 500,
};

static int sample_video_focus_set(int value)
{
    focus.data[UVC_CUR] = value;
    return 0;
}

static int sample_video_focus_get()
{
	return focus.data[UVC_CUR];
}


/*
 * cs : UVC_FOCUS_AUTO_CONTROL
 * set:
 * get:
 */
static struct Ucamera_Video_CT_Control focus_auto = {
    .type = UVC_FOCUS_AUTO_CONTROL,
    .data[UVC_MAX] = 1,
    .data[UVC_MIN] = 0,
    .data[UVC_DEF] = 1,
    .data[UVC_CUR] = 1,
};

static int sample_video_focus_auto_set(int value)
{
    focus_auto.data[UVC_CUR] = value;
    return 0;
}

static int sample_video_focus_auto_get()
{
	return focus_auto.data[UVC_CUR];
}

/*
 * cs : UVC_ZOOM_ABSOLUTE_CONTROL
 * set:
 * get:
 */
static struct Ucamera_Video_CT_Control zoom = {
	.type = UVC_ZOOM_ABSOLUTE_CONTROL,
	.data[UVC_MAX] = 200,
	.data[UVC_MIN] = 100,
	.data[UVC_DEF] = 100,
	.data[UVC_CUR] = 100,
};

static int sample_video_zoom_set(int value)
{
	int ret;
	float value_cur;
	int zoomwidth_cur,zoomheight_cur;
	value_cur = value;

	zoomwidth_cur = round(uvc_ctx.sensor_width / sqrt(value_cur / 100));
	zoomwidth_cur = round((float)(zoomwidth_cur / 4)) * 4;
	zoomheight_cur = round(uvc_ctx.sensor_height / sqrt(value_cur / 100));
	zoomheight_cur = round((float)(zoomheight_cur / 4)) * 4;

	IMPISPFrontCrop fcrop_obj;
	fcrop_obj.fcrop_enable = 1;
	fcrop_obj.fcrop_left = round((float)(uvc_ctx.sensor_width - zoomwidth_cur) / 2);
	fcrop_obj.fcrop_top = round((float)(uvc_ctx.sensor_height -  zoomheight_cur) / 2);
	fcrop_obj.fcrop_width = zoomwidth_cur;
	fcrop_obj.fcrop_height = zoomheight_cur;

	ret = IMP_ISP_Tuning_SetFrontCrop(&fcrop_obj);
	if (ret < 0) {
		IMP_LOG_WARN(MODULE_TAG, " %s failed \n",__func__);
		return -1;
	}
	//zoom.data[UVC_CUR] = value;

	return 0;
}

static int sample_video_zoom_get(void)
{
	return zoom.data[UVC_CUR];
}

static struct Ucamera_Video_CT_Control *Ct_ctrl[] = {
    &exposure_time,
    &auto_exposure_mode,
    &focus,
    &focus_auto,
    &zoom,
    NULL,
};

static struct Ucamera_Video_EU_Control euctl;

/**
 * TODO:
 */
static int sample_video_eu_set(int cmd, void *data, int len)
{
	int ret = -1;
	char data_buf[len];
	memset(data_buf, 0, len);
	memcpy(data_buf, data, len);

	switch(cmd){
		/*实现客户自己的set_cur的功能,下面注释的为参考*/
	case UVC_EU_CMD_USR13:
		{
		int value = 0;
		if (data_buf[1] == 0x1 && data_buf[0] == 0x1) {
		} else {
			/* recovery and cdc log*/
			memcpy(&value, data, sizeof(int));
			ret =  set_norflash_flag(value);
			if(ret < 0)
				printf("ERROR(%s): set cpm error\n", MODULE_TAG);
		}

		}
		break;
	default:
		printf("WARNNING(%s): Unkown uvc eu cmd:%d\n", MODULE_TAG, cmd);
	}
	return ret;
}

static int sample_video_eu_get(int cmd, int req, void *data)
{
	uint8_t ret = 1;
	/* uint8_t len = 1; */
	/* uint8_t *data_cpy; */
	/* data_cpy = (uint8_t *)data; */
	switch(cmd){
		/*实现客户自己的get_cur的功能,下面注释的为参考*/
	case UVC_EU_CMD_USR12:
		switch(req){
		case GET_CUR:
			/* ret = strlen(KIVA_VERSION); */
			/* ret += 1; */
			/* memcpy(data_cpy, KIVA_VERSION, ret); */
			break;
		}
		break;
	case UVC_EU_CMD_USR14:
		switch(req){
		case GET_CUR:
			/* ret = strlen(serial_label); */
			/* ret += 1; */
			/* memcpy(data_cpy, serial_label, ret); */
			break;
		}
		break;
	default:
		/* memcpy(data, &len, sizeof(len)); */
		break;
	}
	return ret;
}

static const char *uvc_pu_type_to_string(int type)
{
	return pu_string[type].s;
}

static int uvc_pu_string_to_type(char *string)
{
	int i, index;
	index = sizeof(pu_string)/sizeof(struct uvc_attr_string);

	for (i = 0; i < index; i++) {
		if (strcmp(string, pu_string[i].s) == 0)
			return pu_string[i].id;
	}
	return 0;
}

static int uvc_pu_attr_setcur(int type, int value)
{
	int ret = 0;
	struct Ucamera_Video_PU_Control *pu_attr = NULL;

	switch (type) {
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
		pu_attr = &gamma_pu;
		break;
	case UVC_WHITE_BALANCE_TEMPERATURE_CONTROL:
		pu_attr = &whitebalance;
		break;
	case UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
		break;
	default:
		printf("ERROR(%s): Unkown uvc pu type:%d\n", MODULE_TAG, type);
		ret = -1;
		break;
	}
	if (pu_attr)
		pu_attr->data[UVC_CUR] = value;

	return ret;
}

static int ucamera_uvc_pu_attr_save(int type, int value)
{
	char key[64] = {0};
	char data[16] = {0};
	char line_str[128] = {0};
	const char *attr_string = NULL;


	if (uvc_ctx.uvc_attr_fd == NULL) {
		printf("ERROR(%s): can not open %s file!\n",MODULE_TAG, KIVA_ATTR_PATH);
		return -1;
	}

	attr_string = uvc_pu_type_to_string(type);
	if (attr_string == NULL)
		return -1;

	pthread_mutex_lock(&uvc_ctx.uvc_attr_mutex);
	fseek(uvc_ctx.uvc_attr_fd, 0L, SEEK_SET);

	while (!feof(uvc_ctx.uvc_attr_fd)) {
		if (fscanf(uvc_ctx.uvc_attr_fd, "%[^\n]", line_str) < 0)
			break;

		if (sscanf(line_str, "%[^:]:%[^\n]", key, data) != 2) {
			printf("WARNNING(%s): Invalid param:%s\n",MODULE_TAG,  line_str);
			fseek(uvc_ctx.uvc_attr_fd , 1L, SEEK_CUR);
			continue;
		}
		if (strcmp(key, attr_string) == 0) {
			fseek(uvc_ctx.uvc_attr_fd, -strlen(line_str), SEEK_CUR);
			fprintf(uvc_ctx.uvc_attr_fd, "%s:%04d", key, value);
		}
		fseek(uvc_ctx.uvc_attr_fd , 1L, SEEK_CUR);
	}

	pthread_mutex_unlock(&uvc_ctx.uvc_attr_mutex);
	return 0;
}

static int strToInt(char const* str) {
	return strtol(str, NULL, 10);
}

static int ucamera_uvc_attr_load(void)
{
	char key[64] = {0};
	char value[16] = {0};
	char line_str[128] = {0};
	int type;

	if ((uvc_ctx.uvc_attr_fd = fopen(KIVA_ATTR_PATH, "r+")) == NULL) {
		printf("ERROR(%s): open config %s failed!\n", MODULE_TAG, KIVA_ATTR_PATH);
		return -1;
	}

	while (!feof(uvc_ctx.uvc_attr_fd)) {
		if (fscanf(uvc_ctx.uvc_attr_fd, "%[^\n]", line_str) < 0)
			break;

		if (sscanf(line_str, "%[^:]:%[^\n]", key, value) != 2) {
			printf("WARNNING(%s): skip config %s\n", MODULE_TAG, line_str);
			fseek(uvc_ctx.uvc_attr_fd , 1L, SEEK_CUR);
			continue;
		}

		/*set PU */
		if ((type = uvc_pu_string_to_type(key))) {
			uvc_pu_attr_setcur(type, strToInt(value));
		}

		/**
		 * TODO: set CT / EU
		 **/
		fseek(uvc_ctx.uvc_attr_fd , 1L, SEEK_CUR);
	}

	return 0;
}


static int get_jpeg_snap(char *img_data)
{
	return  sample_get_jpeg_snap(UVC_VIDEO_CH, img_data);
}

static int get_yuv_snap(char *img_data)
{
	return  sample_get_yuv_snap(UVC_VIDEO_CH, img_data);
}

static int get_h264_snap(char *img_data)
{
	return  sample_get_h264_snap(UVC_VIDEO_CH, img_data);
}

static void register_video_callback()
{
	v_func.get_YuvFrame = get_yuv_snap;
	v_func.get_JpegFrame = get_jpeg_snap;
	v_func.get_H264Frame = get_h264_snap;
	Ucamera_Video_Regesit_CB(&v_func);
}

static void register_pu_callback()
{
	backlight_compens.set = sample_video_backlight_compens_set;
	backlight_compens.get = sample_video_backlight_compens_get;

	brightness.set = sample_video_brightness_set;
	brightness.get = sample_video_brightness_get;

	contrast.set = sample_video_contrast_set;
	contrast.get = sample_video_contrast_get;

	powerlinefreq.set = sample_video_powerlinefreq_set;
	powerlinefreq.get = sample_video_powerlinefreq_get;

	hue.set = sample_video_hue_set;
	hue.get = sample_video_hue_get;

	saturation.set = sample_video_saturation_set;
	saturation.get = sample_video_saturation_get;

	sharpness.set = sample_video_sharpness_set;
	sharpness.get = sample_video_sharpness_get;

	gamma_pu.set = sample_video_gamma_set;
	gamma_pu.get = sample_video_gamma_get;

	whitebalance.set = sample_video_whitebalance_set;
	whitebalance.get = sample_video_whitebalance_get;
	Ucamera_Video_Regesit_Process_Unit_CB(Pu_ctrl);
}

static void register_ct_callback()
{

	auto_exposure_mode.set = sample_video_auto_exposure_mode_set;
	auto_exposure_mode.get = sample_video_auto_exposure_mode_get;

	exposure_time.set = sample_video_exposure_time_set;
	exposure_time.get = sample_video_exposure_time_get;

	focus.set = sample_video_focus_set;
	focus.get = sample_video_focus_get;

	focus_auto.set = sample_video_focus_auto_set;
	focus_auto.get = sample_video_focus_auto_get;

	zoom.set = sample_video_zoom_set;
	zoom.get = sample_video_zoom_get;
	Ucamera_Video_Regesit_Camera_Terminal_CB(Ct_ctrl);
}

static void register_eu_callback()
{
	euctl.set = sample_video_eu_set;
	euctl.get = sample_video_eu_get;
	Ucamera_Video_Regesit_Extension_Unit_CB(euctl);
}

static int imp_isp_tuning_set(void)
{
	int ret;
	uint32_t fps_num = 30;

	IMP_ISP_Tuning_SetContrast(contrast.data[UVC_CUR]);
	IMP_ISP_Tuning_SetSharpness(sharpness.data[UVC_CUR]);
	IMP_ISP_Tuning_SetSaturation(saturation.data[UVC_CUR]);
	IMP_ISP_Tuning_SetBrightness(brightness.data[UVC_CUR]);
	IMP_ISP_Tuning_SetHiLightDepress(backlight_compens.data[UVC_CUR]);

    	IMP_ISP_Tuning_SetBcshHue(hue.data[UVC_CUR]);

	if(auto_exposure_mode.data[UVC_CUR] == 4)
		sample_video_exposure_time_set(exposure_time.data[UVC_CUR]);
	else
		IMP_ISP_Tuning_SetAeComp(128);

	/* IMPISPAntiflickerAttr attr = powerlinefreq.data[UVC_CUR]; */
	int attr = powerlinefreq.data[UVC_CUR];
	if (attr < IMPISP_ANTIFLICKER_DISABLE || attr >= IMPISP_ANTIFLICKER_BUTT) {
		printf("WARNNING(%s): unvaild Antiflicker param:%d\n", MODULE_TAG, attr);
		attr = IMPISP_ANTIFLICKER_DISABLE;
	}
	IMP_ISP_Tuning_SetAntiFlickerAttr(attr);

	if (attr == IMPISP_ANTIFLICKER_50HZ)
		fps_num = SENSOR_FRAME_RATE_NUM_25;
	else if(attr == IMPISP_ANTIFLICKER_60HZ)
		fps_num = SENSOR_FRAME_RATE_NUM_30;
	else
		fps_num = uvc_ctx.sensor_fps;

	ret = IMP_ISP_Tuning_SetSensorFPS(fps_num, 1);
	if (ret < 0) {
		IMP_LOG_ERR(MODULE_TAG, "failed to set sensor fps\n");
		return -1;
	}
	printf("INFO(%s): set Antiflicker level:%d fps:%d\n",MODULE_TAG, attr, fps_num);

	/* update ev status(used by dynamic_fps) */
	ret = IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
	if (ret < 0){
		IMP_LOG_ERR(MODULE_TAG, "failed to set running mode\n");
		return -1;
	}

	if(getGammaOnlyOnce){
		ret = IMP_ISP_Tuning_GetGamma(&gamma_cur);
		if (ret < 0){
			IMP_LOG_ERR(MODULE_TAG, "failed to get gamma value\n");
			return -1;
		}
		sample_video_gamma_set(gamma_pu.data[UVC_CUR]);
		getGammaOnlyOnce = 0;
	}

	return 0;
}

static int imp_sdk_init(int format, int width, int height)
{
	int i, ret;

	imp_isp_tuning_set();

	/* FrameSource init */
	ret = sample_uvc_framesource_init(UVC_VIDEO_CH, uvc_ctx.sensor_fps, format , width, height);
	if (ret < 0) {
		printf("ERROR(%s): sample_uvc_framesource_init faile \n", MODULE_TAG);
		return -1;
	}
	if (format == V4L2_PIX_FMT_YUYV || format == V4L2_PIX_FMT_NV12) {
		ret = sample_framesource_streamon(UVC_VIDEO_CH);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "ImpStreamOn failed\n");
			return -1;
		}

		ret = IMP_FrameSource_SetFrameDepth(UVC_VIDEO_CH, 1);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "%s(%d):IMP_FrameSource_SetFrameDepth failed\n", __func__, __LINE__);
			return -1;
		}
		frame_fmt = 1;
//################IVS-OSD
		/*frame_fmt = 1;
		
		ret = module_osd_control_init();
		if (ret < 0) {
			printf("ERROR(%s): osd init failed\n", MODULE_TAG);
			return -1;
		}

		ret = IVS_OSD_Init(1);
		if (ret < 0) {
			printf("IMP_IVS_CreateGroup(1) failed\n");
			return -1;
		}
		IMPCell ivscell = {DEV_ID_IVS, 1, 1};
		IMPCell osdcell = {DEV_ID_OSD, OSD_CONTROL_GROUP, 0};
		ret = IMP_System_Bind(&chn[UVC_VIDEO_CH].framesource_chn, &osdcell);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, " Bind FrameSource channel%d and OSD failed\n", UVC_VIDEO_CH);
			return -1;
		}
		
		ret = IMP_System_Bind(&osdcell, &ivscell);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, " Bind OSD and IVS failed\n");
			return -1;
		}

		
		IVS_OSD_Start();*/
//###################################
		uvc_ctx.stream_on = 1;
#ifdef MODULE_FACEZOOM_ENABLE
		sem_wait(&uvc_ctx.ucam_fs_sem);
#endif
		return 0;
	}

	/* CreateGroup for Encoder */
	i = UVC_VIDEO_CH;
	if (chn[i].enable) {
		ret = IMP_Encoder_CreateGroup(chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
			return -1;
		}

	}

	/* Encoder init */
	switch (format) {
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_NV12:
		break;
	case V4L2_PIX_FMT_MJPEG:
		{
#if 1
		IMPISPFixedContrastAttr fixedContrastAttr;
		fixedContrastAttr.mode = uvc_ctx.bcsh_en;
		fixedContrastAttr.range_low = uvc_ctx.bcsh_low;
		fixedContrastAttr.range_high = uvc_ctx.bcsh_high;
		ret = IMP_ISP_SetFixedContraster(&fixedContrastAttr);
		if(ret < 0){
			IMP_LOG_ERR(MODULE_TAG, "IMP_ISP_SetFixedContraster failed \n");
			return -1;
		}
		printf("INFO(%s): set Fixedcontrastattr mode = %d OK!\n",MODULE_TAG, fixedContrastAttr.mode);
#endif
		ret = sample_jpeg_init(UVC_VIDEO_CH);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "Encoder init failed\n");
			return -1;
		}
		break;
		}
	case V4L2_PIX_FMT_H264:
		ret = sample_encoder_init(UVC_VIDEO_CH, IMP_ENC_PROFILE_AVC_MAIN);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "Encoder init failed\n");
			return -1;
		}
		break;
	case V4L2_PIX_FMT_H265:
		ret = sample_encoder_init(UVC_VIDEO_CH, IMP_ENC_PROFILE_HEVC_MAIN);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "Encoder init failed\n");
			return -1;
		}
		break;
	}

//创建osd组
		ret = IMP_OSD_CreateGroup(chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "IMP_OSD_CreateGroup(%d) e rror !\n", i);
			return -1;
		}
	
	Ucamera_LOG("OSD_CreateGroup success!");
//初始化osd
	prHander = sample_osd_init(chn[i].index);
	if (prHander == NULL) {
		IMP_LOG_ERR(MODULE_TAG, "OSD init failed\n");
		return -1;
	}
	Ucamera_LOG("OSD_INIT success!");

	/* Step.4 Bind */
	i = UVC_VIDEO_CH;
	if (chn[i].enable) {
#ifdef OSD_CONTROL
		/*OSD init */
		ret = osd_init(OSD_CONTROL_GROUP, UVC_VIDEO_CH);
		if (ret < 0) {
			printf("ERROR(%s): osd init failed\n", MODULE_TAG);
			return -1;
		}

		IMPCell osdcell = {DEV_ID_OSD, OSD_CONTROL_GROUP, 0};
		IMPCell *dstcell = &chn[i].imp_encoder;
		ret = IMP_System_Bind(&chn[i].framesource_chn, &osdcell);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, " Bind FrameSource channel%d and OSD failed\n", i);
			return -1;
		}

		ret = IMP_System_Bind(&osdcell, dstcell);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, " Bind FrameSource OSD and encoder failed\n");
			return -1;
		}
#else
		/*ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "Bind FrameSource channel%d and Encoder failed\n",i);
			return -1;
		}*/
#endif
		ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_osd);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "Bind FrameSource channel%d and OSD failed\n",i);
			return -1;
		}
		
		ret = IMP_System_Bind(&chn[i].imp_osd, &chn[i].imp_encoder);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "Bind OSD and Encoder failed\n");
			return -1;
		}
	}

	/* Stream On */
	ret = sample_framesource_streamon(UVC_VIDEO_CH);
	if (ret < 0) {
		IMP_LOG_ERR(MODULE_TAG, "ImpStreamOn failed\n");
		return -1;
	}

	int enc_ch = 0;
	switch (format) {
	case V4L2_PIX_FMT_MJPEG:
		enc_ch = chn[UVC_VIDEO_CH].index + 3;
		break;
	case V4L2_PIX_FMT_H264:
	case V4L2_PIX_FMT_H265:
		enc_ch = chn[UVC_VIDEO_CH].index;
		break;
	}

	ret = IMP_Encoder_StartRecvPic(enc_ch);
	if (ret < 0) {
		IMP_LOG_ERR(MODULE_TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", enc_ch);
		return -1;
	}

	uvc_ctx.stream_on = 1;
#ifdef MODULE_FACEZOOM_ENABLE
	sem_wait(&uvc_ctx.ucam_fs_sem);
#endif
	return 0;
}

static int imp_sdk_deinit(int format)
{
	int i, ret;

	if (format == V4L2_PIX_FMT_YUYV || format == V4L2_PIX_FMT_NV12) {
		frame_fmt = -1;
		//##################IVS_OSD
		/*frame_fmt = -1;
		IMPCell ivscell = {DEV_ID_IVS, 1, 1};
		IMPCell osdcell = {DEV_ID_OSD, OSD_CONTROL_GROUP, 0};
		ret = IMP_System_UnBind(&chn[UVC_VIDEO_CH].framesource_chn, &osdcell);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, " UnBind FrameSource channel%d and OSD failed\n", UVC_VIDEO_CH);
			return -1;
		}

		ret = IMP_System_UnBind(&osdcell, &ivscell);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, " UnBind OSD and IVS failed\n");
			return -1;
		}

		IVS_OSD_Stop();
		
		ret = module_osd_control_deinit();
		if (ret < 0) {
			printf("ERROR(%s): osd_init exit failed\n", MODULE_TAG);
			return -1;
		}*/
		//###########################
		/* Step.5 Stream Off */
		ret = sample_framesource_streamoff(UVC_VIDEO_CH);
		if (ret < 0) {
			printf("IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[0].index);
			return -1;
		}
		/* Step.6 FrameSource exit */
		/*Destroy channel i*/
		ret = sample_framesource_exit(UVC_VIDEO_CH);
		if (ret < 0) {
			printf("IMP_FrameSource_DestroyChn() error: %d\n", ret);
			return -1;
		}

	} else {

		int enc_ch = 0;
		switch (format) {
		case V4L2_PIX_FMT_MJPEG:
			enc_ch = chn[UVC_VIDEO_CH].index + 3;
			break;
		case V4L2_PIX_FMT_H264:
		case V4L2_PIX_FMT_H265:
			enc_ch = chn[UVC_VIDEO_CH].index;
			break;
		}

		ret = IMP_Encoder_StopRecvPic(enc_ch);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "IMP_Encoder_StopRecvPic(%d) failed\n", chn[0].index);
			return -1;
		}

		/* Step.a Stream Off */
		ret = sample_framesource_streamoff(UVC_VIDEO_CH);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "FrameSource StreamOff failed\n");
			return -1;
		}

		/* Step.b UnBind */
		i = UVC_VIDEO_CH;
		if (chn[i].enable) {
#ifdef OSD_CONTROL
			IMPCell osdcell = {DEV_ID_OSD, OSD_CONTROL_GROUP, 0};
			ret = IMP_System_UnBind(&osdcell, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(MODULE_TAG, "UnBind OSD%d and Encoder failed\n",i);
				return -1;
			}

			ret = IMP_System_UnBind(&chn[i].framesource_chn, &osdcell);
			if (ret < 0) {
				IMP_LOG_ERR(MODULE_TAG, "UnBind FrameSource channel%d and OSD failed\n", i);
				return -1;
			}

			/*osd_deinit */
			ret = osd_deinit(OSD_CONTROL_GROUP);
			if (ret < 0) {
				printf("ERROR(%s): osd_init exit failed\n", MODULE_TAG);
				return -1;
			}
#else
			/*ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(MODULE_TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
				return -1;
			}*/
			ret = IMP_System_UnBind(&chn[i].imp_osd, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(MODULE_TAG, "UnBind FrameSource channel%d and OSD failed\n",i);
				return -1;
			}
			ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_osd);
			if (ret < 0) {
				IMP_LOG_ERR(MODULE_TAG, "UnBind OSD and Encoder failed\n");
				return -1;
			}
#endif
		}
	
		/* Step.c OSD exit */
		ret = sample_osd_exit(prHander,chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "OSD exit failed\n");
			return -1;
		}
		
		/* Step.c Encoder exit */
		switch (format) {
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_NV12:
			break;
		case V4L2_PIX_FMT_MJPEG:
			ret = sample_jpeg_exit(UVC_VIDEO_CH);
			break;
		case V4L2_PIX_FMT_H264:
		case V4L2_PIX_FMT_H265:
			ret = sample_encoder_exit(UVC_VIDEO_CH);
			break;
		}
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "Encoder init failed\n");
			return -1;
		}

		i = UVC_VIDEO_CH;
		if (chn[i].enable) {
			ret = IMP_Encoder_DestroyGroup(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(MODULE_TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
				return -1; }
		}

		/* Step.d FrameSource exit */
		ret = sample_framesource_exit(UVC_VIDEO_CH);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "FrameSource exit failed\n");
			return -1;
		}
	}

	return 0;
}


static int uvc_event_process(int event_id, void *data)
{
	unsigned int intervals = 0;

	switch (event_id) {
	case UCAMERA_EVENT_STREAMON: {
		struct Ucamera_Video_Frame *frame = (struct Ucamera_Video_Frame *) data;
		/*uvc_ctx.stream_on = 1;*/
		intervals = frame->intervals;
		uvc_ctx.sensor_fps = 10000000 / intervals;
		osd_width = frame->width;
		osd_height = frame->height;
		frame_stream_switch = 1;
		int retry_cnt = 0;
imp_init_check:
		if (!uvc_ctx.imp_inited) {
			if (retry_cnt++ > 40) {
				printf("ERROR(%s): imp system init failed.\n", MODULE_TAG);
				return -1;
			}
			printf("ERROR(%s): imp sys not ready, wait and retry:%d \n", MODULE_TAG, retry_cnt);
			usleep(50*1000);
			goto imp_init_check;
		}

		return imp_sdk_init(frame->fcc, frame->width, frame->height);
	}
	case UCAMERA_EVENT_STREAMOFF: {
		struct Ucamera_Video_Frame *frame = (struct Ucamera_Video_Frame *) data;
		uvc_ctx.stream_on = 0;
		frame_stream_switch = 0;
#ifdef MODULE_FACEZOOM_ENABLE
		sem_wait(&uvc_ctx.ucam_fs_sem);
#endif
		return imp_sdk_deinit(frame->fcc);
	}

	default:
		printf("ERROR(%s): unknown message ->[%d]!\n", MODULE_TAG, event_id);
		return -1;
	};

	return 0;
}

void uvc_post_fs_sem()
{
	sem_post(&uvc_ctx.ucam_fs_sem);
}

int uvc_get_focus_cur()
{
	return focus.data[UVC_CUR];
}

int uvc_get_focus_auto()
{
	return focus_auto.data[UVC_CUR];
}

int uvc_stream_on()
{
	return uvc_ctx.stream_on;
}

void uvc_enable_impinited(int on)
{
	uvc_ctx.imp_inited = on;
}

int uvc_control_init(void *param)
{
	int ret = -1;
	config_func_param_t *cfg_param = (config_func_param_t *)param;
	if (!cfg_param) {
		printf("ERROR(%s): config_func_param is NULL \n", MODULE_TAG);
		return -1;
	}

	/*memset(&uvc_ctx, 0, sizeof(uvc_control_t));*/
	uvc_ctx.stream_on = 0;
	uvc_ctx.dynamic_fps = cfg_param->isp_attr.dynamic_fps;
	uvc_ctx.hvflip = cfg_param->isp_attr.hvflip;
	uvc_ctx.sensor_fps = cfg_param->isp_attr.fps_num;
	uvc_ctx.sensor_width = cfg_param->isp_attr.sensor_info.sensor_width;
	uvc_ctx.sensor_height = cfg_param->isp_attr.sensor_info.sensor_height;
	uvc_ctx.bcsh_en = cfg_param->video_info.bcsh_en;
	uvc_ctx.bcsh_low = cfg_param->video_info.bcsh_low;
	uvc_ctx.bcsh_high = cfg_param->video_info.bcsh_high;

	pthread_mutex_init(&uvc_ctx.uvc_attr_mutex, NULL);
	sem_init(&uvc_ctx.ucam_fs_sem, 0, 0);

	ret = ucamera_uvc_attr_load();
	if (ret < 0) {
		printf("ERROR(%s): ucamera_uvc_attr_load failed \n", MODULE_TAG);
		return -1;
	}

	register_video_callback();
	register_ct_callback();
	register_pu_callback();
	register_eu_callback();
	UCamera_Registe_Event_Process_CB(uvc_event_process);

	UCamera_Video_Start();
	return 0;
}

void uvc_control_deinit()
{
	Ucamera_DeInit();
	/*Ucamera_Video_Stop();*/
}
