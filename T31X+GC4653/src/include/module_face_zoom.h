#ifndef __MODULE_FACEZOOM_CONTROL__H
#define __MODULE_FACEZOOM_CONTROL__H
#include <global_config.h>

#define SINGLE_MODE	1
#define MULTI_MODE	2

int module_facezoom_process(void);
void module_facezoom_enable_impinited(uint8_t on);
int module_facezoom_init(imp_isp_attr_t isp_param, facezoom_func_param_t fz_param);
int module_facezoom_deinit(facezoom_func_param_t fz_param);
void set_face_zoom_mode(int mode);
void set_face_zoom_switch(int on_off);
int get_facezoom_switch_class(void);
#endif

