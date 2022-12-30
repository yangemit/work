/*********************************************************
 * File Name   : face_zoom.c
 * Author      : Tania Xiang
 * Mail        : xiuhui.xiang@ingenic.com
 * Created Time: 2022-01-26 15:25
 ********************************************************/

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <math.h>
#include <pthread.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>

#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_faceDet.h>
#include <imp/imp_ivs.h>

#include <node_list.h>

#include <imp-common.h>
#include <global_config.h>
#include <module_face_zoom.h>

#define TAG "SAMPLE-FACEDET"
#define TIME_OUT 1500

#define MAX_NUM_FACE	NUM_OF_FACES
#define ALGO_CHN_SCALER	3

static pthread_t facedet_pid;
static pthread_t facezoom_pid;
static pthread_t facezoom_switch_pid; //算法开关线程
static int facedet_stop = 0;

static nl_context_t face_zoom_nl;
static float last_scaler = 1.0;
static int face_width = 0;
static int face_width_tmp = 0;
static int face_width_last = 0;
static int anti_shake_cnt = 0;
static int facezoom_mode_empty = 0;

static int last_fcrop_left = 0;
static int last_fcrop_top = 0;
static int last_face_center_x = 0;
static int last_face_center_y = 0;

static int face_zoom_mode;   //模式切换开关
static int facezoom_switch; //算法开关
static int facezoom_switch_class; //实现算法开关
static uint8_t face_zoom_impinited = 0;

static uint16_t g_VideoWidth = 1920;
static uint16_t g_VideoHeight = 1080;
static uint8_t g_HV_Flip = 3;

#define VALUE_INTERVAL 		(0.01)
#define VALUE_LEVEL_1_0		1		// 1 scaler
#define VALUE_LEVEL_1_3		1.3		// 1.3 scaler
#define VALUE_LEVEL_1_3_1	1.31 	// 1.31 scaler
#define VALUE_LEVEL_1_5		1.5		// 1.5 scaler
#define VALUE_LEVEL_2_0		2		// 2.0 scaler
#define VALUE_LEVEL_2_3		2.3		// 2.3 scaler
#define VALUE_LEVEL_2_5		2.5		// 2.5 scaler
#define VALUE_LEVEL_3_0		3		// 3.0 scaler

typedef struct{
	int face_center_x;
	int face_center_y;
	int face_width;
	int face_height;
	int track_id;
}face_zoom_t;

IMPIVSInterface *interface = NULL;

void set_face_zoom_mode(int mode)
{
	face_zoom_mode = mode;	
}

void set_face_zoom_switch(int on_off)
{
	facezoom_switch = on_off;	
}


int get_facezoom_switch_class(void)
{
	return facezoom_switch_class;	
}
static int sample_ivs_facedet_start(int grp_num, int chn_num, IMPIVSInterface **interface) {
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
	param.switch_face_pose = true; // true: 做人脸角度检测; false: 不做人脸角度检测
	param.switch_face_blur = true; // true: 做人脸模糊检测; false: 不做人脸模糊检测

	*interface = FaceDetInterfaceInit(&param);
	if (*interface == NULL) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}

	ret = IMP_IVS_CreateChn(chn_num, *interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_RegisterChn(grp_num, chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
		return -1;
	}

	ret = IMP_IVS_StartRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StartRecvPic(%d) failed\n", chn_num);
		return -1;
	}

	return 0;
}

static int sample_ivs_facedet_stop(int chn_num, IMPIVSInterface *interface) {
	int ret = 0;

	ret = IMP_IVS_StopRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StopRecvPic(%d) failed\n", chn_num);
		return -1;
	}
	usleep( 500 * 1000 );

	ret = IMP_IVS_UnRegisterChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_UnRegisterChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_DestroyChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_DestroyChn(%d) failed\n", chn_num);
		return -1;
	}

	FaceDetInterfaceExit(interface);

	return 0;
}

