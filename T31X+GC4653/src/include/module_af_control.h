#ifndef __MODULE_AF_CONTROL__H
#define __MODULE_AF_CONTROL__H
#include <stdint.h>
#include <global_config.h>

#define VCM_DEV_NAME        "/dev/dw9714"
#define MOTOR_PIXEL4M       400
#define MOTOR_PIXEL2M       200

#define LOWPOWER_TIMESTAMP  (30 * 1000000) /*30s */

#define MOTOR_MOVE          _IOW('M', 1, int)


int module_autofocus_init(void *param);
void module_autofocus_deinit(void);
#endif

