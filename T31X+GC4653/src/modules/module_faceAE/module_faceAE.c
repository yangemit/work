#include <string.h>
#include <unistd.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>

#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_faceDet.h>
#include <imp/imp_ivs.h>
#include <imp/imp_isp.h>

#include <iaac.h>

#include <imp-common.h>
#include <node_list.h>
#include <module_faceAE.h>


#define MODULE_TAG "SAMPLE-FACEDET"
#define TIME_OUT 1500
#define FACEDET_VIDOE_CH	1

#define ALGO_NODE_NUM               (2)

#define OSD_SHOW_SCALER             (6)
extern void *osd_show_thread(void *arg);
typedef struct _algorithm_ctx {
	/* int stop; */
	/* int lens_hov; */
	/* float run_speed_percent; */
	/* pthread_t pid; */
	/* motor_control_ctx_t motor_ctx; */

#ifdef OSD_CONTROL
	nl_context_t algo_nl_ctx;
	unsigned int algo_nl_node_size;
	int osd_show_scaler;
#endif

} algorithm_ctx_t;

uint8_t g_array[4];
static algorithm_ctx_t algo_ctx;
static int decimal_to_binary(int num, uint8_t array[])
{
	int i = 0;
	memset(array, 0, 4 * sizeof(uint8_t));
	while (num > 0) {
		array[i] = num % 2;
		i = i + 1;
		num = num / 2;
	}
	/* for (i--; i >= 0; i--) */
	/* 	printf("%d", array[i]); */
	/* printf("\n"); */
	return 0;
}

int sample_ivs_facedet_init() {
    int ret = 0;

    IAACInfo ainfo = {
        .license_path = "./license.txt",
        .cid = 1,
        .fid = 1,
        .sn = "ae7117082a18d846e4ea433bcf7e3d2b",
    };

    ret = IAAC_Init(&ainfo);
    if (ret) {
        printf("IAAC_Init error!\n");
        return -1;
    }

    return 0;
}

void sample_ivs_facedet_exit() {

    IAAC_DeInit();
}

static int dump_ispweight()
{
	int i = 0;
	int j = 0;
	int ret = 0;
	IMPISPWeight ae_weight_cur;
	ret = IMP_ISP_Tuning_GetAeWeight(&ae_weight_cur);
	if (ret < 0) {
		printf("get ae failed!\n");
		return -1;
	}
	for (i = 0; i < 15; i++) {
		for (j = 0; j < 15; j++) {
			printf("%d, ", ae_weight_cur.weight[i][j]);
		}
		printf("\n");
	}

}
enum {
	AE_STATUS_KEEP,
	AE_STATUS_ADJUST,
	AE_STATUS_RESET,
	AE_STATUS_RESTORE,
};

typedef struct _rect_t {
	int x;
	int y;
} rect_t;

typedef struct _face_ae_t {
	IMPISPWeight ae_weight_ori;
	uint32_t hilight_ori;
	int status;
	rect_t ul;
	rect_t br;
	rect_t fc;
} face_ae_t;

face_ae_t face_ae;
facedet_param_output_t fo;

uint8_t face_ae_status;

static int ae_adjust_thread(void *arg)
{
	int i = 0, ret = 0;
	ret = IMP_ISP_Tuning_GetAeWeight(&face_ae.ae_weight_ori);
	if (ret < 0) {
		printf("get ae failed!\n");
		return -1;
	}
	while (1) {
		/* printf("=2=>face_ae_status:%d\n", face_ae_status); */
		if (face_ae_status == AE_STATUS_ADJUST) {
			ret = IMP_ISP_Tuning_SetAeWeight(&face_ae.ae_weight_ori);
			if (ret < 0) {
				printf("set ae failed!\n");
				return -1;
			}
		}
		/* if (face_ae_status == AE_STATUS_KEEP) { */
			/* memset(&face_ae.ae_weight_ori, 0, sizeof(face_ae.ae_weight_ori)); */
			/* //face ae flag */
			/* face_ae.ae_weight_ori.weight[0][0] = 1; */
			/* face_ae.ae_weight_ori.weight[0][1] = 0; */
			/* face_ae.ae_weight_ori.weight[0][2] = 1; */
			/* face_ae.ae_weight_ori.weight[0][3] = 0; */
			/* ret = IMP_ISP_Tuning_SetAeWeight(&face_ae.ae_weight_ori); */
			/* if (ret < 0) { */
			/* 	printf("set ae failed!\n"); */
			/* 	return -1; */
			/* } */
		/* } */

		if (face_ae_status == AE_STATUS_RESET) {
			memset(&face_ae.ae_weight_ori, 1, sizeof(face_ae.ae_weight_ori));
			ret = IMP_ISP_Tuning_SetAeWeight(&face_ae.ae_weight_ori);
			if (ret < 0) {
				printf("set ae failed!\n");
				return -1;
			}
		}
		face_ae_status = AE_STATUS_KEEP;

		usleep(5 * 1000);
	}
}