static int face_zoom_set(float scaler_level, int face_center_x, int face_center_y)
{
	int ret;
	uint16_t zoomwidth_cur,zoomheight_cur;
	uint16_t zoomleft_cur,zoomtop_cur;
	IMPISPFrontCrop fcrop_obj;
	float dir_value = 0.0;
	float value_cur = 1.0;
	int move_x_offset = abs(last_face_center_x - face_center_x);
	int move_y_offset = abs(last_face_center_y - face_center_y);
	int face_center_x_cur;
	int face_center_y_cur;
	float move_x_step = 0.0;
	float move_y_step = 0.0;

	dir_value =  scaler_level;
	value_cur = last_scaler;

	/* zoom */
	if((last_scaler - dir_value) > 0.0001){// last_scaler > dir_value
		value_cur = last_scaler - VALUE_INTERVAL;
	}else if((dir_value - last_scaler) > 0.0001){ // dir_value > last_scaler
		value_cur = last_scaler + VALUE_INTERVAL;
	}

	zoomwidth_cur = round(g_VideoWidth/value_cur); // sqrt(value_cur));
	zoomwidth_cur = round((float)(zoomwidth_cur/4)) * 4;
	zoomheight_cur = round(g_VideoHeight/value_cur); // sqrt(value_cur));
	zoomheight_cur = round((float)(zoomheight_cur/4)) * 4;

	/* PT */
	/*move_x_offset > move_x_step * 4 */
	if(move_x_offset > 20){
		move_x_step = ((move_x_offset / 25) > 6) ? (move_x_offset / 25) : 6;
		move_x_step = round(move_x_step / 2) * 2; // step must be even
	}
	/*move_y_offset > move_x_step * 4 */
	if(move_y_offset > 20){
		move_y_step = ((move_y_offset / 25) > 6) ? (move_y_offset / 25) : 6;
		move_y_step = round(move_y_step / 2) * 2; // step must be even
	}

	if(last_face_center_x > face_center_x){
		face_center_x_cur = last_face_center_x - move_x_step;
		if(last_face_center_y > face_center_y){
			face_center_y_cur = last_face_center_y - move_y_step;
		}else{
			face_center_y_cur = last_face_center_y +  move_y_step;
		}
	}else{
		face_center_x_cur = last_face_center_x + move_x_step;
		if(last_face_center_y > face_center_y){
			face_center_y_cur = last_face_center_y - move_y_step;
		}else{
			face_center_y_cur = last_face_center_y + move_y_step;
		}
	}

	if((face_center_x_cur - zoomwidth_cur/2) <= 0){
		if(g_HV_Flip & 0x01) {
			zoomleft_cur = g_VideoWidth - zoomwidth_cur;
		}else {
			zoomleft_cur = 0;
		}
		last_fcrop_left = 0;
	}else if((face_center_x_cur + zoomwidth_cur/2) >= g_VideoWidth){
		if(g_HV_Flip & 0x01) {
			zoomleft_cur = 0;
		} else {
			zoomleft_cur = g_VideoWidth - zoomwidth_cur;
		}
		last_fcrop_left = g_VideoWidth - zoomwidth_cur;
	}else {
		if(g_HV_Flip & 0x01) {
			zoomleft_cur = (g_VideoWidth - face_center_x_cur) - zoomwidth_cur/2;
		} else {
			zoomleft_cur = face_center_x_cur - zoomwidth_cur/2;
		}
		last_fcrop_left = face_center_x_cur - zoomwidth_cur/2;
	}

	if((face_center_y_cur - zoomheight_cur/2) <= 0){
		if(g_HV_Flip & 0x02) {
			zoomtop_cur = g_VideoHeight - zoomheight_cur;
		} else {
			zoomtop_cur = 0;
		}
		last_fcrop_top = 0;
	}else if((face_center_y_cur + zoomheight_cur/2) >= g_VideoHeight){
		if(g_HV_Flip & 0x02){
			zoomtop_cur = 0;
		} else {
			zoomtop_cur = g_VideoHeight - zoomheight_cur;
		}
		last_fcrop_top = g_VideoHeight - zoomheight_cur;
	}else {
		if(g_HV_Flip & 0x02) {
			zoomtop_cur = (g_VideoHeight - face_center_y_cur) - zoomheight_cur/2;
		} else {
			zoomtop_cur = face_center_y_cur - zoomheight_cur/2;
		}
		last_fcrop_top = face_center_y_cur - zoomheight_cur/2;
	}

	last_scaler = value_cur;
	last_face_center_x = face_center_x_cur;
	last_face_center_y = face_center_y_cur;

	IMP_ISP_Tuning_GetFrontCrop(&fcrop_obj);
	if((fcrop_obj.fcrop_left == zoomleft_cur) && (fcrop_obj.fcrop_top == zoomtop_cur) \
		&& (fcrop_obj.fcrop_width == zoomwidth_cur) && (fcrop_obj.fcrop_height == zoomheight_cur)) {
		return 0;
	}

	fcrop_obj.fcrop_enable = 1;
	fcrop_obj.fcrop_left = zoomleft_cur;
	fcrop_obj.fcrop_top = zoomtop_cur;
	fcrop_obj.fcrop_width = zoomwidth_cur;
	fcrop_obj.fcrop_height = zoomheight_cur;
#if 0
	printf("INFO: value_cur = %f, face_center(%d, %d), face_center_cur(%d, %d), fcrop(%d, %d), width = %d, height = %d\n", value_cur,\
			face_center_x, face_center_y,\
			face_center_x_cur, face_center_y_cur,\
			fcrop_obj.fcrop_left, fcrop_obj.fcrop_top,\
			fcrop_obj.fcrop_width, fcrop_obj.fcrop_height);
#endif
	ret = IMP_ISP_Tuning_SetFrontCrop(&fcrop_obj);
	if (ret < 0) {
		printf("IMP Set Fcrop failed=%d\n",__LINE__);
		return -1;
	}
	usleep(20 * 1000);
	/*printf("INFO: last_scaler = %f\n", last_scaler);*/
	return 0;
}

