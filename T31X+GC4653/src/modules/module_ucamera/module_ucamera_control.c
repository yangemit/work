#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <module_ucamera_control.h>
#include <usbcamera.h>
#include "uvc_control.h"
#include "uac_control.h"
#include <global_config.h>
#include <unistd.h>
#define MODULE_TAG          "UCAMERA"

static struct Ucamera_Cfg ucfg;

/* static ucamera_control_t ucamera_ctx; */

static void uvc_device_info_init(void *param)
{
	config_func_param_t *cfg_param = (config_func_param_t *)param;

	ucfg.pcfg.Vid = cfg_param->dev_info.vendor_id;
	ucfg.pcfg.version = cfg_param->dev_info.dev_bcd;
	strcpy(ucfg.pcfg.manufacturer, (char *)cfg_param->dev_info.manufact_label);
	strcpy(ucfg.pcfg.product, (char *)cfg_param->dev_info.product_label);
	strcpy(ucfg.pcfg.serial, (char *)cfg_param->dev_info.serial_number);
	strcpy(ucfg.pcfg.video_name, (char *)cfg_param->dev_info.video_name);
}

static void uac_device_info_init(void *param)
{
	config_func_param_t *cfg_param = (config_func_param_t *)param;

	int spk_en = cfg_param->audio_info.spk_en;
	if (spk_en) {
		cfg_param->dev_info.product_id += 1;
		ucfg.pcfg.Pid = cfg_param->dev_info.product_id;
	} else
		ucfg.pcfg.Pid = cfg_param->dev_info.product_id;;

	strcpy(ucfg.pcfg.audio_name, (char *)cfg_param->dev_info.audio_name);

	ucfg.acfg.mic_volume = cfg_param->audio_info.mic_vol;
	ucfg.acfg.spk_volume = cfg_param->audio_info.spk_vol;
	if (spk_en)
		ucfg.acfg.speak_enable = 1;
	else
		ucfg.acfg.speak_enable = 0;

	ucfg.audio_en = cfg_param->audio_info.audio_en;
}

static void uvc_foramt_init(void *param)
{
	config_func_param_t *cfg_param = (config_func_param_t *)param;
	video_info_t *video_info = (video_info_t *)(&(cfg_param->video_info));

	ucfg.adb_en = video_info->adb_en;
	ucfg.stillcap = video_info->stillcap_en;
	ucfg.h264_en = video_info->h264_en;
	ucfg.h265_en = video_info->h265_en;

	struct Ucamera_Video_Cfg *vcfg;
	vcfg = &(ucfg.vcfg);
	vcfg->yuyvnum = video_info->yuv_frame->frames_num;
	vcfg->jpegnum = video_info->jpeg_frame->frames_num;
	vcfg->h264num = video_info->h264_frame->frames_num;

	/*TODO: */
	struct Ucamera_YUYV_Param *yuyvl = NULL;
	struct Ucamera_JPEG_Param *jpegl = NULL;
	struct Ucamera_H264_Param *h264l = NULL;
	int nframes = vcfg->jpegnum;
	yuyvl = malloc(vcfg->yuyvnum * sizeof(uvc_frame_t));
	jpegl = malloc(vcfg->jpegnum * sizeof(uvc_frame_t));
	h264l = malloc(vcfg->h264num * sizeof(uvc_frame_t));

	vcfg->yuyvlist = yuyvl;
	vcfg->jpeglist = jpegl;
	vcfg->h264list = h264l;

	int i = 0, index = 0;
	for (i = 0; i < nframes; i++) {
		jpegl[i].width = video_info->jpeg_frame[i].width;
		jpegl[i].height = video_info->jpeg_frame[i].height;
		jpegl[i].intervals = video_info->jpeg_frame[i].intervals;
		h264l[i].width = video_info->h264_frame[i].width;
		h264l[i].height = video_info->h264_frame[i].height;
		h264l[i].intervals = video_info->h264_frame[i].intervals;

		int width = video_info->yuv_frame[i].width;
		if (width <= 1280) {
			yuyvl[index].width = video_info->yuv_frame[i].width;
			yuyvl[index].height = video_info->yuv_frame[i].height;
			yuyvl[index].intervals = video_info->yuv_frame[i].intervals;
			index++;
		} else {
			vcfg->yuyvnum -= 1;
		}
	}

}

static void device_info_init(void *param)
{
	uvc_device_info_init(param);
	uac_device_info_init(param);

	uvc_foramt_init(param);

	Ucamera_Config(&ucfg);
	Ucamera_Init(UVC_BUF_NUM, UVC_BUF_SIZE);
}

int module_ucamera_stream_on(void)
{
	return uvc_stream_on();
}

void module_ucamera_enable_impinited(void)
{
	uvc_enable_impinited(1);
}

int module_ucamera_get_focus_cur()
{
	return uvc_get_focus_cur();
}

int module_ucamera_get_focus_auto()
{
	return uvc_get_focus_auto();
}

void module_ucamera_post_fs()
{
	uvc_post_fs_sem();
}

int module_ucamera_init(void *param)
{
	int ret = -1;
	/*device info */
	device_info_init(param);
	usleep(1000);
	uvc_control_init(param);
	ret = uac_control_init(param);
	if (ret < 0) {
		printf("ERROR(%s): uac_control_init failed \n", MODULE_TAG);
		return -1;
	}


	return 0;
}

void module_ucamera_deinit()
{
	uvc_control_deinit();
	uac_control_deinit();

	printf("INFO(%s): module_ucamera_deinit ...ok \n", MODULE_TAG);
}
