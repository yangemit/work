#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>

#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_person_tracker.h>
#include <imp/imp_ivs.h>

#include <iaac.h>

#include "sample-common.h"

#define TAG "SAMPLE-MOVE"

#define USE_MOTOR_DRIVER
#ifdef USE_MOTOR_DRIVER
/* ioctl cmd */
#define MOTOR_STOP			0x1
#define MOTOR_RESET			0x2
#define MOTOR_MOVE			0x3
#define MOTOR_GET_STATUS		0x4
#define MOTOR_SPEED			0x5

#define MOTOR_DIRECTIONAL_UP		0x0
#define MOTOR_DIRECTIONAL_DOWN		0x1
#define MOTOR_DIRECTIONAL_LEFT		0x2
#define MOTOR_DIRECTIONAL_RIGHT		0x3

struct motor_move_st {
	int motor_directional;
	int motor_move_steps;
	int motor_move_speed;
};

enum motor_status {
	MOTOR_IS_STOP,
	MOTOR_IS_RUNNING,
};

struct motor_message {
	int x;
	int y;
	enum motor_status status;
	int speed;
};

struct motors_steps{
	int x;
	int y;
};

struct motor_status_st {
	int directional_attr;
	int total_steps;
	int current_steps;
	int min_speed;
	int cur_speed;
	int max_speed;
	int move_is_min;
	int move_is_max;
};

struct motor_reset_data {
	unsigned int x_max_steps;
	unsigned int y_max_steps;
	unsigned int x_cur_step;
	unsigned int y_cur_step;
};

struct motors_steps jb_motors_steps;
struct motor_message jb_motor_message;
struct motor_status_st motor_status;
struct motor_move_st motor_action;
struct motor_reset_data motor_reset_data;

