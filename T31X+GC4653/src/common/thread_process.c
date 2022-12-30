/*
 * kiva.c
 *
 * Copyright (C) 2022 Ingenic Semiconductor Co.,Ltd
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>

//#include <module_led_control.h>
#include <module_ucamera_control.h>
#ifdef MOTOR_TRACK_ENABLE
#include <module_track_control.h>
#endif
#ifdef MODULE_FACEZOOM_ENABLE
#include <module_face_zoom.h>
#endif
#include <imp-common.h>
#include "main.h"
#include "global_config.h"

#define MOTAG_TAG            "KIVA_PROCRSS"

//static pthread_t led_pid;
static pthread_t dyn_pid;

static int g_fps_num;

/*static void *led_control_process(void *arg)
{
	int stream_on = 0;
	led_ctl_t *led_ctl = (led_ctl_t *)arg;
	prctl(PR_SET_NAME, "led_control_process");

	while (1) {
		stream_on = module_ucamera_stream_on();
		if (stream_on) {
			module_led_ctl(led_ctl->gpio, led_ctl->level);
		} else {
			module_led_ctl(led_ctl->gpio, !led_ctl->level);
		}
		usleep(100 * 1000);
	}

	return NULL;
}*/

static int imp_sensor_fps_adapture()
{
	int ret, ev_cur = 0;
	int sensor_fps = 0;
	int stream_on = 0;
	IMPISPEVAttr evattr = {0};

	stream_on = module_ucamera_stream_on();
	if (!stream_on)
		return 0;

	ret = IMP_ISP_Tuning_GetEVAttr(&evattr);
	if (ret < 0) {
		printf("ERROR(%s): failed to get evattr\n", MOTAG_TAG);
		return -1;
	}

	printf("INFO(%s): IMP get Ev:%d\n", MOTAG_TAG, evattr.ev);

	if (evattr.ev > 8000)
		ev_cur = 2;
	else if (evattr.ev > 4000)
		ev_cur = 1;
	else
		ev_cur = 0;

	switch (ev_cur) {
	case 2:
		sensor_fps = 12;
		break;
	case 1:
		sensor_fps = 15;
		break;
	case 0:
		sensor_fps = g_fps_num;
		break;
	default:
		return 0;
	}

	ret = IMP_ISP_Tuning_SetSensorFPS(sensor_fps, 1);
	if (ret < 0) {
		printf("ERROR(%s): failed to set sensor fps\n", MOTAG_TAG);
		return -1;
	}

	return 0;

}

static void *dynamic_fps_process(void *arg)
{
	int stream_on = 0;

	prctl(PR_SET_NAME, "dynamic_fps_process");

	while (1) {
		stream_on = module_ucamera_stream_on();
		if (stream_on) {
			imp_sensor_fps_adapture();
		}

		usleep(2 * 1000 * 1000);
	}

	return NULL;
}

#ifdef OSD_CONTROL
static int osd_stop = 0;
static pthread_t osd_pid;
extern void osd_render(int x, int y, int w, int h);
extern void osd_render_cls(void);
void *osd_show_process(void *arg)
{
	int r_node;
	human_result_t *result = NULL;

	prctl(PR_SET_NAME, "osd_show");

	/* Initialize faceDet algorithm */

	while (!osd_stop) {
		/*Step1: get tesult from algorithm */
		r_node = module_track_get_result((void **)&result);
		if (!r_node) {

			osd_render_cls();
			usleep(100 * 1000);
			continue;
		}

		osd_render_cls();

		if (module_ucamera_stream_on()) {
			/*Step2: Draw OSD region to ch0 */
			for (int i = 0; i < result->human_cnt; i++) {
				osd_render(result->hobj[i].x, result->hobj[i].y, result->hobj[i].width, result->hobj[i].height);
			}
		}

		/* Give back algorithm node */
		module_track_put_result(r_node);
	}

	return NULL;
}
#endif