static void face_zoom_reset(void)
{	
	IMPISPFrontCrop fcrop_obj;
	int ret;
	
	IMP_ISP_Tuning_GetFrontCrop(&fcrop_obj);

	fcrop_obj.fcrop_enable = 1;
	fcrop_obj.fcrop_left = 0;
	fcrop_obj.fcrop_top = 0;
	fcrop_obj.fcrop_width = g_VideoWidth;
	fcrop_obj.fcrop_height = g_VideoHeight;
	
	ret = IMP_ISP_Tuning_SetFrontCrop(&fcrop_obj);
	if (ret < 0) {
		printf("[%s]%s error\n",TAG,__func__);
		return;
	}
#ifdef DBUG
	printf("[%s]%s success !\n",TAG,__func__);
	printf("[%s]%s g_VideoWidth  g_VideoHeight !\n",TAG,__func__, g_VideoWidth,g_VideoHeight);
#endif
	return;
}

static int face_zoom_single_mode(void)
{
	node_t* fz_node = NULL;
	face_zoom_t face_zoom[MAX_NUM_FACE];
	unsigned char face_num = 0;
	int face_index = 0;
	static int face_center_x = 0;
	static int face_center_y = 0;

	fz_node = Get_Use_Node(face_zoom_nl);
	if(!fz_node){
		/*printf("ERROR[%s]: Get_USE_Node failed!\n", TAG);*/
		usleep(2*1000);
		goto zoom_continue;
	}

	memcpy(face_zoom, fz_node->data, fz_node->size);
	face_num = fz_node->size / sizeof(face_zoom_t);

	Put_Free_Node(face_zoom_nl, fz_node);

	for(int i = 0; i < face_num; i++){
		/* face_width before framesource ,such as in 3840x2160 frame , 6 is max scaler */
		if(face_width < (face_zoom[i].face_width * ALGO_CHN_SCALER / last_scaler)) {
			face_width_tmp = (face_zoom[i].face_width * ALGO_CHN_SCALER / last_scaler);
			/*printf("INFO: face_width = %d\n", face_width);*/
			face_index = i;
		}
	}

	/* moving */
	if(abs(face_width_tmp - face_width_last) > (3*ALGO_CHN_SCALER / last_scaler)) {
		face_width_last = face_width_tmp;
		anti_shake_cnt = 0;
		return 0;
	} else {
		anti_shake_cnt++;
		if(anti_shake_cnt <= 6) {
			return 0;
		}
		anti_shake_cnt = 0;
	}
	face_width = face_width_tmp;

	/* face center location before fcrop, such as in 1920x1080 frame */
	face_center_x = last_fcrop_left + face_zoom[face_index].face_center_x * ALGO_CHN_SCALER / last_scaler;
	face_center_y = last_fcrop_top + face_zoom[face_index].face_center_y * ALGO_CHN_SCALER / last_scaler;
#ifdef DBUG
	printf("face_width %d , face_center2(%d, %d),num:%d \n", face_width,face_center_x, face_center_y,face_index);
#endif
zoom_continue:
	if((face_width > 100) && (face_width < 500)) {
		face_zoom_set(VALUE_LEVEL_1_3, face_center_x, face_center_y);
	}else if(face_width > 520) {
		face_zoom_set(VALUE_LEVEL_1_0, face_center_x, face_center_y);
	}
#ifdef DBUG
	printf("[%s]facezoom_mode:%d \n",__func__,face_zoom_mode);
#endif
	return 0;
}