static int trackerFd = -1;
bool iflast_stop = false;
static int setMotorPtz1(int dx, int dy, int tracking, struct motor_message msg, int capture_lost, int x_max_steps, int y_max_steps)
{
	if((iflast_stop)&&(MOTOR_IS_STOP != msg.status))
		return 0;
	jb_motors_steps.x = 0;
	jb_motors_steps.y = 0;

	static int is_moving = 0;
	/* float s_t1c = 0; */
	/* float s_tc = 0; */
	int Maxstep = 200;
	int ret = 0;
	bool reset = false;

	/* float kpy = 0.1, kdy = 0.0, kiy = 0.0; */
	float kpy = 0.1, kdy = 0.0;
	static int dy_t1 ;
	jb_motors_steps.y = (int)(kpy*(dy) + kdy*( dy - dy_t1));
	dy_t1 = dy;
	if (abs(dy) < 80){
		jb_motors_steps.y = 0;
	}else{
		;
	}

	if(tracking){
		if(100 == motor_action.motor_move_speed){
			if(abs(dx) >= 80){
				motor_action.motor_move_speed=900;
			}else if(abs(dx) >= 50){
				motor_action.motor_move_speed=700;
			}
		}else{
			if(abs(dx) >= 80){
				motor_action.motor_move_speed=900;
			}else if(abs(dx) >= 60){
				motor_action.motor_move_speed=700;
			}else if(abs(dx) >= 40){
				motor_action.motor_move_speed=500;
			}else if(abs(dx) >= 30){
				motor_action.motor_move_speed=300;
			}else{
				motor_action.motor_move_speed=100;
			}
		}

		switch(motor_action.motor_move_speed){
		case 100:
			jb_motors_steps.x = 0;
			break;
		default:
			jb_motors_steps.x = dx*2;
			break;
		}

		reset = true;
	}else{

		if(capture_lost){

			is_moving = 1;
			motor_action.motor_move_speed = 900;

			if(abs(dx)>80){
				jb_motors_steps.x = dx*2;
			}else{
				jb_motors_steps.x = 0;
			}

			if(abs(dy)>80){
				jb_motors_steps.y = 0.8*dy;
			}else{
				jb_motors_steps.y = 0;
			}

			reset = true;
		}else{
			if(!is_moving){
				motor_action.motor_move_speed = 100;
				if(!iflast_stop){
					ioctl(trackerFd, MOTOR_STOP);
					iflast_stop = true;
				}

				ret = 1;
			}else{
				if(MOTOR_IS_STOP == msg.status){
					is_moving = 0;
				}
			}
		}
	}

	if(reset){
		//ioctl(trackerFd, MOTOR_SPEED, (unsigned int) (&motor_action.motor_move_speed));

		bool y_limit = ((jb_motors_steps.y <= 0)&&(msg.y <= 0)) || ((jb_motors_steps.y >= 0)&&(msg.y >= y_max_steps));
		bool x_limit = ((jb_motors_steps.x <= 0)&&(msg.x <= 0)) || ((jb_motors_steps.x >= 0)&&(msg.x >= x_max_steps));
		bool x_cond = x_limit && (y_limit || (jb_motors_steps.y == 0));
		bool y_cond = y_limit && (x_limit || (jb_motors_steps.x == 0));

		if(x_cond)
			jb_motors_steps.x = 0;
		if(y_cond)
			jb_motors_steps.y = 0;

		if( ((0 == jb_motors_steps.x)&&(0 == jb_motors_steps.y)) || (x_cond && y_cond)){
			if(!iflast_stop){
				ioctl(trackerFd, MOTOR_STOP);
				iflast_stop = true;
			}

			ret = 1;
		}else{
			if(jb_motors_steps.x > Maxstep)
				jb_motors_steps.x = Maxstep;
			else if(jb_motors_steps.x < -Maxstep)
				jb_motors_steps.x = -Maxstep;
			ioctl(trackerFd, MOTOR_SPEED, (unsigned int) (&motor_action.motor_move_speed));
			ioctl(trackerFd, MOTOR_MOVE, (unsigned long) &jb_motors_steps);
			iflast_stop = false;
		}
	}

	IMP_LOG_INFO(TAG, "Person (%d, %d) %d: (%d, %d)\n",dx, dy, motor_action.motor_move_speed, jb_motors_steps.x, jb_motors_steps.y);
	return ret;
}
#endif

int sample_ivs_person_tracker_init(int grp_num)
{
	int ret = 0;

	ret = IMP_IVS_CreateGroup(grp_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}

	IAACInfo ainfo = {
		.license_path = "/system/etc/license/license.txt",
		.cid = 1,
		.fid = 1,
		.sn = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
	};

	ret = IAAC_Init(&ainfo);
	if (ret) {
		IMP_LOG_ERR(TAG, "IAAC_Init error!\n");
		return -1;
	}

	return 0;
}

int sample_ivs_person_tracker_exit(int grp_num)
{
	int ret = 0;

	IAAC_DeInit();

	ret = IMP_IVS_DestroyGroup(grp_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup(%d) failed\n", grp_num);
		return -1;
	}
	return 0;
}

