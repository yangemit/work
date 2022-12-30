/*
 * load_config.c
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co.,Ltd
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* #include <usbcamera.h> */
#include "global_config.h"
#include <imp-common.h>

#define MODULE_TAG            "LOAG_CONFIG"
static FILE *uvc_config_fd = NULL;

static int spk_volume_line;
static uvc_frame_t default_frame[4] = {
	{
		.width = 1920,
		.height = 1080,
		.intervals = 333333,
		.frames_num = 4,
	},
	{
		.width = 1280,
		.height = 720,
		.intervals = 333333,
		.frames_num = 4,
	},
	{
		.width = 640,
		.height = 480,
		.intervals = 333333,
		.frames_num = 4,
	},
	{
		.width = 640,
		.height = 360,
		.intervals = 333333,
		.frames_num = 4,
	}
};
config_func_param_t g_func_param = {
	.dev_info = {
		.vendor_id = VENDOR_ID,               /* VID */
		.product_id = PRODUCT_ID,             /* PID */
		.dev_bcd = BCD_VERSION,               /* 版本号 */
		.serial_number = SERIAL_NUMBER,       /* SN */
		.product_label = PRODUCT_LABEL,       /* 厂商名字 */
		.manufact_label = MANUFACT_LABEL,     /* 厂商名字 */
		.video_name = VIDEO_NAME,             /* 摄像头名字 */
		.audio_name = AUDIO_NAME,             /* 音频名称 */
	},

	.audio_info = {
		.audio_en = AUDIO_ENABLE,             /* audio 使能， 默认开AMIC */
		.audio_ns = 1,                        /* 音频降噪等级 */
		.spk_en= !SPKER_ENABLE,               /* spk 使能 ，和 mic 一起使用*/
		.dmic_en = !DMIC_ENABLE,              /* dmic 使能 */
		.mic_vol = MIC_VOL,                   /* mic_volume */
		.spk_vol = SPK_VOL,                   /* spk_volume */
	},

	.video_info = {
		.adb_en = !ADB_ENABLE,
		.stillcap_en = STILLCAP_ENABLE,
		.h264_en = !H264_ENABLE,
		.h265_en = !H265_ENABLE,
		.bcsh_en = !BCSH_ENABLE,
		.bcsh_low = 16,
		.bcsh_high = 235,
		.yuv_frame = default_frame,
		.jpeg_frame = default_frame,
		.h264_frame = default_frame,
	},

	.af_param = {
		.af_en = !AF_ENABLE,                             /* AF 使能 */
		.focus_trigger_value = FOCUS_TRIGGER_VALUE,      /* AF 触发值 */
		.motor_pixel = MOTOR_PIXEL,                      /* 权重范围 */
		.motor_step = MOTOR_STEP,                        /* 电机步长 */
		.motor_step_max = MOTOR_STEP_MAX,                /* vcm 行程最小值 */
		.motor_step_min = MOTOR_STEP_MIN,                /* vcm 行程最大值 */
		.motor_sleep_time = MOTOR_STEP_TIME,             /* 电机移动等待时间 */
	},
	.isp_attr = {
		.encoder_info = {
			.bitrate = 4000,
			.rcmode = IMP_ENC_RC_MODE_CBR,
			.qp_value = 80,
			.gop = 25,
		},
		.sensor_info = {
			.sensor_name = DEFAULT_SENSOR_NAME,
			.i2c_addr = 0x40,
			.sensor_width =1920 ,
			.sensor_height = 1080,

		},
		.fps_num = 30,
		.hvflip = 3,
	},
	.motor_track_param = {
		.motor_track_en = !MOTOR_TRACK_ENABLE,                     /* 电机是否打开 */
		.ucamera_en = MOTOR_TRACK_UCAMERA_ENABLE,                  /* ucamera_en */
		.direction = MOTOR_TRACK_DIRECTION,                        /* 移动方向 */
		.islimit = MOTOR_TRACK_LIMIT,                              /* 是否限位 */
		.reset_speed = MOTOR_TRACK_RESET_SPEED,                    /* 初始速度 */
		.lens_hov = MOTOR_TRACK_LENS_HOV,                          /* 水平方向视场角 */
		.hvflip = MOTOR_TRACK_HVFLIP,                              /* 镜像 */
		.hmaxstep = MOTOR_TRACK_HMAXSTEP,                          /* 电机转动一圈需要的最大脉冲数 */
		.run_speed_percent = MOTOR_TRACK_RUN_SPEED_PERCENT,        /* 电机运行速度百分比 */
	},
#ifdef MODULE_FACEZOOM_ENABLE
	.facezoom_param = {
		.facezoom_en = 1,
		.facezoom_mode = 1,
	},
#endif

};

