#ifndef __MODULE_UCAMERA_CONTROL__H
#define __MODULE_UCAMERA_CONTROL__H
#include <stdint.h>
void module_ucamera_enable_impinited();
int module_ucamera_stream_on(void);
int module_ucamera_get_focus_cur();
int module_ucamera_get_focus_auto();
void module_ucamera_post_fs();

int module_ucamera_init(void *param);
void module_ucamera_deinit(void);
#endif

