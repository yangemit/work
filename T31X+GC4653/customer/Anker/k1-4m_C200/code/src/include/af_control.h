#ifndef __AF_CONTROL__H
#define __AF_CONTROL__H
#include <stdint.h>

#define VCM_DEV_NAME        "/dev/dw9714"
#define MOTOR_PIXEL4M       400
#define MOTOR_PIXEL2M       200

#define LOWPOWER_TIMESTAMP  (30 * 1000000) /*30s */

#define MOTOR_MOVE          _IOW('M', 1, int)

#if 0
extern int g_motor_pixel;
extern int g_motor_step;
extern int g_motor_sleep_time;
extern int g_motor_step_max;
extern int g_motor_step_min;
extern int g_focus_trigger_value;
#endif

typedef struct _vcm_motor_param {
	int af_en;                   /*af 使能 */
	int af_cur;                  /*af 当前值 */
	int motor_pixel;             /* 2M 或者4M 权重*/
	int motor_step;              /* 马达移动范围 */
	int motor_step_min;          /* 马达行程最小值 */
	int motor_step_max;          /* 马达行程最大值 */
	int focus_trigger_value;     /* af 触发阈值 */
	int motor_sleep_time;         /* 采样等待时间 */
} vcm_motor_param_t;

void *sample_af_control_process(void *args);

void sample_af_control_vcm_param_init(void *param);
#endif