int sample_ivs_person_tracker_start(int grp_num, int chn_num, IMPIVSInterface **interface)
{
	int ret = 0;
	person_tracker_param_input_t param;

	memset(&param, 0, sizeof(person_tracker_param_input_t));
	param.frameInfo.width = sensor_sub_width;
	param.frameInfo.height = sensor_sub_height;
    param.stable_time_out = 4;
    param.move_sense = 2;
    param.obj_min_width = 40;
    param.obj_min_height = 40;
    param.move_thresh_denoise = 20;
    param.add_smooth = 0;
    param.conf_thresh = 0.8;
    param.conf_lower = 0.2;
    param.frozen_in_boundary = 5;
    param.boundary_ratio = 0.25;
    param.mode = 0;
    param.enable_move = false;
    param.area_ratio = 0.2;
	param.exp_x = 1.0;


	*interface = Person_TrackerInterfaceInit(&param);
	if (*interface == NULL) {
		IMP_LOG_ERR(TAG, "Person_TrackerInterfaceInit failed\n");
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

int sample_ivs_person_tracker_stop(int chn_num, IMPIVSInterface *interface)
{
	int ret = 0;

	ret = IMP_IVS_StopRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StopRecvPic(%d) failed\n", chn_num);
		return -1;
	}
	sleep(1);

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

	Person_TrackerInterfaceExit(interface);

	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0, i = 0, cnt = 0;
	IMPIVSInterface *inteface = NULL;
	person_tracker_param_output_t *result = NULL;
    person_tracker_param_input_t param;

#ifdef USE_MOTOR_DRIVER
      trackerFd= open("/dev/motor", 0);
      if (trackerFd < 0){
	printf("Error /dev/motor!\n");
	exit(-1);
      }
      motor_action.motor_move_speed = 900;
      ioctl(trackerFd, MOTOR_SPEED, (unsigned int) (&motor_action.motor_move_speed));

      struct motor_reset_data rdata;
      memset(&rdata, 0, sizeof(rdata));
      ret = ioctl(trackerFd, MOTOR_RESET, (unsigned int) (&rdata));
      if (ret != 0){
	close(trackerFd);
	exit(-1);
      }
	  int x_max_steps = rdata.x_max_steps;
	  int y_max_steps = rdata.y_max_steps;
#endif

	/* Step.1 System init */
	ret = sample_system_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
		return -1;
	}

	/* Step.2 FrameSource init */
	IMPFSChnAttr fs_chn_attr;
	memset(&fs_chn_attr, 0, sizeof(IMPFSChnAttr));
	fs_chn_attr.pixFmt = PIX_FMT_NV12;
	fs_chn_attr.outFrmRateNum = SENSOR_FRAME_RATE;
	fs_chn_attr.outFrmRateDen = 1;
	fs_chn_attr.nrVBs = 3;
	fs_chn_attr.type = FS_PHY_CHANNEL;

	fs_chn_attr.crop.enable = 0;
	fs_chn_attr.crop.top = 0;
	fs_chn_attr.crop.left = 0;
	fs_chn_attr.crop.width = sensor_main_width;
	fs_chn_attr.crop.height = sensor_main_height;

	fs_chn_attr.scaler.enable = 1;	/* ivs use the second framesource channel, need scale*/
	fs_chn_attr.scaler.outwidth = sensor_sub_width;
	fs_chn_attr.scaler.outheight = sensor_sub_height;

	fs_chn_attr.picWidth = sensor_sub_width;
	fs_chn_attr.picHeight = sensor_sub_height;

	ret = sample_framesource_init(FS_SUB_CHN, &fs_chn_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

	/* Step.3 Encoder init */
	ret = sample_ivs_person_tracker_init(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_move_init(0) failed\n");
		return -1;
	}

	/* Step.4 Bind */
	IMPCell framesource_cell = {DEV_ID_FS, FS_SUB_CHN, 0};
	IMPCell ivs_cell = {DEV_ID_IVS, 0, 0};

	ret = IMP_System_Bind(&framesource_cell, &ivs_cell);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and ivs0 failed\n", FS_SUB_CHN);
		return -1;
	}

	/* Step.5 Stream On */
	IMP_FrameSource_SetFrameDepth(FS_SUB_CHN, 0);
	ret = sample_framesource_streamon(FS_SUB_CHN);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		return -1;
	}

	ret = sample_ivs_person_tracker_start(0, 0, &inteface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_move_start(0, 0) failed\n");
		return -1;
	}

	/* Step.6 Get result */
	for (i = 0; i < NR_FRAMES_TO_IVS; i++) {
		ret = IMP_IVS_PollingResult(0, IMP_IVS_DEFAULT_TIMEOUTMS);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult(%d, %d) failed\n", 0, IMP_IVS_DEFAULT_TIMEOUTMS);
			return -1;
		}
		ret = IMP_IVS_GetResult(0, (void **)&result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_GetResult(%d) failed\n", 0);
			return -1;
		}
		IMP_LOG_INFO(TAG, "frame[%d], result->count=%d, dx=%d, dy=%d, step=%d\n",
				i, result->count, result->dx, result->dy, result->step);
		for (cnt = 0; cnt < result->count; cnt++) {
            printf("person_status %d\n", result->person_status);
			IMP_LOG_INFO(TAG, "rect[%d]=((%d,%d),(%d,%d))\n",
					cnt, result->rect[cnt].ul.x, result->rect[cnt].ul.y, result->rect[cnt].ul.x, result->rect[cnt].ul.y);
		}
        /* //mode = 3 */
        /* for (cnt = 0; cnt < result->move_count; cnt++) { */
		/* 	IMP_LOG_INFO(TAG, "move_rect[%d]=((%d,%d),(%d,%d))\n", */
		/* 			cnt, result->move_rect[cnt].ul.x, result->move_rect[cnt].ul.y, result->move_rect[cnt].ul.x, result->move_rect[cnt].ul.y); */
		/* } */
        /* for (cnt = 0; cnt < result->person_count; cnt++) { */
		/* 	IMP_LOG_INFO(TAG, "person_rect[%d]=((%d,%d),(%d,%d))\n", */
		/* 			cnt, result->person_rect[cnt].ul.x, result->person_rect[cnt].ul.y, result->person_rect[cnt].ul.x, result->person_rect[cnt].ul.y); */
		/* } */

