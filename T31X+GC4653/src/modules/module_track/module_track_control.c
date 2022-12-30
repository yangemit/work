#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <imp-common.h>
#include <imp/imp_ivs.h>
#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_person_tracker.h>

#include <module_track_control.h>
#include <signal.h>
#include <motor_process.h>
#include <node_list.h>
#include "motor_control.h"
#include <global_config.h>

#define MODULE_TAG                  "MOTOR_TRACK"

/* algorithm params */
#define ALGO_NODE_NUM               (2)

#define OSD_SHOW_SCALER             (3)

#define EXT_FUNC_ENABLE             (1)
#define EXT_FUNC_DISABLE            (0)
#define MOTOR_TRACE_STATUS           0
#define MOTOR_PAUSE_STATUS           1
#define MOTOR_OLDMODE_STATUS         2

typedef struct _algorithm_ctx {
	int motor_track_en;
	int stop;
	int lens_hov;
	float run_speed_percent;
	pthread_t pid;
	motor_control_ctx_t motor_ctx;

#ifdef OSD_CONTROL
	nl_context_t algo_nl_ctx;
	unsigned int algo_nl_node_size;
	int osd_show_scaler;
#endif

} algorithm_ctx_t;

typedef struct _ext_func_t {
	int sig;
	int pause;
	int old_mode;
	int trace;

	int uart_ctl;
	int *uart_cmd;
	pthread_t sig_pid;
	pthread_t ext_func_pid;
} ext_func_t;

static algorithm_ctx_t algo_ctx;
static int person_tracker_init(IMPIVSInterface **interface)
{
	person_tracker_param_input_t param;

	memset(&param, 0, sizeof(person_tracker_param_input_t));
	param.frameInfo.width = SENSOR_WIDTH_SECOND;
	param.frameInfo.height = SENSOR_HEIGHT_SECOND;
#if 0
	param.stable_time_out = 4;
	param.move_sense = 4;
	param.obj_min_width = 40;
	param.obj_min_height = 40;
	param.move_thresh_denoise = 20;
	param.add_smooth = 0;
	param.conf_thresh = 0.8;
	param.conf_lower = 0.2;
	param.frozen_in_boundary = 5;
	param.boundary_ratio = 0.25;
	param.mode = 1;
#endif

	param.stable_time_out = 4;
	param.move_sense = 2;
	param.obj_min_width = 30;
	param.obj_min_height = 30;
	param.move_thresh_denoise = 20;
	param.add_smooth = 0;
	param.conf_thresh = 0.8;
	param.conf_lower = 0.2;
	param.frozen_in_boundary = 5;
	param.boundary_ratio = 0.25;
	param.mode = 1;
	/* param.enable_move = true; */
	param.sense = 4;
	param.detdist = 0;                         /*0: 6m, 1: 8m */
	param.ptime = false;
	param.area_ratio = 0.8;
	param.exp_x = 1;

	param.check_freq = 4;

	*interface = Person_TrackerInterfaceInit(&param);
	if (*interface == NULL) {
		printf("Person_TrackerInterfaceInit failed\n");
		return -1;
	}

	return 0;
}

static void person_track_deinit(IMPIVSInterface *interface)
{
	Person_TrackerInterfaceExit(interface);
}