static int face_zoom_multi_mode(void)
{
	node_t* fz_node = NULL;
	face_zoom_t face_zoom[MAX_NUM_FACE];
	unsigned char face_num = 0;
	int face_ul_x = 0;
	unsigned char face_ul_index = 0;
	int face_br_x = 0;
	unsigned char face_br_index = 0;
	int i = 0;
	static int face_center_x = 0;
	static int face_center_y = 0;

	fz_node = Get_Use_Node(face_zoom_nl);
	if(!fz_node){
		/*printf("ERROR[%s]: Get_USE_Node failed!\n", TAG);*/
		usleep(2*1000);
		goto zoom_continue;
	}

	memcpy(face_zoom, fz_node->data, fz_node->size);
	face_num = fz_node->size / sizeof(face_zoom_t);

	Put_Free_Node(face_zoom_nl, fz_node);

	/* face_num != 0*/
	if(face_num == 1) {
		face_width_tmp = face_zoom[0].face_width * ALGO_CHN_SCALER / last_scaler;
		/* moving */
		if(abs(face_width_tmp - face_width_last) > (3*ALGO_CHN_SCALER / last_scaler)) {
			face_width_last = face_width_tmp;
			anti_shake_cnt = 0;
			return 0;
		} else {
			anti_shake_cnt++;
			if(anti_shake_cnt <= 6) {
				return 0;
			}
			anti_shake_cnt = 0;
		}
		face_width = face_width_tmp;
		face_center_x = last_fcrop_left + face_zoom[0].face_center_x * ALGO_CHN_SCALER / last_scaler;
		face_center_y = last_fcrop_top + face_zoom[0].face_center_y * ALGO_CHN_SCALER / last_scaler;
#ifdef DBUG
		printf("%d face_width %d ,face_center(%d, %d)\n", face_num, face_width, face_center_x, face_center_y);
#endif
	} else {
		face_ul_x = face_zoom[0].face_center_x;
		face_br_x = face_zoom[0].face_center_x;
		for(i = 1; i < face_num; i++) {
			if(face_zoom[i].face_center_x < face_ul_x) {
				face_ul_x = face_zoom[i].face_center_x;
				face_ul_index = i;
			}

			if(face_zoom[i].face_center_x > face_br_x) {
				face_br_x = face_zoom[i].face_center_x;
				face_br_index = i;
			}
		}

		face_width_tmp = face_br_x - face_ul_x;
		face_width_tmp = face_width_tmp * ALGO_CHN_SCALER / last_scaler;
		/* moving */
		if(abs(face_width_tmp - face_width_last) > (3*ALGO_CHN_SCALER / last_scaler)) {
			face_width_last = face_width_tmp;
			anti_shake_cnt = 0;
			return 0;
		} else {
			anti_shake_cnt++;
			if(anti_shake_cnt <= 6) {
				return 0;
			}
			anti_shake_cnt = 0;
		}

		face_center_x = face_zoom[face_ul_index].face_center_x + (face_zoom[face_br_index].face_center_x - face_zoom[face_ul_index].face_center_x) / 2;
		face_center_y = face_zoom[face_ul_index].face_center_y + (face_zoom[face_br_index].face_center_y - face_zoom[face_ul_index].face_center_y) / 2;
		/* fcrop location */
		face_width = face_width_tmp;
		face_center_x = last_fcrop_left + face_center_x * ALGO_CHN_SCALER / last_scaler;
		face_center_y = last_fcrop_top + face_center_y * ALGO_CHN_SCALER / last_scaler;
#ifdef DBUG
		printf("%d face_width %d , face_center(%d, %d)\n",face_num, face_width, face_center_x, face_center_y);
#endif
	}

zoom_continue:
	
	if((face_width > 200) && (face_width < 680)) {
		face_zoom_set(VALUE_LEVEL_1_3_1, face_center_x, face_center_y);
	}else if(face_width > 700) {
		face_zoom_set(VALUE_LEVEL_1_0, face_center_x, face_center_y);
	}
#ifdef DBUG
	printf("[%s]facezoom_mode:%d\n",__func__,face_zoom_mode);
#endif
	return 0;
}