#ifdef USE_MOTOR_DRIVER
		ioctl(trackerFd, MOTOR_GET_STATUS, (unsigned long) &jb_motor_message);

		ret = IMP_IVS_GetParam(0, &param);
		if (ret < 0){
		  IMP_LOG_ERR(TAG, "IMP_IVS_GetParam(%d) failed\n", 0);
		  return -1;
		}

		setMotorPtz1(result->dx, result->dy, result->tracking, jb_motor_message, result->capture_lost, x_max_steps, y_max_steps);

		//ioctl(trackerFd, MOTOR_GET_STATUS, (unsigned long) &jb_motor_message);
		param.is_motor_stop = ((MOTOR_IS_STOP == jb_motor_message.status)? 1 : 0);
		param.is_feedback_motor_status = 1;
#else
		ret = IMP_IVS_GetParam(0, &param);
		if (ret < 0){
		  IMP_LOG_ERR(TAG, "IMP_IVS_GetParam(%d) failed\n", 0);
		  return -1;
		}

		param.is_motor_stop = 1;
		param.is_feedback_motor_status = 1;
#endif

		ret = IMP_IVS_SetParam(0, &param);
		if (ret < 0){
		  IMP_LOG_ERR(TAG, "IMP_IVS_SetParam(%d) failed\n", 0);
		  return -1;
		}

		ret = IMP_IVS_ReleaseResult(0, (void *)result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 0);
			return -1;
		}
	}

	ret = sample_ivs_person_tracker_stop(0, inteface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_move_stop(0) failed\n");
		return -1;
	}

	/* Step.7 Stream Off */
	ret = sample_framesource_streamoff(FS_SUB_CHN);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		return -1;
	}

	/* Step.b UnBind */
	ret = IMP_System_UnBind(&framesource_cell, &ivs_cell);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and ivs0 failed\n", FS_SUB_CHN);
		return -1;
	}

	/* Step.c ivs exit */
	ret = sample_ivs_person_tracker_exit(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ivs mode exit failed\n");
		return -1;
	}

	/* Step.d FrameSource exit */
	ret = sample_framesource_exit(FS_SUB_CHN);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource(%d) exit failed\n", FS_SUB_CHN);
		return -1;
	}

	/* Step.e System exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
		return -1;
	}

#ifdef USE_MOTOR_DRIVER
	if (trackerFd >= 0) {
	  close(trackerFd);
	  trackerFd = -1;
	}
#endif
	return 0;
}