static void *algorithm_process_thread(void *arg)
{
	int ret = 0;
	IMPIVSInterface *interface = NULL;
	IMPFrameInfo sframe;
	unsigned char *sub_nv12_buf = NULL;

#ifdef OSD_CONTROL
	int i = 0;
	node_t *node = NULL;
	human_result_t *h_result = NULL;
#endif
	person_tracker_param_output_t *result = NULL;
	person_tracker_param_input_t param;
	motor_message_t jb_motor_message;

	prctl(PR_SET_NAME, "algorithm_process");

	/* Initialize faceDet algorithm */
	ret = person_tracker_init(&interface);
	if (ret < 0) {
		printf("%s(ERROR): person_track_init!\n", MODULE_TAG);
		return NULL;
	}

	ret = interface->init(interface);
	if (ret < 0) {
		printf("interface_init_err\n");
		return NULL;
	}

	sub_nv12_buf = (unsigned char *)malloc(SENSOR_WIDTH_SECOND * SENSOR_HEIGHT_SECOND * 3 / 2);
	if (!sub_nv12_buf) {
		printf("%s(ERROR): malloc data for sub_nv12_buf error!\n", MODULE_TAG);
		goto exp;
	}

	/* check ivs version */
	uint32_t persontracker_ver = person_tracker_get_version_info();
	if(persontracker_ver !=PERSONTRACKER_VERSION_NUM){
		printf("The version numbers of head file and lib do not match, head file version: %08x, lib version: %08x\n",PERSONTRACKER_VERSION_NUM, persontracker_ver);
		return (void*)-1;
	}
	/* check ivs version */

	printf("algo_ctx.stop == %d\n", algo_ctx.stop);
	while (!algo_ctx.stop) {
		ret = IMP_FrameSource_SnapFrame(ALGORITHM_VIDEO_CH, PIX_FMT_NV12, SENSOR_WIDTH_SECOND, SENSOR_HEIGHT_SECOND, sub_nv12_buf, &sframe);
		if (ret < 0) {
			printf("%s(ERROR): IMP_FrameSource_SnapFrame error ret == %d!\n", MODULE_TAG, ret);
			usleep(30 * 1000);
			continue;
		}

		memset(&jb_motor_message, 0, sizeof(motor_message_t));
		ret = motor_control_get_status(&jb_motor_message);
		if (ret < 0) {
			printf("ERROR(%s): ioctl MOTOR_GET_STATUS error \n", MODULE_TAG);
			return NULL;
		}
		param.is_motor_stop = ((MOTOR_IS_STOP == jb_motor_message.status)? 1 : 0);
		param.is_feedback_motor_status = 1;

		ret = interface->setParam(interface, &param);
		if (ret < 0){
			printf("ERROR(%s): getParam error \n", MODULE_TAG);
			return NULL;
		}

		sframe.virAddr = (unsigned int)sub_nv12_buf;
		ret = interface->preProcessSync(interface, &sframe);
		if (ret < 0) {
			printf("%s(ERROR): preProcessSync error!\n", MODULE_TAG);
			usleep(30 * 1000);
			continue;
		}

		ret = interface->processAsync(interface, &sframe);
		if (ret < 0) {
			printf("%s(ERROR): processSync error!\n", MODULE_TAG);
			usleep(30 * 1000);
			continue;
		}

		ret = interface->getResult(interface, (void **)&result);
		if (ret < 0) {
			printf("%s(ERROR): getResult error!\n", MODULE_TAG);
			usleep(30 * 1000);
			continue;
		}

		person_tracker_param_output_t relt;
		person_tracker_param_output_t *r = &relt;
		memset(&relt, 0, sizeof(person_tracker_param_output_t));
		memcpy(r, result, sizeof(person_tracker_param_output_t));

		ret = interface->releaseResult(interface, (void **)&result);
		if (ret < 0) {
			printf("%s(ERROR): releaseResult error!\n", MODULE_TAG);
			usleep(30 * 1000);
			continue;
		}

#ifdef OSD_CONTROL
		/* osd process */
		node = Get_Free_Node(algo_ctx.algo_nl_ctx);
		if (!node) {
			printf("%s(ERROR): should not return NULL!\n", MODULE_TAG);
			goto exp;
		}

		memset(node->data, 0, algo_ctx.algo_nl_node_size);
		h_result = (human_result_t *)node->data;
		h_result->human_cnt = r->count;

		/* IMP_LOG_INFO(MODULE_TAG, "frame[%d], result->count=%d, dx=%d, dy=%d, step=%d\n", i, result->count, result->dx, result->dy, result->step); */
		/*Step7.2: couvert x,y in uvc */
		for(i = 0; i < r->count; i++) {
			IVSRect* h_rect = &r->rect[i];
			h_result->hobj[i].x = algo_ctx.osd_show_scaler * (h_rect->ul.x) * 1.0;
			h_result->hobj[i].y = algo_ctx.osd_show_scaler * (h_rect->ul.y) * 1.0;
			h_result->hobj[i].width = algo_ctx.osd_show_scaler * (h_rect->br.x - h_rect->ul.x) * 1.0;
			h_result->hobj[i].height = algo_ctx.osd_show_scaler * (h_rect->br.y - h_rect->ul.y) * 1.0;
		}

		node->data = h_result;
		Put_Use_Node(algo_ctx.algo_nl_ctx, node);
#endif
		/*motor process */
		/* static int tmp = 0; */
		/* if (tmp == 50 || r->count) { */
		/* 	printf("r->count == %d\n", r->count); */
		/* 	tmp = 0; */
		/* } */
		/* tmp++; */
		if (r->count > 0) {
			MotorTrackProcess(r, algo_ctx.lens_hov, algo_ctx.run_speed_percent);
		}
	}

	/* Step8: deinit */
exp:
	free(sub_nv12_buf);
	person_track_deinit(interface);

	return NULL;
}