void *face_zoom_process(void *none)
/*int module_facezoom_process(void)*/
{
	int ret = -1;

	facezoom_switch_class = 0;
	prctl(PR_SET_NAME, "face_zoom");
	printf("[%s]%s success!\n",TAG,__func__);
	while(!facezoom_switch_class)
	{
		if((face_zoom_mode != SINGLE_MODE) && !facezoom_mode_empty)
		{
			face_zoom_reset();
			last_scaler = 1.0;
			face_width = 0;
			face_width_tmp = 0;
			face_width_last = 0;
			anti_shake_cnt = 0;
			facezoom_mode_empty = 1;
		}
		switch(face_zoom_mode){
			case SINGLE_MODE:
				ret = face_zoom_single_mode();
				if(ret < 0) {
					printf("ERROR[%s], face_zoom_single_mode failed!\n", TAG);
				}
				break;
			case MULTI_MODE:
				ret = face_zoom_multi_mode();
				if(ret < 0) {
					printf("ERROR[%s], face_zoom_multi_mode failed!\n", TAG);
				}
				break;
			default:
				/*printf("INFO[%s]: face center mode doesn't support!\n", TAG);*/
				usleep(200 * 1000);
				break;
		}
	}
	printf("[%s]%s exit ok!\n",TAG,__func__);
	return NULL;
	/*return 0;*/
}

void *face_detect_process(void* none)
{
	int ret = 0;
	facedet_param_output_t* result = NULL;
	node_t* fz_node = NULL;
	face_zoom_t face_zoom[MAX_NUM_FACE];

	printf("facedet ivs start\n");
	ret = sample_ivs_facedet_start(0, 0, &interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_facedet_start(0, 0) failed\n");
		return NULL;
	}
	printf("facedet ivs start ok\n");

	prctl(PR_SET_NAME, "face_detect");
	while(!facezoom_switch_class)
	{
		ret = IMP_IVS_PollingResult(0, TIME_OUT);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult(%d, %d) failed\n", 0, IMP_IVS_DEFAULT_TIMEOUTMS);
			break;
		}
		ret = IMP_IVS_GetResult(0, (void **)&result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_GetResult(%d) failed\n", 0);
			break;
		}
		facedet_param_output_t* facedet_output = (facedet_param_output_t*)result;

		if(facedet_output->count > 0){
			fz_node = Get_Free_Node(face_zoom_nl);
			if(!fz_node){
				printf("ERROR[%s]: Get_free_node failed!\n", TAG);
				goto release_fs;
			}

			for(int j = 0; j < facedet_output->count; j++) {
				face_zoom[j].face_width		= facedet_output->face[j].show_box.br.x - facedet_output->face[j].show_box.ul.x;
				face_zoom[j].face_height	= facedet_output->face[j].show_box.br.y - facedet_output->face[j].show_box.ul.y;
				face_zoom[j].face_center_x	= facedet_output->face[j].show_box.ul.x + face_zoom[j].face_width / 2;
				face_zoom[j].face_center_y 	= facedet_output->face[j].show_box.ul.y + face_zoom[j].face_height / 2;
#if 0
				printf("face[%d]: ul(%d, %d), br(%d, %d), face_center1(%d, %d)\n", j, facedet_output->face[j].show_box.ul.x, facedet_output->face[j].show_box.ul.y,\
						facedet_output->face[j].show_box.br.x, facedet_output->face[j].show_box.br.y,\
						face_zoom[j].face_center_x, face_zoom[j].face_center_y);
#endif
			}

			fz_node->size = sizeof(face_zoom_t) * facedet_output->count;
			memcpy(fz_node->data, face_zoom, fz_node->size);

			Put_Use_Node(face_zoom_nl, fz_node);
		}
release_fs:

		ret = IMP_IVS_ReleaseResult(0, (void *)result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 0);
			break;
		}

	}

	ret = sample_ivs_facedet_stop(0, interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_facedet_stop(0) failed\n");
		return NULL;
	}

	printf("[%s]%s exit ok!\n",TAG,__func__);
	return NULL;
}

