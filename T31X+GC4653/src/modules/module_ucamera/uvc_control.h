#ifndef __UVC_CONTROL__H
#define __UVC_CONTROL__H
#include <imp/imp_osd.h>
int uvc_stream_on(void);
void uvc_enable_impinited(int on);
int uvc_get_focus_cur();
int uvc_get_focus_auto();
void uvc_post_fs_sem();

int uvc_control_init(void *param);
void uvc_control_deinit();

IMPRgnHandle *prHander_interface(void);
int get_osd_width(void);
int get_osd_height(void);
int get_frame_fmt(void);
int get_frame_switch(void);
#endif