static int isp_set_hvflip(int hv)
{
	int ret = 0;

	ret = IMP_ISP_Tuning_SetHVFLIP(hv);
	if (ret < 0){
		printf("ERROR(%s): failed to set HVflip mode\n", MODULE_TAG);
		return -1;
	}
	usleep(100*1000);
	return 0;

}

static void *motor_init_thread(void *arg)
{
	int ret = -1;
	ret = motor_control_init(arg);
	if (ret) {
		printf("ERROR(%s): motor_control_init !\n", MODULE_TAG);
		return NULL;
	}
	return NULL;
}

static int motor_self_check(void *arg)
{
	int ret = -1;
	pthread_t motor_pid;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
	ret = pthread_create(&motor_pid, &attr, &motor_init_thread, arg);
	if (ret) {
		printf("ERROR(%s): create thread for motor_init_thread failed!\n", MODULE_TAG);
		return -1;
	}

	return 0;
}

#ifdef OSD_CONTROL
int module_track_get_result(void **result)
{
	human_result_t **h_result = (human_result_t **)result;
	node_t *node = NULL;
	node = Get_Use_Node(algo_ctx.algo_nl_ctx);
	if (!node) {
		printf("ERROR(%s): Get_Use_Node error!\n", MODULE_TAG);
		return 0;
	}

	*h_result = (human_result_t *)node->data;

	return (int)node;
}

void module_track_put_result(int node)
{
	Put_Free_Node(algo_ctx.algo_nl_ctx, (node_t *)node);
}
#endif

#if 0
int module_algorithm_process_motor_init(const void *param)
{
	return motor_control_init(param);
}
#endif

int module_track_init(void* param)
{
	int ret = -1;
	motor_track_attr_t *_param = (motor_track_attr_t *)param;

	memset(&algo_ctx, 0, sizeof(algorithm_ctx_t));
	if (_param->motor_track_en) {
		algo_ctx.motor_ctx.motor_reset_speed = _param->reset_speed;
		algo_ctx.motor_ctx.islimit = _param->islimit;
		algo_ctx.motor_ctx.hmaxstep = _param->hmaxstep;
		algo_ctx.motor_ctx.motor_direction = _param->direction;
		algo_ctx.run_speed_percent = _param->run_speed_percent;
		algo_ctx.lens_hov = _param->lens_hov;
		algo_ctx.motor_track_en = _param->motor_track_en;

#ifdef OSD_CONTROL
		/*1080P */
		algo_ctx.osd_show_scaler = OSD_SHOW_SCALER;
		algo_ctx.algo_nl_node_size = sizeof(human_result_t);
		algo_ctx.algo_nl_ctx = Node_List_Init(algo_ctx.algo_nl_node_size, ALGO_NODE_NUM, 0, 1);
		if (!algo_ctx.algo_nl_ctx) {
			printf("ERROR(%s): Node_List_Init failed.\n", MODULE_TAG);
			return -1;
		}
#endif

		/* create video stream for algorithm process */
#if 0
#ifdef ALGO_AUTHORIZE
	ret = jz_iaac_init();
	if (ret < 0) {
		printf("ERROR(%s): jz_iaac_init failed, please get license!\n", TAG);
		return -1;
	}
#endif
#endif

		/* motor self check */
		motor_self_check(param);

		ret = algorithm_video_init(ALGORITHM_VIDEO_CH, IVS_MODE_UNBIND);
		if (ret) {
			printf("ERROR(%s): algorithm_video_stream_init error!\n", MODULE_TAG);
			return -1;
		}

		int hv = _param->hvflip;
		ret = isp_set_hvflip(hv);
		if (ret) {
			printf("ERROR(%s): algo isp tuning error!\n", MODULE_TAG);
			return -1;
		}

		/* create algorithm process thread */
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
		ret = pthread_create(&algo_ctx.pid, &attr, &algorithm_process_thread, NULL);
		if (ret) {
			printf("ERROR(%s): create thread for algorithm_process_thread failed!\n", MODULE_TAG);
			return -1;
		}
		printf("%s: create thread for algorithm_process_thread\n", MODULE_TAG);

	}

	return 0;
}

void module_track_deinit(void)
{
	if (algo_ctx.motor_track_en) {
		int ret = -1;

		algo_ctx.stop = 1;
		pthread_cancel(algo_ctx.pid);

		/* destroy video stream for algorithm process */
		ret = algorithm_video_deinit(ALGORITHM_VIDEO_CH, IVS_MODE_UNBIND);
		if (ret) {
			printf("ERROR(%s): video_stream_deinit error!\n", MODULE_TAG);
			return ;
		}

	}
	return;
}
