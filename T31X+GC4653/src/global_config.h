#ifndef __GLOBAL_CONFIG__
#define __GLOBAL_CONFIG__

#include <stdint.h>

#define LABEL_LEN  64
#define KIVA_VERSION		"2.0.0"
#define KIVA_VERSION_PATH       "/tmp/version.txt"
#define KIVA_CONFIG_PATH        "/system/config/uvc.config"
#define KIVA_ATTR_PATH          "/system/config/uvc.attr"

#define AUDIO_ENABLE                       1
#define SPKER_ENABLE                       1
#define DMIC_ENABLE                        1
#define MIC_VOL                            70
#define SPK_VOL                            70


/*Audio Info */
typedef struct _audio_info {
	uint8_t audio_en;                  /* audio 使能， 默认开AMIC */
	uint8_t audio_ns;                  /* 音频降噪等级 */
	uint8_t spk_en;                    /* spk 使能 ，和 mic 一起使用*/
	uint8_t dmic_en;                   /* dmic 使能 */
	uint16_t mic_vol;                  /* mic_volume */
	uint16_t spk_vol;                  /* spk_volume */
} audio_info_t;

/*UVC Frame */
typedef struct _uvc_frame {
	uint32_t width;                    /* 宽 */
	uint32_t height;                   /* 高 */
	uint32_t intervals;                /* 帧间隔 */
	uint32_t frames_num;               /* 分辨率格式 */
} uvc_frame_t;


#define ADB_ENABLE                         1
#define STILLCAP_ENABLE                    1
#define H264_ENABLE                        1
#define H265_ENABLE                        1
#define BCSH_ENABLE	1

/*Video Info */
typedef struct _video_info {
	uint8_t adb_en;
	uint8_t stillcap_en;
	uint8_t h264_en;
	uint8_t h265_en;
	uint8_t bcsh_en;
	uint8_t bcsh_low;
	uint8_t bcsh_high;
	uvc_frame_t *yuv_frame;
	uvc_frame_t *jpeg_frame;
	uvc_frame_t *h264_frame;
} video_info_t;

#define VENDOR_ID                          0xA108                /* Ingenic Semiconductor Co.,Ltd */
#define PRODUCT_ID                         0x2231                /* 2022/T31 */
#define BCD_VERSION                        0x0200                /* V2.0.0   */
#define SERIAL_NUMBER                      "1234567890"            /* 1234567890 */
#define PRODUCT_LABEL                      "Ingenic HD Web Camera"
#define MANUFACT_LABEL                     "Ingenic Semiconductor Co.,Ltd"
#define VIDEO_NAME                         "Ingenic HD Web Camera"
#define AUDIO_NAME                         "Ingenic MIC Web Camera"

/* USB Device Info */
typedef struct _device_info {
	uint32_t vendor_id;                    /* VID */
	uint32_t product_id;                   /* PID */
	uint32_t dev_bcd;                      /* 版本号 */
	uint8_t serial_number[LABEL_LEN];      /* SN */
	uint8_t product_label[LABEL_LEN];      /* 厂商名字 */
	uint8_t manufact_label[LABEL_LEN];     /* 厂商名字 */
	uint8_t video_name[LABEL_LEN];         /* 摄像头名字 */
	uint8_t audio_name[LABEL_LEN];         /* 音频名称 */
} device_info_t;

#define AF_ENABLE                              1
#define FOCUS_TRIGGER_VALUE                    15
#define MOTOR_PIXEL                            400
#define MOTOR_STEP                             25
#define MOTOR_STEP_MAX                         600
#define MOTOR_STEP_MIN                         300
#define MOTOR_STEP_TIME                        (80 * 1000)

/*AF Param */
typedef struct _af_param {
	uint8_t af_en;                          /* AF 使能 */
	uint8_t focus_trigger_value;            /* AF 触发值 */
	uint16_t motor_pixel;                   /* 权重范围 */
	uint16_t motor_step;                    /* 电机步长 */
	uint16_t motor_step_max;                /* vcm 行程最小值 */
	uint16_t motor_step_min;                /* vcm 行程最大值 */
	uint32_t motor_sleep_time;              /* 电机移动等待时间 */
} af_param_t;

/* LED Attr */
typedef struct _led_ctl {
	uint8_t level;                          /* 电平属性 */
	int16_t gpio;                           /* GPIO pin */
} led_ctl_t;
#define DEFAULT_SENSOR_NAME		"jxf37"
typedef struct _sensor_info {
	uint16_t sensor_width;                 /* sensor width */
	uint16_t sensor_height;                /* sensor height */
	uint16_t i2c_addr;                     /* i2c_addr */
	uint8_t sensor_name[LABEL_LEN];        /* sesnor_name */
} sensor_info_t;

typedef struct _encoder_info {
	uint8_t rcmode;                        /* 编码模式 */
	uint8_t gop;
	uint8_t qp_value;                      /* qp value */
	uint32_t bitrate;                      /* 码率 */
} encoder_info_t;

/* IMP ISP Attr */
typedef struct _imp_isp_attr {
	uint8_t hvflip;
	uint8_t fps_num;
	uint8_t dynamic_fps;
	sensor_info_t sensor_info;
	encoder_info_t encoder_info;
} imp_isp_attr_t;

/* motor_track param */
#define MOTOR_TRACK_ENABLE                          1
#define MOTOR_TRACK_DIRECTION                       1
#define MOTOR_TRACK_HVFLIP                          1
#define MOTOR_TRACK_LIMIT                           0
#define MOTOR_TRACK_RESET_SPEED                     750
#define MOTOR_TRACK_LENS_HOV                        88
#define MOTOR_TRACK_HMAXSTEP                        4096
#define MOTOR_TRACK_UCAMERA_ENABLE                  1
#define MOTOR_TRACK_RUN_SPEED_PERCENT               1.0
typedef struct _motor_track_attr {
	uint8_t motor_track_en;                     /* 电机是否打开 */
	uint8_t ucamera_en;                         /* ucamera_en */
	uint8_t direction;                          /* 移动方向 */
	uint8_t islimit;                            /* 是否限位 */
	uint16_t reset_speed;                       /* 初始速度 */
	uint16_t lens_hov;                          /* 水平方向视场角 */
	uint8_t hvflip;                             /* 镜像 */
	uint32_t hmaxstep;                          /* 电机转动一圈需要的最大脉冲数 */
	float run_speed_percent;                    /* 电机运行速度百分比 */
} motor_track_attr_t;

#ifdef MODULE_FACEZOOM_ENABLE
typedef struct _facezoom_func_param_t {
	uint8_t facezoom_en;
	uint8_t facezoom_mode;
} facezoom_func_param_t;
#endif

typedef struct _config_func_param {
	device_info_t dev_info;
	audio_info_t audio_info;
	video_info_t video_info;
	imp_isp_attr_t isp_attr;
	af_param_t af_param;
	led_ctl_t led_ctl;
	motor_track_attr_t motor_track_param;
#ifdef MODULE_FACEZOOM_ENABLE
	facezoom_func_param_t facezoom_param;
#endif
} config_func_param_t;

config_func_param_t g_func_param;
#endif