static inline uint64_t get_time(void)
{
	struct timeval tv_date;
	gettimeofday( &tv_date, NULL );
	return( (uint64_t) tv_date.tv_sec * 1000000 + (uint64_t) tv_date.tv_usec );
}

int sample_ivs_facedet_start(int chn_num, IMPIVSInterface **interface) {
    int ret = 0;
    facedet_param_input_t param;

    memset(&param, 0, sizeof(facedet_param_input_t));
    param.frameInfo.width = SENSOR_WIDTH_SECOND;
    param.frameInfo.height = SENSOR_HEIGHT_SECOND;

    param.skip_num = 0;
    param.max_face_box = 10;
    param.sense = 1;         //detection sensibility
    param.detdist = 0;         //detection distance
    param.rot90 = false; // true:图像顺时针旋转90度 false:图像不旋转
    param.switch_liveness = false; // true: 做人脸活体检测; false: 不做人脸活体检测
    param.switch_face_pose = false; // true: 做人脸角度检测; false: 不做人脸角度检测
    param.switch_face_blur = false; // true: 做人脸模糊检测; false: 不做人脸模糊检测

    *interface = FaceDetInterfaceInit(&param);
    if (*interface == NULL) {
        printf("IMP_IVS_CreateGroup failed\n");
        return -1;
    }

    return 0;
}

static uint32_t get_ivs_rect_area(IVSRect* rect)
{
	int xmin, xmax, ymin, ymax;
	xmin = rect->ul.x;
	ymin = rect->ul.y;
	xmax = rect->br.x;
	ymax = rect->br.y;
    if ((xmin >= xmax) || (ymin >= ymax)) {
        return 0;
    } else {
		return (xmax - xmin) * (ymax - ymin);
	}
}