static int strToInt(char const* str) {
#if 0
	int val;
	if (sscanf(str, "%d", &val) == 1) return val;
	return -1;
#endif
	return strtol(str, NULL, 10);
}

int strToHex(char const* str) {
#if 0
	int val;
	if (sscanf(str, "%x", &val) == 1) return val;
	return -1;
#endif
	return strtol(str, NULL, 16);
}

int speak_volume_write_config(int val)
{
	char value[24] = "speak_volume";
	char line_str[128] = {0};
	int line_num = 0;

	fseek(uvc_config_fd , 0L, SEEK_SET);
	while (!feof(uvc_config_fd)) {
		if (fscanf(uvc_config_fd, "%[^\n]", line_str) < 0){
			break;
		} else {
			fseek(uvc_config_fd , 1, SEEK_CUR);
			line_num++;
		}

		if (line_num == spk_volume_line - 1) {
			fprintf(uvc_config_fd, "%s    :%03d", value, val);
			} else
				continue;
		}
	return 0;
}

int kiva_load_config_init()
{
	int nframes = 0;
	unsigned int i, width, height;
	int line_num = 0;
	char key[64] = {0};
	char value[64] = {0};
	char *line_str = NULL;
	int intervals = 10000000/30;

	if ((uvc_config_fd = fopen(KIVA_CONFIG_PATH, "r+")) == NULL) {
		printf("ERROR(%s): open config file failed!\n", MODULE_TAG);
		return -1;
	}
	line_str = malloc(256*sizeof(char));
	if(line_str == NULL){
		printf("ERROR(%s): malloc if line_str is failed\n", MODULE_TAG);
		return -1;
	}

	while (!feof(uvc_config_fd)) {
		if (fscanf(uvc_config_fd, "%[^\n]", line_str) < 0)
			break;
		fseek(uvc_config_fd , 1, SEEK_CUR);
		line_num++;

		if (sscanf(line_str, "%[^:]:%[^\n]", key, value) != 2) {
			printf("warning: skip config %s\n", line_str);
			fseek(uvc_config_fd , 1, SEEK_CUR);
			continue;
		}

		char *ch = strchr(key, ' ');
		if (ch) *ch = 0;
		if (strcmp(key, "sensor_name") == 0) {
			strncpy((char *)g_func_param.isp_attr.sensor_info.sensor_name, value, LABEL_LEN);
		} else if (strcmp(key, "i2c_addr") == 0) {
			g_func_param.isp_attr.sensor_info.i2c_addr = strToHex(value);
		} else if (strcmp(key, "width") == 0) {
			g_func_param.isp_attr.sensor_info.sensor_width = strToInt(value);
		} else if (strcmp(key, "height") == 0) {
			g_func_param.isp_attr.sensor_info.sensor_height = strToInt(value);
		} else if (strcmp(key, "fps_num") == 0) {
			g_func_param.isp_attr.fps_num = strToInt(value);

		} else if (strcmp(key, "af_en") == 0) {
			g_func_param.af_param.af_en = strToInt(value);
		} else if(strcmp(key, "focus_trigger_value") == 0){
			g_func_param.af_param.focus_trigger_value = strToInt(value);
		} else if(strcmp(key,"motor_pixel") == 0) {
			g_func_param.af_param.motor_pixel = strToInt(value);
		} else if (strcmp(key, "motor_step") == 0) {
			g_func_param.af_param.motor_step = strToInt(value);
		} else if (strcmp(key, "motor_sleep") == 0) {
			g_func_param.af_param.motor_sleep_time = strToInt(value);
		} else if (strcmp(key, "motor_step_max") == 0) {
			g_func_param.af_param.motor_step_max = strToInt(value);
		} else if (strcmp(key, "motor_step_min") == 0) {
			g_func_param.af_param.motor_step_min = strToInt(value);

#ifdef MODULE_FACEZOOM_ENABLE
		} else if (strcmp(key, "face_zoom_on") == 0) {
			g_func_param.facezoom_param.facezoom_en = strToInt(value);
		} else if (strcmp(key, "face_zoom_mode") == 0) {
			g_func_param.facezoom_param.facezoom_mode = strToInt(value);
#endif
		} else if (strcmp(key, "bitrate") == 0) {
			g_func_param.isp_attr.encoder_info.bitrate = strToInt(value);
		} else if (strcmp(key, "gop") == 0) {
			g_func_param.isp_attr.encoder_info.gop = strToInt(value);
		} else if (strcmp(key, "qp_value") == 0) {
			g_func_param.isp_attr.encoder_info.qp_value = strToInt(value);

		} else if (strcmp(key, "adb_en") == 0) {
			g_func_param.video_info.adb_en = strToInt(value);
		} else if (strcmp(key, "stillcap_en") == 0) {
			g_func_param.video_info.stillcap_en = strToInt(value);
		} else if (strcmp(key, "audio_en") == 0) {
			g_func_param.audio_info.audio_en = strToInt(value);
		} else if (strcmp(key, "dmic_en") == 0) {
			g_func_param.audio_info.dmic_en = strToInt(value);
		} else if (strcmp(key, "speak_en") == 0) {
			g_func_param.audio_info.spk_en = strToInt(value);
		} else if (strcmp(key, "mic_volume") == 0) {
			g_func_param.audio_info.mic_vol = strToInt(value);
		} else if (strcmp(key, "speak_volume") == 0) {
			g_func_param.audio_info.spk_vol = strToInt(value);
			spk_volume_line = line_num;
		} else if (strcmp(key, "audio_ns") == 0) {
			g_func_param.audio_info.audio_ns = strToInt(value);

		} else if (strcmp(key, "h264_en") == 0) {
			g_func_param.video_info.h264_en = strToInt(value);
		} else if (strcmp(key, "h265_en") == 0) {
			g_func_param.video_info.h265_en = strToInt(value);
		} else if (strcmp(key, "bcsh_en") == 0) {
			g_func_param.video_info.bcsh_en = strToInt(value);
		} else if (strcmp(key, "bcsh_low") == 0) {
			g_func_param.video_info.bcsh_low = strToInt(value);
		} else if (strcmp(key, "bcsh_high") == 0) {
			g_func_param.video_info.bcsh_high = strToInt(value);
		} else if (strcmp(key, "uvc_led") == 0) {
			g_func_param.led_ctl.gpio = strToInt(value);
		} else if (strcmp(key, "led_level") == 0) {
			g_func_param.led_ctl.level = strToInt(value);
		} else if (strcmp(key, "hvflip") == 0) {
			g_func_param.isp_attr.hvflip = strToInt(value);
		} else if (strcmp(key, "dynamic_fps") == 0) {
			g_func_param.isp_attr.dynamic_fps = strToInt(value);
		} else if (strcmp(key, "device_bcd") == 0) {
			g_func_param.dev_info.dev_bcd = strToHex(value);
		} else if (strcmp(key, "product_id") == 0) {
			g_func_param.dev_info.product_id = strToHex(value);
		} else if (strcmp(key, "vendor_id") == 0) {
			g_func_param.dev_info.vendor_id = strToHex(value);
		} else if (strcmp(key, "serial_lab") == 0) {
			strncpy((char *)g_func_param.dev_info.serial_number, value, LABEL_LEN);
		} else if (strcmp(key, "product_lab") == 0) {
			strncpy((char*)g_func_param.dev_info.product_label, value, LABEL_LEN);
		} else if (strcmp(key, "manufact_lab") == 0) {
			strncpy((char*)g_func_param.dev_info.manufact_label, value, LABEL_LEN);
		} else if (strcmp(key, "video_name") == 0) {
			strncpy((char *)g_func_param.dev_info.video_name, value, LABEL_LEN);
		} else if (strcmp(key, "audio_name") == 0) {
			strncpy((char *)g_func_param.dev_info.audio_name, value, LABEL_LEN);
		} else if (strcmp(key, "rcmode") == 0) {
			if (strcmp(value, "vbr") == 0) {
				g_func_param.isp_attr.encoder_info.rcmode = IMP_ENC_RC_MODE_VBR;
			} else if (strcmp(value, "cbr") == 0) {
				g_func_param.isp_attr.encoder_info.rcmode = IMP_ENC_RC_MODE_CBR;
			} else if (strcmp(value, "fixqp") == 0) {
				g_func_param.isp_attr.encoder_info.rcmode = IMP_ENC_RC_MODE_FIXQP;
			} else if (strcmp(value, "cappedvbr") == 0) {
				g_func_param.isp_attr.encoder_info.rcmode = IMP_ENC_RC_MODE_CAPPED_VBR;
			} else {
				printf("Invalid RC method: %s\n", value);
			}
		} else if (strcmp(key, "nframes") == 0) {
			i = 0;
			uvc_frame_t *yuyvl = NULL;
			uvc_frame_t *jpegl = NULL;
			uvc_frame_t *h264l = NULL;

			nframes = strToInt(value);
			yuyvl = malloc(nframes * sizeof(uvc_frame_t));
			jpegl = malloc(nframes * sizeof(uvc_frame_t));
			h264l = malloc(nframes * sizeof(uvc_frame_t));

			g_func_param.video_info.yuv_frame = yuyvl;
			g_func_param.video_info.jpeg_frame = jpegl;
			g_func_param.video_info.h264_frame = h264l;

			g_func_param.video_info.yuv_frame->frames_num = nframes;
			g_func_param.video_info.jpeg_frame->frames_num = nframes;
			g_func_param.video_info.h264_frame->frames_num = nframes;

			int fps_num = g_func_param.isp_attr.fps_num;
			intervals = 10000000/fps_num;
			while (i < nframes) {
				if (fscanf(uvc_config_fd, "%[^\n]", line_str) < 0)
					break;
				sscanf(line_str, "{%d, %d}", &width, &height);

				if (width > 0 && height > 0 && (width%16 == 0)) {
					yuyvl[i].width = width;
					yuyvl[i].height = height;
					yuyvl[i].intervals = intervals;
					jpegl[i].width = width;
					jpegl[i].height = height;
					jpegl[i].intervals = intervals;
					h264l[i].width = width;
					h264l[i].height = height;
					h264l[i].intervals = intervals;
				} else {
					printf("error(%s %d)Invalid width or height(%d %d)\n", __func__, __LINE__, width, height);
				}
				i++;
				fseek(uvc_config_fd , 1, SEEK_CUR);
			}
		} else {
			printf("Invalid config param: %s\n", key);
		}

	}
	free(line_str);
	return 0;
}

void kiva_load_config_deinit()
{
	fclose(uvc_config_fd);
	uvc_config_fd = NULL;
}