//开关线程
void *face_zoom_switch(void *none)
{
	int ret = -1;

	prctl(PR_SET_NAME, "face_zoom_switch");
	facedet_stop = 0;
	facezoom_switch = 0;
#ifdef DBUG
	printf("[%s]%s success!\n",TAG,__func__);
#endif
	while(!facedet_stop)
	{
		switch(facezoom_switch){
			case 0:
				ret = pthread_create(&facezoom_pid, NULL, face_zoom_process, NULL);
				if (ret != 0) {
				printf("pthread_create failed \n");
				return NULL;
				}

				ret = pthread_create(&facedet_pid, NULL, face_detect_process, NULL);
				if (ret != 0) {
				printf("pthread_create failed \n");
				return NULL;
				}
				
				facezoom_switch = -1;
				break;
			case 1:
				facezoom_switch_class = 1;
				last_scaler = 1.0;
				face_width = 0;
				facezoom_mode_empty = 0;
				face_zoom_reset();
				usleep(20 * 1000);
				//printf("&&&&&&&&&&&&&facezoom_switch_class:%d\n&&&&&&&",facezoom_switch_class);
				pthread_join(facedet_pid, NULL);
				pthread_join(facezoom_pid, NULL);
				facezoom_switch = -1;
				break;
			default:
				/*printf("INFO[%s]: face center mode doesn't support!\n", TAG);*/
				usleep(200 * 1000);
				break;
		}
		//printf("&&&&&&&&&&&&&facezoom_switch:%d\n&&&&&&&",facezoom_switch);
	}
#ifdef DBUG
	printf("[%s]%s exit ok!\n",TAG,__func__);
#endif
	return NULL;
	/*return 0;*/
}

void module_facezoom_enable_impinited(uint8_t on)
{
	face_zoom_impinited = on;
}

int module_facezoom_init(imp_isp_attr_t isp_param, facezoom_func_param_t fz_param)
{
	int ret;

	if(!fz_param.facezoom_en) {
		return 0;
	}

	printf("INFO[%s]: facezoom isp attr setting... \n", TAG);
	g_VideoWidth = isp_param.sensor_info.sensor_width;
	g_VideoHeight = isp_param.sensor_info.sensor_height;
	g_HV_Flip = isp_param.hvflip;

	face_zoom_mode = fz_param.facezoom_mode;

	while(!face_zoom_impinited) {
		usleep(20 * 1000);
	}
	ret = sample_framesource_init(ALGORITHM_VIDEO_CH);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

	ret = IMP_IVS_CreateGroup(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(0) failed\n");
		return -1;
	}

	IMPCell framesource_cell = {DEV_ID_FS, ALGORITHM_VIDEO_CH, 0};
	IMPCell ivs_cell = {DEV_ID_IVS, 0, 0};

	ret = IMP_System_Bind(&framesource_cell, &ivs_cell);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and ivs0 failed\n", ALGORITHM_VIDEO_CH);
		return -1;
	}

	ret = sample_framesource_streamon(ALGORITHM_VIDEO_CH);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		return -1;
	}

	face_zoom_nl = Node_List_Init(sizeof(face_zoom_t)*MAX_NUM_FACE, 2, 1, 0);
	if(!face_zoom_nl){
		printf("node list init failed!\n");
		return -1;
	}

	ret = pthread_create(&facezoom_switch_pid, NULL, face_zoom_switch, NULL);
	if (ret != 0) {
		printf("pthread_create failed \n");
		return -1;
	}
	
	return 0;
}


int module_facezoom_deinit(facezoom_func_param_t fz_param)
{
	int ret;

	if(!fz_param.facezoom_en) {
		return 0;
	}

	last_scaler = 1.0;
	facedet_stop = 1;
	face_width = 0;
	face_width_tmp = 0;
	face_width_last = 0;
	anti_shake_cnt = 0;
	facezoom_switch = 1;
	facezoom_switch_class = 1;
	facezoom_mode_empty = 0;
	pthread_join(facedet_pid, NULL);
	pthread_join(facezoom_pid, NULL);
	
	pthread_join(facezoom_switch_pid, NULL);

	IMPCell framesource_cell = {DEV_ID_FS, ALGORITHM_VIDEO_CH, 0};
	IMPCell ivs_cell = {DEV_ID_IVS, 0, 0};

	ret = IMP_System_UnBind(&framesource_cell, &ivs_cell);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and ivs0 failed\n", ALGORITHM_VIDEO_CH);
		return -1;
	}

	ret = IMP_IVS_DestroyGroup(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup(0) failed\n");
		return -1;
	}

	ret = sample_framesource_streamoff(ALGORITHM_VIDEO_CH);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOff failed\n");
		return -1;
	}

	ret = sample_framesource_exit(ALGORITHM_VIDEO_CH);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -1;
	}

	Node_List_Deinit(face_zoom_nl);
	return 0;
}


