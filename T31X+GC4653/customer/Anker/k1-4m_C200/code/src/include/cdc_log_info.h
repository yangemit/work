#ifndef __CDC_LOG_INFO__H
#define __AF_CONTROL__H

#include <stdint.h>

#define CDC_LOG_FILE          "/tmp/cdc_log.txt"

void cdc_log_init();
void cdc_log_deinit();

void cdc_log_fun(int err_code, int line, const char*func, const char *fmt, ...);
int cdc_log_write();

#define CDC_LOG(err_code, fmt, ...) \
	cdc_log_fun(err_code, __LINE__, __func__, fmt, ##__VA_ARGS__)

/* ERROR code */
#define UCAMERA_LOG_INFO                 0x0
#define UCAMERA_LOAD_CONFIG_ERROR        0x1
#define UCAMERA_LOAD_ATTR_ERROR          0x2

#define IMP_POLLING_RESULT_ERROR         0x11
#define IMP_ENCODER_GET_STREAM_ERROR     0x12
#define IMP_FRAMESOURCE_GET_STREAM_ERROR 0x13
#define IMP_AI_POLLING_RESULT_ERROR      0x14
#define IMP_AI_GET_FRAME_ERROR           0x15
#define IMP_SET_AF_WEIGHT_ERROR          0x16
#define IMP_GET_AF_WEIGHT_ERROR          0x17

#define UVC_STREAMON_ERROR               0x21
#define UVC_STREAMOFF_ERROR              0x22
#define UVC_SET_FOV_ERROR                0x23
#define UVC_SET_ZOOM_ERROR               0x24
#define UVC_SET_PANTILT_ERROR            0x25
#define UVC_SET_HVFLIP_ERROR             0x26
#define UVC_AUTO_FOCUS_ERROR             0x27

#endif