static int facedet_thread(void *arg)
{
	int ret = 0, i = 0;
	unsigned char *sub_nv12_buf = NULL;
	IMPFrameInfo sframe;
	IMPIVSInterface *interface = NULL;
	facedet_param_output_t *result = NULL;

#ifdef OSD_CONTROL
	node_t *node = NULL;
	human_result_t *h_result = NULL;
#endif

	ret = sample_ivs_facedet_start(FACEDET_VIDOE_CH, &interface);
	if (ret < 0) {
		printf("sample_ivs_facedet_start failed\n");
		return -1;
	}

	ret = interface->init(interface);
	if (ret < 0) {
		printf("interface_init_err\n");
		return -1;
	}
	sub_nv12_buf = (unsigned char *)malloc(SENSOR_WIDTH_SECOND * SENSOR_HEIGHT_SECOND * 3 / 2);
	if (!sub_nv12_buf) {
		printf("%s(ERROR): malloc data for sub_nv12_buf error!\n", MODULE_TAG);
		return -1;
	}
	/* Step.6 Get result */
	/* for (i = 0; i < NR_FRAMES_TO_IVS; i++) { */
	for (;;) {
		ret = IMP_FrameSource_SnapFrame(FACEDET_VIDOE_CH, PIX_FMT_NV12, SENSOR_WIDTH_SECOND, SENSOR_HEIGHT_SECOND, sub_nv12_buf, &sframe);
		if (ret < 0) {
			printf("%s(ERROR): IMP_FrameSource_SnapFrame error ret == %d!\n", MODULE_TAG, ret);
			usleep(30 * 1000);
			continue;
		}
		/* ret = interface->setParam(interface, &param); */
		/* if (ret < 0){ */
		/* 	printf("ERROR(%s): getParam error \n", MODULE_TAG); */
		/* 	return -1; */
		/* } */

		uint64_t t1, t2;
		t1 = get_time();
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
		t2 = get_time();
		/* printf("=1=>face_ae_status:%d\n", face_ae_status); */
		/* printf("NETWORK TIME %f ms\n", (t2 - t1) * 1.0/1000); */

		facedet_param_output_t* r = (facedet_param_output_t*)result;
		memcpy(&fo, result, sizeof(fo));
		int i = 0;
		uint32_t x0 = 0, y0 = 0, x1 = 0, y1 = 0, area = 0;
		uint32_t xMinValue = 0, xMaxValue = 0;
		uint32_t yMinValue = 0, yMaxValue = 0;
		float confidence = 0.0;

		uint32_t area_temp;
		float conf_temp;

		for (i = 0; i < r->count; i++) {
			IVSRect* rect = &r->face[i].box;
			/* printf("face location:%d, %d, %d, %d\n", rect->ul.x, rect->ul.y, rect->br.x, rect->br.y); */
			/* printf("face pose:%f, %f, %f\n", r->face[i].face_pose_res.yaw, r->face[i].face_pose_res.pitch, r->face[i].face_pose_res.roll); */
			/* printf("face confidence:%f\n", r->face[i].confidence); */

			area_temp = get_ivs_rect_area(rect);
			conf_temp = r->face[i].confidence;

			if (area < area_temp) {
				x0 = rect->ul.x;
				y0 = rect->ul.y;
				x1 = rect->br.x;
				y1 = rect->br.y;
				confidence = conf_temp;
			} else if (area == area_temp && area != 0) {
				if (confidence < conf_temp) {
					x0 = rect->ul.x;
					y0 = rect->ul.y;
					x1 = rect->br.x;
					y1 = rect->br.y;
					confidence = conf_temp;
				}
			} else {
				// TODO
			}
		}

#ifdef OSD_CONTROL
		/* osd process */
		node = Get_Free_Node(algo_ctx.algo_nl_ctx);
		if (!node) {
			printf("%s(ERROR): should not return NULL!\n", MODULE_TAG);
			continue;
			/* return 0; */
			/* goto exp; */
		}

		memset(node->data, 0, algo_ctx.algo_nl_node_size);
		h_result = (human_result_t *)node->data;
		h_result->human_cnt = r->count;

		/*Step7.2: couvert x,y in uvc */
		for(i = 0; i < r->count; i++) {
			IVSRect* h_rect = &r->face[i].box;
			/* printf("face location:%d, %d, %d, %d\n", h_rect->ul.x, h_rect->ul.y, h_rect->br.x, h_rect->br.y); */
			/* IVSRect* h_rect = &r->rect[i]; */
			h_result->hobj[i].x = algo_ctx.osd_show_scaler * (h_rect->ul.x) * 1.0;
			h_result->hobj[i].y = algo_ctx.osd_show_scaler * (h_rect->ul.y) * 1.0;
			h_result->hobj[i].width = algo_ctx.osd_show_scaler * (h_rect->br.x - h_rect->ul.x) * 1.0;
			h_result->hobj[i].height = algo_ctx.osd_show_scaler * (h_rect->br.y - h_rect->ul.y) * 1.0;
		}

		node->data = h_result;
		Put_Use_Node(algo_ctx.algo_nl_ctx, node);
#endif


		xMinValue = (int)ceil((float)x0/SENSOR_WIDTH_SECOND * 15);
		xMaxValue = (int)floor((float)x1/SENSOR_WIDTH_SECOND * 15);
		yMinValue = (int)ceil((float)y0/SENSOR_HEIGHT_SECOND * 15);
		yMaxValue = (int)floor((float)y1/SENSOR_HEIGHT_SECOND * 15);

		/* printf("xMinValue:%d\n", xMinValue); */
		/* printf("xMaxValue:%d\n", xMaxValue); */
		/* printf("yMinValue:%d\n", yMinValue); */
		/* printf("yMaxValue:%d\n", yMaxValue); */

		if (r->count > 0) {
			face_ae_status = AE_STATUS_ADJUST;
			memset(&face_ae.ae_weight_ori, 0, sizeof(face_ae.ae_weight_ori));
			//face ae flag
			face_ae.ae_weight_ori.weight[0][0] = 0;
			face_ae.ae_weight_ori.weight[0][1] = 1;
			face_ae.ae_weight_ori.weight[0][2] = 1;
			face_ae.ae_weight_ori.weight[0][3] = 0;

			//xMinValue [0~15]
			decimal_to_binary(xMinValue, g_array);
			for (i = 0; i < 4; i++) {
				face_ae.ae_weight_ori.weight[1][i] = g_array[i];
			}

			//yMinValue [0~15]
			decimal_to_binary(yMinValue, g_array);
			for (i = 0; i < 4; i++) {
				face_ae.ae_weight_ori.weight[2][i] = g_array[i];
			}

			//xMaxValue [0~15]
			decimal_to_binary(xMaxValue, g_array);
			for (i = 0; i < 4; i++) {
				face_ae.ae_weight_ori.weight[3][i] = g_array[i];
			}

			//yMaxValue [0~15]
			decimal_to_binary(yMaxValue, g_array);
			for (i = 0; i < 4; i++) {
				face_ae.ae_weight_ori.weight[4][i] = g_array[i];
			}


		} else {
			face_ae_status = AE_STATUS_RESET; // norm ae
		}


		ret = interface->releaseResult(interface, (void **)&result);
		if (ret < 0) {
			printf("%s(ERROR): releaseResult error!\n", MODULE_TAG);
			usleep(30 * 1000);
			continue;
		}
		usleep(100 * 1000);
	}
	return 0;
}

