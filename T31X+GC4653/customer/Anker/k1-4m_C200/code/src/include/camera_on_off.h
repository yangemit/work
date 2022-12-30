#ifndef __CAMERA_ON_OFF__H
#define __CAMERA_ON_OFF__H

#define CAMERA_OFF_PIC_720P      "/system/etc/off_ori_720p.jpg"
#define CAMERA_OFF_PIC_1080P     "/system/etc/off_ori_1080p.jpg"
#define CAMERA_OFF_PIC_2k        "/system/etc/off_ori_2k.jpg"

#define MAX_JPEG_SIZE          (100 * 1024)
#define KEY_DEV                "/dev/key1"
#define CMD_GET_VAL            _IOR('s', 1, int)

void sample_camera_off_pic_init(void);
void sample_camera_off_pic_deinit(void);

int sample_enable_camera_off_pic(int fmt);
void sample_disable_camera_off_pic(int fmt);
void sample_update_resution(unsigned int width,unsigned int height);

int sample_camera_off_pic_key_process(void);
unsigned long sample_camera_off_pic_get_key_val(void);

int sample_get_yuv_snap_2(char *img_buf);
int sample_get_jpeg_snap_2(char *img_buf);
int sample_get_h264_snap_2(char *img_buf);
#endif