#ifdef MODULE_FACEZOOM_ENABLE
static int face_zoom_stop = 0;
static pthread_t fz_pid;
extern int key_type;
void *face_zoom_control_process(void *none)
{
	uint8_t fz_status = 0;
	int ret = -1;
	
	prctl(PR_SET_NAME, "face_zoom_control");
	while(!face_zoom_stop)
	{
		switch(fz_status){
			case 0:
				if (module_ucamera_stream_on()) {
					printf("INFO(%s): module_facezoom_init ...\n", MOTAG_TAG);
					ret = module_facezoom_init(g_func_param.isp_attr, g_func_param.facezoom_param);
					if (ret < 0) {
						printf("error(%s): module_face_zoom_init failed \n", MOTAG_TAG);
						return NULL;
					}
					printf("INFO(%s): module_facezoom_init ...ok\n", MOTAG_TAG);

					module_ucamera_post_fs();
					fz_status = 1;
				}
				break;
			case 1:
				if (!module_ucamera_stream_on()) {
					printf("INFO(%s): module_facezoom_deinit ...\n", MOTAG_TAG);
					ret = module_facezoom_deinit(g_func_param.facezoom_param);
					if (ret < 0) {
						printf("error(%s): module_face_zoom_deinit failed \n", MOTAG_TAG);
						return NULL;
					}
					printf("INFO(%s): module_facezoom_deinit ...ok\n", MOTAG_TAG);

					module_ucamera_post_fs();
					key_type = 0;
					fz_status = 0;
				}
				break;
			default:
				break;
		}
		usleep(200 * 1000);
		/*if (!module_ucamera_stream_on()) {*/
			/*sleep(1);*/
			/*continue;*/
		/*}*/
		/*module_facezoom_process();*/
	}

	return NULL;
}
#endif

int kiva_process_init(void *param)
{
	int ret = -1;
	config_func_param_t *cfg_param;
	cfg_param = (config_func_param_t *)param;
	g_fps_num = cfg_param->isp_attr.fps_num;

	/* start osd clean thread */
/*	pthread_attr_t led_attr;
	pthread_attr_init(&led_attr);
	pthread_attr_setdetachstate(&led_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&led_attr, SCHED_OTHER);
	ret = pthread_create(&led_pid, &led_attr, &led_control_process, &cfg_param->led_ctl);
	if (ret) {
		printf("ERROR(%s): create thread for led control failed!\n", MOTAG_TAG);
		return -1;
	}
*/
	if (cfg_param->isp_attr.dynamic_fps) {
		/* start osd clean thread */
		pthread_attr_t dyn_attr;
		pthread_attr_init(&dyn_attr);
		pthread_attr_setdetachstate(&dyn_attr, PTHREAD_CREATE_DETACHED);
		pthread_attr_setschedpolicy(&dyn_attr, SCHED_OTHER);
		ret = pthread_create(&dyn_pid, &dyn_attr, &dynamic_fps_process, NULL);
		if (ret) {
			printf("ERROR(%s): create thread for dynamic_fps failed!\n", MOTAG_TAG);
			return -1;
		}
	}

#ifdef OSD_CONTROL
	pthread_attr_t osd_attr;
	pthread_attr_init(&osd_attr);
	pthread_attr_setdetachstate(&osd_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&osd_attr, SCHED_OTHER);
	ret = pthread_create(&osd_pid, &osd_attr, &osd_show_process, NULL);
	if (ret) {
		printf("ERROR(%s): create thread for osd show failed!\n", MOTAG_TAG);
		return -1;
	}

#endif

#ifdef MODULE_FACEZOOM_ENABLE
	pthread_attr_t fz_attr;
	pthread_attr_init(&fz_attr);
	pthread_attr_setdetachstate(&fz_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&fz_attr, SCHED_OTHER);
	ret = pthread_create(&fz_pid, &fz_attr, &face_zoom_control_process, NULL);
	if (ret) {
		printf("ERROR(%s): create thread for face_zoom_control_process failed!\n", MOTAG_TAG);
		return -1;
	}

#endif

	return 0;

}

void kiva_process_deinit()
{
//	pthread_cancel(led_pid);
	pthread_cancel(dyn_pid);
#ifdef OSD_CONTROL
	pthread_cancel(osd_pid);
#endif
#ifdef MODULE_FACEZOOM_ENABLE
	pthread_cancel(fz_pid);
#endif
	printf("INFO(%s): kiva_process_deinit ...ok\n", MOTAG_TAG);
}