#ifdef OSD_CONTROL
int module_algorithm_process_get_result(human_result_t **result)
{
	/* human_result_t *h_result = NULL; */
	node_t *node = NULL;
	node = Get_Use_Node(algo_ctx.algo_nl_ctx);
	if (!node) {
		printf("ERROR(%s): Get_Use_Node error!\n", MODULE_TAG);
		return 0;
	}

	*result = (human_result_t *)node->data;

	return (int)node;
}

void module_algorithm_process_put_result(int node)
{
	Put_Free_Node(algo_ctx.algo_nl_ctx, (node_t *)node);
}
#endif


int module_faceAE_init()
{
	printf("face detect\n");
	int ret = 0, i = 0;


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

	ret = sample_ivs_facedet_init();
	if (ret < 0) {
		printf("sample_ivs_facedet_init failed\n");
		return -1;
	}
	/* ret = algorithm_video_init(FACEDET_VIDOE_CH, IVS_MODE_BIND); */
	ret = algorithm_video_init(FACEDET_VIDOE_CH, IVS_MODE_UNBIND);
	if (ret) {
		printf("ERROR(%s): algorithm_video_stream_init error!\n", MODULE_TAG);
		return -1;
	}

#ifdef OSD_CONTROL
	pthread_t osd_pid;
	pthread_attr_t osd_attr;
	pthread_attr_init(&osd_attr);
	pthread_attr_setdetachstate(&osd_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&osd_attr, SCHED_OTHER);
	ret = pthread_create(&osd_pid, &osd_attr, &osd_show_thread, NULL);
	if (ret) {
		printf("ERROR(%s): create thread for osd_show failed!\n", MODULE_TAG);
		return -1;
	}
#endif

	pthread_t facedet_tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
	ret = pthread_create(&facedet_tid, &attr, &facedet_thread, NULL);
	if (ret) {
		printf("ERROR(%s): create thread for algorithm_process_thread failed!\n", MODULE_TAG);
		return -1;
	}
	printf("%s: create thread for algorithm_process_thread\n", MODULE_TAG);

	pthread_t ae_adjust_tid;
	/* pthread_attr_t attr; */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
	ret = pthread_create(&ae_adjust_tid, &attr, &ae_adjust_thread, NULL);
	if (ret) {
		printf("ERROR(%s): create thread for ae_adjust_thread failed!\n", MODULE_TAG);
		return -1;
	}


    return 0;
}

int module_faceAE_deinit(IMPIVSInterface *interface)
{
	int ret = 0;
	ret = algorithm_video_deinit(FACEDET_VIDOE_CH, IVS_MODE_UNBIND);
	if (ret) {
		printf("ERROR(%s): video_stream_deinit error!\n", MODULE_TAG);
		return -1;
	}

	sample_ivs_facedet_exit();

	return 0;
}
