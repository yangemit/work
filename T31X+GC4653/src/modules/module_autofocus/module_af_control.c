/*
 * module_af_control.c
 *
 * Copyright (C) 2022 Ingenic Semiconductor Co.,Ltd
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <pthread.h>

#include <imp-common.h>
#include <module_af_control.h>

#define MODULE_TAG "AF_CONTROL"
#define MOTOR_DOUBLE_CHECK 1

typedef struct _motor_param_info {
	int fd;                         /* 文件句柄 */
	int cur_pos;                    /* 当前对焦过程中电机位置 */
	int center_pos;                 /* 马达中心位置，即点胶位置 */
	int clarity_cur;                /* 当前isp 获取清晰度值 */
	int clarity_ori;                /* 上一次获取清晰度值*/
	pthread_mutex_t af_mutex;       /* af_mutex */
	struct timeval checktime;       /* af checktime */
} motor_param_info_t;

static motor_param_info_t motor_param_ctx;
static af_param_t motor_param;

static int caculate_rate(IMPISPAFHist af_hist)
{
	int clarity_now = 0, clarity_diff = 0, fall_rate = 0;

	clarity_now = af_hist.af_stat.af_metrics_alt + 1;
	clarity_diff = abs(motor_param_ctx.clarity_cur - clarity_now);
	fall_rate = (100 * clarity_diff) / (motor_param_ctx.clarity_cur + 1);

	return fall_rate;
}

static void update_af_clarity(IMPISPAFHist af_hist)
{
	motor_param_ctx.clarity_cur = af_hist.af_stat.af_metrics_alt + 1;

	if (motor_param_ctx.clarity_cur < motor_param_ctx.clarity_ori) {
		motor_param_ctx.clarity_cur = motor_param_ctx.clarity_ori;
	}
	gettimeofday(&motor_param_ctx.checktime, NULL);
}

static int autoFocus(int motor_step, int trigger_rate, int isDoubleCheck)
{
	int ret = -1, fd = -1;

	int dac_left = 0, dac_right = 0;
	int dac_hi  = 0, dac_cur = 0, dac_new = 0;

	int dac_clarity_left = 0, dac_clarity_right = 0;
	int dac_clarity_hi  = 0, dac_clarity_cur = 0, dac_clarity_new = 0;

	int right_rate = 0, left_rate = 0;
	int diff_rate = 0, diff_hi_rate = 0;

	/* 0: top, 1: right, -1: left */
	int direction = 0, i = 0;
	int pos_cnt = 0, hi_cnt = 0;

	int pos_buf[50] = {0};
	int pos_cla[50] = {0};
	int hi_buf[50] = {0};
	int hi_cla[50] = {0};

	int dac_max = motor_param.motor_step_max;
	int dac_min = motor_param.motor_step_min;
	IMPISPAFHist af_hist;
	dac_cur = motor_param_ctx.cur_pos;
	fd = motor_param_ctx.fd;

	/* Ucamera_DEBUG("INFO: ************** cur_pos = %d , g_center_pos = %d, ************\n", dac_cur, motor_param_ctx.center_pos); */

	/* step2: choose the direction to climb*/
	dac_right = dac_cur + motor_step;
	dac_left = dac_cur - motor_step;

	if (dac_left < dac_min) {
		dac_left = dac_min + 10;
		dac_cur = dac_min + 20;
		dac_right = dac_min + 30;
	} else if (dac_right > dac_max) {
		dac_right = dac_max -10;
		dac_cur = dac_max - 20;
		dac_left = dac_max - 30;
	}

	if (!isDoubleCheck) {
		ioctl(fd , MOTOR_MOVE , dac_right);
		usleep(motor_param.motor_sleep_time);
		IMP_ISP_Tuning_GetAfHist(&af_hist);
		dac_clarity_right = af_hist.af_stat.af_metrics_alt + 1;
	} else {
		ioctl(fd , MOTOR_MOVE , dac_left);
		usleep(motor_param.motor_sleep_time);
		IMP_ISP_Tuning_GetAfHist(&af_hist);
		dac_clarity_left = af_hist.af_stat.af_metrics_alt + 1;
	}

	/* current */
	ioctl(fd , MOTOR_MOVE , dac_cur);
	usleep(motor_param.motor_sleep_time);
	IMP_ISP_Tuning_GetAfHist(&af_hist);
	dac_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;

	if (!isDoubleCheck) {
		/* left */
		ioctl(fd , MOTOR_MOVE , dac_left);
		usleep(motor_param.motor_sleep_time);
		IMP_ISP_Tuning_GetAfHist(&af_hist);
		dac_clarity_left = af_hist.af_stat.af_metrics_alt + 1;
	} else {
		ioctl(fd , MOTOR_MOVE , dac_right);
		usleep(motor_param.motor_sleep_time);
		IMP_ISP_Tuning_GetAfHist(&af_hist);
		dac_clarity_right = af_hist.af_stat.af_metrics_alt + 1;
	}

	/* caculate fill rate */
	right_rate = (dac_clarity_right - dac_clarity_cur) * 100 / (dac_clarity_cur + 1);
	left_rate = (dac_clarity_left - dac_clarity_cur) * 100/ (dac_clarity_cur + 1);

	/* condition 1: flat region (1-1) */
	if ((abs(right_rate) <= trigger_rate && (abs(left_rate) <= trigger_rate))) {
		if ((motor_param_ctx.cur_pos - motor_param_ctx.center_pos) > 0)
			direction = -1; /* LEFT */
		else
			direction = 1;

		goto climb;
	}

	/* condition 2: flat top region right edge (1-3)(3-1) */
	if (abs(right_rate) <= trigger_rate && (abs(left_rate) > trigger_rate) && left_rate < 0) {
		direction = 1;
		goto climb;
	} else if (abs(left_rate) <= trigger_rate && (abs(right_rate) > trigger_rate) && right_rate < 0) {
		direction = -1;
		goto climb;
	}

	/* condition 3: top region (3-3) */
	if ((abs(right_rate) > trigger_rate) && right_rate < 0 && (abs(left_rate) > trigger_rate) && left_rate < 0) {
		/* Ucamera_DEBUG("************ climb : stay stable right_rate = %d, left_rate = %d,  ************\n", right_rate, left_rate); */
		ioctl(fd , MOTOR_MOVE ,dac_cur);
		usleep(motor_param.motor_sleep_time);
		IMP_ISP_Tuning_GetAfHist(&af_hist);
		dac_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;

		motor_param_ctx.cur_pos = dac_cur;
		return dac_clarity_cur;
	}

	/* condition 4: other conditions, judgement direction (2-2)  */
	if (right_rate > 0 && (abs(right_rate) > trigger_rate) && left_rate > 0 && (abs(left_rate) > trigger_rate)) { //station 3: botoom
		if (dac_clarity_right > dac_clarity_left)
			direction = 1;
		else
			direction = -1;
	} else if ((abs(right_rate) > trigger_rate && right_rate > 0 && abs(left_rate) <= trigger_rate) ||
			((abs(right_rate) > trigger_rate) && right_rate > 0 && (abs(left_rate) > trigger_rate) && left_rate < 0)) //station 4: slope (2-1)(2-3)
		direction = 1;
	else
		direction = -1;

climb:
	/* Ucamera_DEBUG("************* climb: direction = %d, right_rate = %d, left_rate = %d  *************\n", direction, right_rate, left_rate); */
	/* station 3: climb */
	if (!isDoubleCheck) {
		dac_cur = dac_left;
	} else {
		dac_cur = dac_right;
	}
	IMP_ISP_Tuning_GetAfHist(&af_hist);
	dac_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;
	dac_hi = dac_cur;
	dac_clarity_hi = dac_clarity_cur;

	do {
		pos_buf[pos_cnt] = dac_cur;
		pos_cla[pos_cnt] = dac_clarity_cur;
		pos_cnt++;

		if (pos_cnt >= 49) {
			/* Ucamera_DEBUG("************ climb: pos_buf is over 50 , return ************\n", dac_new); */
			ioctl(fd, MOTOR_MOVE, motor_param_ctx.center_pos);
			return -1;
		}

		dac_new = dac_cur + direction * motor_step;
		if (dac_new >= dac_max || dac_new <= dac_min) {
			/* Ucamera_DEBUG("************ climb: dac_new = %d, return ************\n", dac_new); */
			direction = -direction;
			continue;
		}
		ioctl(fd, MOTOR_MOVE, dac_new);
		usleep(motor_param.motor_sleep_time);
		ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
		if (ret < 0) {
			printf("ERROR(%s): IMP_ISP_Tuning_GetAfHist error!\n", MODULE_TAG);
			return -1;
		}

		dac_clarity_new = af_hist.af_stat.af_metrics_alt + 1;
		diff_rate = (dac_clarity_new - dac_clarity_cur) * 100 / (dac_clarity_cur + 1);
		diff_hi_rate = (dac_clarity_new - dac_clarity_hi) * 100 / (dac_clarity_hi + 1);

		/* stop climb and return hi position */
		if (diff_hi_rate < -10) {
			dac_cur = dac_hi;
			dac_clarity_cur = dac_clarity_hi;

			ioctl(fd, MOTOR_MOVE, dac_cur);

			motor_param_ctx.cur_pos = dac_cur;
			/* Ucamera_DEBUG("************** climb: hi_pos = %d, dac_new = %d, hi_clarity = %d, dac_clarity_new = %d ***************\n", dac_hi, dac_new, dac_clarity_hi, dac_clarity_new); */
			return dac_clarity_cur;
		}

		/* Ucamera_DEBUG("************** climb: dac_cur: %d, dac_new: %d, dac_clarity_cur: %d, dac_clarity_new: %d, diff_rate: %d, diff_hi_rate: %d ***************\n", */
				/* dac_cur, dac_new, dac_clarity_cur, dac_clarity_new, diff_rate, diff_hi_rate); */

		dac_cur = dac_new;
		dac_clarity_cur  = dac_clarity_new;

		/* update hi position */
		if (dac_clarity_cur > dac_clarity_hi) {
			dac_hi = dac_cur;
			dac_clarity_hi = dac_clarity_cur;
		}
	} while (!(diff_rate < 0 && abs(diff_rate) > (trigger_rate + 1)));

#if 1
	for (i = 0; i < pos_cnt; i++) {
		diff_rate = (dac_clarity_hi - pos_cla[i]) * 100 / (dac_clarity_hi + 1);
		if (diff_rate < 3) {
			hi_buf[hi_cnt] = pos_buf[i];
			hi_cla[hi_cnt] = pos_cla[i];
			hi_cnt++;
		}
	}

	for (i = 0; i < hi_cnt; i++) {
		if (dac_cur < hi_buf[i]) {
			dac_cur = hi_buf[i];
			dac_clarity_cur = hi_cla[i];
		}
	}
#else
	dac_cur = dac_hi;
	dac_clarity_cur = dac_clarity_hi;
#endif

	/* station 4: end process */

	ioctl(fd, MOTOR_MOVE, dac_cur);

	/* Ucamera_DEBUG("************* climb: pos = %d, clarity = %d *************\n", dac_cur, dac_clarity_cur); */
	motor_param_ctx.cur_pos = dac_cur;

	return dac_clarity_cur;

}

static int autoFocusClimb(int motor_step)
{
	return autoFocus(motor_step, 3, !MOTOR_DOUBLE_CHECK);
}

static int autoFocusDobuleCheck(int motor_step)
{
	return autoFocus(motor_step, 0, MOTOR_DOUBLE_CHECK);

}

static int autoFocusCheck(int fd, int motor_step)
{
	int ret = -1;

	int dac_left = 0, dac_right = 0;
	int dac_hi  = 0, dac_cur = 0, dac_new = 0;

	int dac_clarity_left = 0, dac_clarity_right = 0;
	int dac_clarity_hi  = 0, dac_clarity_cur = 0, dac_clarity_new = 0;

	int right_rate = 0, left_rate = 0;
	int diff_rate = 0, diff_hi_rate = 0, trigger_rate = 0;

	/* 0: top, 1: right, -1: left */
	int direction = 0, i = 0;

	int pos_buf[50] = {0};
	int pos_cla[50] = {0};
	int hi_buf[50] = {0};
	int hi_cla[50] = {0};

	int pos_cnt = 0, hi_cnt = 0;

	int dac_max = motor_param.motor_step_max;
	int dac_min = motor_param.motor_step_min;
	IMPISPAFHist af_hist;

	dac_cur = motor_param_ctx.cur_pos;

	/* Ucamera_DEBUG("INFO: ############## check : cur_pos = %d , g_center_pos = %d, ############\n", dac_cur, motor_param_ctx.center_pos); */

	/* step2: choose the direction to climb*/
	dac_right = dac_cur + motor_step;
	dac_left = dac_cur - motor_step;

	if (dac_left < dac_min) {
		dac_left = dac_min + 10;
		dac_cur = dac_min + 20;
		dac_right = dac_min + 30;
	} else if (dac_right > dac_max) {
		dac_right = dac_max -10;
		dac_cur = dac_max - 20;
		dac_left = dac_max - 30;
	}

	/* left */
	ioctl(fd , MOTOR_MOVE , dac_left);
	usleep(motor_param.motor_sleep_time);
	IMP_ISP_Tuning_GetAfHist(&af_hist);
	dac_clarity_left = af_hist.af_stat.af_metrics_alt + 1;

	/* current */
	ioctl(fd , MOTOR_MOVE , dac_cur);
	usleep(motor_param.motor_sleep_time);
	IMP_ISP_Tuning_GetAfHist(&af_hist);
	dac_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;

	/* right */
	ioctl(fd , MOTOR_MOVE , dac_right);
	usleep(motor_param.motor_sleep_time);
	IMP_ISP_Tuning_GetAfHist(&af_hist);
	dac_clarity_right = af_hist.af_stat.af_metrics_alt + 1;

	/* caculate fill rate */
	right_rate = (dac_clarity_right - dac_clarity_cur) * 100 / (dac_clarity_cur + 1);
	left_rate = (dac_clarity_left - dac_clarity_cur) * 100 / (dac_clarity_cur + 1);

	/* condition 1: flat region (1-1) */
	if ((abs(right_rate) <= trigger_rate && (abs(left_rate) <= trigger_rate))) {
		if ((motor_param_ctx.cur_pos - motor_param_ctx.center_pos) > 0)
			direction = -1;
		else
			direction = 1;

		goto climb;
	}

	/* condition 2: flat top region right edge (1-3) */
	if (abs(right_rate) <= trigger_rate && (abs(left_rate) > trigger_rate) && left_rate < 0) {
		direction = 1;
		goto climb;
	}

	/* condition 3: top region (3-3)(3-1) */
	if (((abs(right_rate) > trigger_rate) && right_rate < 0 && (abs(left_rate) > trigger_rate) && left_rate < 0) ||
			(abs(left_rate) <= trigger_rate && (abs(right_rate) > trigger_rate) && right_rate < 0)) {
		/* Ucamera_DEBUG("############## check : stay stable right_rate = %d, left_rate = %d, ###########\n", right_rate, left_rate); */
		ioctl(fd , MOTOR_MOVE ,dac_cur);
		usleep(motor_param.motor_sleep_time);
		IMP_ISP_Tuning_GetAfHist(&af_hist);
		dac_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;

		motor_param_ctx.cur_pos = dac_cur;
		return dac_clarity_cur;
	}

	/* condition 4: other conditions, judgement direction (2-2)  */
	if (right_rate > 0 && (abs(right_rate) > trigger_rate) && left_rate > 0 && (abs(left_rate) > trigger_rate)) { //station 3: botoom
		if (dac_clarity_right > dac_clarity_left)
			direction = 1;
		else
			direction = -1;
	} else if ((abs(right_rate) > trigger_rate && right_rate > 0 && abs(left_rate) <= trigger_rate) ||
			((abs(right_rate) > trigger_rate) && right_rate > 0 && (abs(left_rate) > trigger_rate) && left_rate < 0)) //station 4: slope (2-1)(2-3)
		direction = 1;
	else
		direction = -1;

climb:
	/* Ucamera_DEBUG("############# check: direction = %d, right_rate = %d, left_rate = %d  #############\n", direction, right_rate, left_rate); */

	/* start climb */
	dac_cur = dac_right;
	IMP_ISP_Tuning_GetAfHist(&af_hist);
	dac_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;
	dac_hi = dac_cur;
	dac_clarity_hi = dac_clarity_cur;

	do {
		pos_buf[pos_cnt] = dac_cur;
		pos_cla[pos_cnt] = dac_clarity_cur;
		pos_cnt++;

		if (pos_cnt >= 49) {
			/* Ucamera_DEBUG("############ climb: pos_buf is over 50 , return ############\n", dac_new); */
			ioctl(fd, MOTOR_MOVE, motor_param_ctx.center_pos);
			return -1;
		}

		dac_new = dac_cur + direction * motor_step;
		if (dac_new >= dac_max || dac_new <= dac_min) {
			/* Ucamera_DEBUG("############ check: dac_new = %d, edge!!! return ############\n", dac_new); */
			direction = -direction;
			continue;
		}
		ioctl(fd, MOTOR_MOVE, dac_new);
		usleep(motor_param.motor_sleep_time);
		ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
		if (ret < 0) {
			IMP_LOG_ERR(MODULE_TAG, "IMP_ISP_Tuning_GetAfHist error!\n");
			return -1;
		}

		dac_clarity_new = af_hist.af_stat.af_metrics_alt + 1;
		diff_rate = (dac_clarity_new - dac_clarity_cur) * 100 / (dac_clarity_cur + 1);
		diff_hi_rate = (dac_clarity_new - dac_clarity_hi) * 100 / (dac_clarity_hi + 1);

		/* stop climb and return hi position */
		if (diff_hi_rate < -15) {
			dac_cur = dac_hi;
			dac_clarity_cur = dac_clarity_hi;

			ioctl(fd, MOTOR_MOVE, dac_cur);

			motor_param_ctx.cur_pos = dac_cur;
			/* Ucamera_DEBUG("############## check: hi_pos = %d, dac_new = %d, hi_clarity = %d, dac_clarity_new = %d ###############\n", dac_hi, dac_new, dac_clarity_hi, dac_clarity_new); */
			return dac_clarity_cur;
		}

		/* Ucamera_DEBUG("############## check: dac_cur: %d, dac_new: %d, dac_clarity_cur: %d, dac_clarity_new: %d, diff_rate: %d, diff_hi_rate: %d ###############\n", */
				/* dac_cur, dac_new, dac_clarity_cur, dac_clarity_new, diff_rate, diff_hi_rate); */

		dac_cur = dac_new;
		dac_clarity_cur  = dac_clarity_new;

		/* update hi position */
		if (dac_clarity_cur > dac_clarity_hi) {
			dac_hi = dac_cur;
			dac_clarity_hi = dac_clarity_cur;
		}
	} while (!(diff_rate < 0 && abs(diff_rate) > (trigger_rate + 1)));

#if 1
	for (i = 0; i < pos_cnt; i++) {
		diff_rate = (dac_clarity_hi - pos_cla[i]) * 100 / (dac_clarity_hi + 1);
		if (diff_rate < 3) {
			hi_buf[hi_cnt] = pos_buf[i];
			hi_cla[hi_cnt] = pos_cla[i];
			hi_cnt++;
		}
	}

	for (i = 0; i < hi_cnt; i++) {
		if (dac_cur < hi_buf[i]) {
			dac_cur = hi_buf[i];
			dac_clarity_cur = hi_cla[i];
		}
	}
#else
	dac_cur = dac_hi;
	dac_clarity_cur = dac_clarity_hi;
#endif

	/* station 4: end process */

	ioctl(fd, MOTOR_MOVE, dac_cur);

	/* Ucamera_DEBUG("############# check: pos = %d, clarity = %d #############\n", dac_cur, dac_clarity_cur); */
	motor_param_ctx.cur_pos = dac_cur;

	return dac_clarity_cur;
}

/*static int motor_restart = 0;*/
static void *af_control_process(void *args)
{
	int stream_on = 0, trigger_count = 0;
	int ret = -1, dac_pos = 300;

	int fd = motor_param_ctx.fd;

	IMPISPAFHist af_hist;
	IMPISPWeight af_weight;

#if UCAMERA_DEBUG
	struct timeval ts_s, ts_d;
#endif

	prctl(PR_SET_NAME, "af_control_process");

	while (!stream_on) {
		stream_on = sample_get_ucamera_streamonoff();
		usleep(1000 * 1000);
	}

	/* step1 : initialize clarity algorithm */
	int focus_trigger_value_now = motor_param.focus_trigger_value;

	while(IMP_ISP_Tuning_GetAfHist(&af_hist) < 0){
		usleep(1000 * 1000);
	}

	printf("INFO(%s): af_enable is %u \n" , MODULE_TAG, af_hist.af_enable);

#if UCAMERA_DEBUG
	Ucamera_LOG("af_metrics_shift is %u \n" , af_hist.af_metrics_shift);
	Ucamera_LOG("af_delta is %u \n" , af_hist.af_delta);
	Ucamera_LOG("af_theta is %u \n" , af_hist.af_theta);
	Ucamera_LOG("af_hilight_th is %u \n" , af_hist.af_hilight_th);
	Ucamera_LOG("af_alpha_alt is %u \n" , af_hist.af_alpha_alt);
	Ucamera_LOG("af_hstart is %u \n" , af_hist.af_hstart);
	Ucamera_LOG("af_vstart is %u \n" , af_hist.af_vstart);
	Ucamera_LOG("af_stat_nodeh is %u \n" , af_hist.af_stat_nodeh);
	Ucamera_LOG("af_stat_nodev is %u \n\n\n" , af_hist.af_stat_nodev);
#endif

	int motor_pixel = motor_param.motor_pixel;

	if (motor_pixel == MOTOR_PIXEL2M) {
		unsigned char set_weight[15][15] = {
			{0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
			{0,1,1,1,1,1,1,0,1,1,1,1,1,1,1},
			{0,1,1,1,1,1,1,0,1,1,1,1,1,1,1},
			{0,1,1,8,8,1,1,0,1,1,1,1,1,1,1},
			{0,1,1,8,8,1,1,0,1,1,1,1,1,1,1},
			{0,1,1,1,1,1,1,0,1,1,1,1,1,1,1},
			{0,1,1,1,1,1,1,0,1,1,1,1,1,1,1},
			{0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		};

		for(int i = 0; i < 15; i++) {
			for(int j = 0; j < 15; j++) {
				af_weight.weight[i][j] = set_weight[i][j];
			}
		}

	} else if (motor_pixel == MOTOR_PIXEL4M) {
		unsigned char set_weight[15][15] = {

			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,1,1,1,1,1,1,1,1,1,0,0,0},
			{0,0,0,1,4,4,4,4,4,4,1,1,0,0,0},
			{0,0,0,1,4,8,8,8,8,8,4,1,0,0,0},
			{0,0,0,1,4,8,8,8,8,8,4,1,0,0,0},
			{0,0,0,1,4,8,8,8,8,8,4,1,0,0,0},
			{0,0,0,1,4,8,8,8,8,8,4,1,0,0,0},
			{0,0,0,1,4,8,8,8,8,8,4,1,0,0,0},
			{0,0,0,1,4,4,4,4,4,4,4,1,0,0,0},
			{0,0,0,1,1,1,1,1,1,1,1,1,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

		};

		for(int i = 0; i < 15; i++) {
			for(int j = 0; j < 15; j++) {
				af_weight.weight[i][j] = set_weight[i][j];
			}
		}

	} else {
		printf("ERROR(%s): set motor_pixel err! \n", MODULE_TAG);
		return NULL;
	}

	/* step2: set af weight  */
	ret = IMP_ISP_Tuning_SetAfWeight(&af_weight);
	if (ret < 0) {
		IMP_LOG_ERR(MODULE_TAG, "IMP_ISP_Tuning_SetAfWeight err! \n");
		return NULL;
	}

	ret = IMP_ISP_Tuning_GetAfWeight(&af_weight);
	if (ret < 0) {
		IMP_LOG_ERR(MODULE_TAG, "IMP_ISP_Tuning_GetAfHist err! \n");
		return NULL;
	}

#if UCAMERA_DEBUG
	/* Ucamera_DEBUG("af_weight is : \n"); */
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			printf("%u " , af_weight.weight[i][j]);
		}
		printf("\n");
	}
#endif

	/*Step2: first autoFocus */
	pthread_mutex_lock(&motor_param_ctx.af_mutex);

	motor_param_ctx.clarity_ori = autoFocusClimb(25);
	motor_param_ctx.center_pos = motor_param_ctx.cur_pos;

	/* after process */
	usleep(150 * 1000);
	IMP_ISP_Tuning_GetAfHist(&af_hist);
	update_af_clarity(af_hist);

	pthread_mutex_unlock(&motor_param_ctx.af_mutex);

	int flag_motor_standby = 0;
	int af_on = 0;
	struct timeval cur_ts;
	uint64_t ts1 = 0, ts2 = 0;
	int first_af = 0;
	int fall_rate = 0;

	while (1) {
		stream_on = sample_get_ucamera_streamonoff();
		if (stream_on == 0) {
			gettimeofday(&cur_ts, NULL);
			ts1 = (uint64_t)(cur_ts.tv_sec * 1000000) + cur_ts.tv_usec;
			first_af = 1;
		}

		while (stream_on == 0) {
			/*low power mode */
			gettimeofday(&cur_ts, NULL);
			ts2 = (uint64_t)(cur_ts.tv_sec * 1000000) + cur_ts.tv_usec;
			if ((ts2 - ts1) > LOWPOWER_TIMESTAMP ) {
				if(flag_motor_standby == 1) {
					flag_motor_standby = 0;
					ret = ioctl(fd, MOTOR_MOVE, 0);
					/* Ucamera_DEBUG("vcm move 0, low power mode \n "); */
					if (ret < 0) {
						printf("ERROR(%s): ioctl err! \n", MODULE_TAG);
					}
				}
			}
			/*motor_restart = 1;*/

			usleep(1000 * 1000);
			/* update stream_on */
			stream_on = sample_get_ucamera_streamonoff();
		}

		flag_motor_standby = 1;

		if (first_af == 1 && af_on == 1) {
			pthread_mutex_lock(&motor_param_ctx.af_mutex);

			motor_param_ctx.clarity_ori = autoFocusClimb(motor_param.motor_step);
			if (motor_param_ctx.clarity_ori < 0) {
				printf("ERROR(%s): first autoFocusClimb error\n", MODULE_TAG);
				pthread_mutex_unlock(&motor_param_ctx.af_mutex);
				continue;
			}

			motor_param_ctx.clarity_ori = autoFocusDobuleCheck(8);
			if (motor_param_ctx.clarity_ori < 0) {
				printf("ERROR(%s): autoFocusClimb check failed \n", MODULE_TAG);
				pthread_mutex_unlock(&motor_param_ctx.af_mutex);
				continue;
			}

			motor_param_ctx.center_pos = motor_param_ctx.cur_pos;

			/* after process */
			usleep(150 * 1000);
			IMP_ISP_Tuning_GetAfHist(&af_hist);
			update_af_clarity(af_hist);

			pthread_mutex_unlock(&motor_param_ctx.af_mutex);

			first_af = 0;
			continue;
		}

		af_on = sample_get_ucamera_af_onoff();
		if (af_on) {
			ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
			if (ret < 0) {
				IMP_LOG_ERR(MODULE_TAG, "IMP_ISP_Tuning_GetAFMetrices err! \n");
				usleep(34 * 1000);
				continue;
			}

#if 0
			clarity_now = af_hist.af_stat.af_metrics_alt + 1;
			clarity_diff = abs(motor_param_ctx.clarity_cur - clarity_now);
			fall_rate = (100 * clarity_diff) / (motor_param_ctx.clarity_cur + 1);
#endif
			fall_rate = caculate_rate(af_hist);
			if (fall_rate > focus_trigger_value_now) {
				pthread_mutex_lock(&motor_param_ctx.af_mutex);
				trigger_count = 0;
				for (int i = 0; i < 15; i++) {
					usleep(34 * 1000);
					ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
					if (ret < 0) {
						IMP_LOG_ERR(MODULE_TAG, "IMP_ISP_Tuning_GetAFMetrices err! (%d)\n", __LINE__);
						pthread_mutex_unlock(&motor_param_ctx.af_mutex);
						usleep(34 * 1000);
						continue;
					}

					fall_rate = caculate_rate(af_hist);
					if (fall_rate > focus_trigger_value_now) {
						trigger_count++;
					}
				}

				/*Ucamera_LOG("trigger_count is %d\n",trigger_count);*/
				if (trigger_count >= 3) {
#if UCAMERA_DEBUG
					gettimeofday(&ts_s , NULL);
#endif
					int clarity1, clarity2, tmp_diff = 0;
autoclimb:
					motor_param_ctx.clarity_ori = autoFocusClimb(motor_param.motor_step);
					if (motor_param_ctx.clarity_ori < 0) {
						printf("ERROR(%s): autoFocusClimb failed . (%d)\n",MODULE_TAG, __LINE__);
						pthread_mutex_unlock(&motor_param_ctx.af_mutex);
						continue;
					}

					clarity1 = motor_param_ctx.clarity_ori;
					motor_param_ctx.clarity_ori = autoFocusDobuleCheck(8);
					if (motor_param_ctx.clarity_ori < 0) {
						printf("ERROR(%s): autoFocusClimb check failed . (%d)\n", MODULE_TAG, __LINE__);
						pthread_mutex_unlock(&motor_param_ctx.af_mutex);
						continue;
					}

					clarity2 = motor_param_ctx.clarity_ori;
					if (clarity2 < clarity1) {
						tmp_diff = abs(clarity2 - clarity1) * 100 / (clarity1 + 1);
						if (tmp_diff > 25) {
							/* Ucamera_DEBUG("############## clarity1 = %d, clarity2 = %d, diff = %d, restart autoclimb  #############\n", */
									/* clarity1, clarity2, tmp_diff); */
							goto autoclimb;
						}
					}

#if UCAMERA_DEBUG
					gettimeofday(&ts_d, NULL);
					/* Ucamera_DEBUG("####AF time = %llums ####\n", ((((unsigned long long )ts_d.tv_sec * 1000000) + ts_d.tv_usec) - ((( unsigned long long)ts_s.tv_sec * 1000000) + ts_s.tv_usec)) / 1000); */
#endif

					/* after process */
					usleep(150 * 1000);
					ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
					if(ret < 0){
						IMP_LOG_ERR(MODULE_TAG, "IMP_ISP_Tuning_GetAFMetrices err! (%d)\n", __LINE__);
						usleep(34 * 1000);
						pthread_mutex_unlock(&motor_param_ctx.af_mutex);
						continue;
					}

					update_af_clarity(af_hist);
				} else {
					motor_param_ctx.clarity_ori = autoFocusCheck(fd, 8);

					/* after process */
					usleep(150 * 1000);
					ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
					if(ret < 0){
						IMP_LOG_ERR(MODULE_TAG, "IMP_ISP_Tuning_GetAFMetrices err! (%d)\n", __LINE__);
						usleep(34 * 1000);
						pthread_mutex_unlock(&motor_param_ctx.af_mutex);
						continue;
					}
					update_af_clarity(af_hist);

					/* Ucamera_DEBUG("trigger_count is 0 ~3 \n"); */
				}
				pthread_mutex_unlock(&motor_param_ctx.af_mutex);
			} else {
				usleep(34 * 1000);
			}
		} else {
			int focus_cur;
			focus_cur = sample_get_ucamera_af_cur();
			if (dac_pos != focus_cur) {
				dac_pos = focus_cur;
				ret = ioctl(fd, MOTOR_MOVE, dac_pos);
				if (ret < 0) {
					printf("ERROR(%s): ioctl err! (%d)\n", MODULE_TAG, __LINE__);
				}
			}

			usleep(300 * 1000);
		}

	}

	return NULL;

}

static void *af_check_process(void *arg)
{
	int af_on = 0;
	int fd = motor_param_ctx.fd;
	int stream_on = 0;
	unsigned long long ts_diff = 0;
	struct timeval cur_ts;
	IMPISPAFHist af_hist;
	int re_cnt = 0;
	int tmp_clarity = 0;
	int tmp_diff = 0;

	prctl(PR_SET_NAME, "af_check_process");
	while (1) {
		stream_on = sample_get_ucamera_streamonoff();
		while (!stream_on) {
			stream_on = sample_get_ucamera_streamonoff();
			usleep(1000 * 1000);
		}

		af_on = sample_get_ucamera_af_onoff();
		if (af_on && stream_on) {
			pthread_mutex_lock(&motor_param_ctx.af_mutex);

			gettimeofday(&cur_ts, NULL);
			ts_diff = ((unsigned long long)cur_ts.tv_sec * 1000000 + cur_ts.tv_usec) - ((unsigned long long)motor_param_ctx.checktime.tv_sec * 1000000 + motor_param_ctx.checktime.tv_usec);

			if (ts_diff > 1000000) {
				motor_param_ctx.clarity_ori = autoFocusCheck(fd, 8);
				if (motor_param_ctx.clarity_ori < 0) {
						printf("ERROR(%s): autoFocusClimb check failed .(%d)\n", MODULE_TAG, __LINE__);
						pthread_mutex_unlock(&motor_param_ctx.af_mutex);
						continue;
				}

				tmp_clarity = motor_param_ctx.clarity_ori;
				if (tmp_clarity < motor_param_ctx.clarity_cur) {
					tmp_diff = abs(tmp_clarity - motor_param_ctx.clarity_cur) * 100 / (motor_param_ctx.clarity_cur + 1);
					if (tmp_diff > 25) {
						re_cnt++;
						if (re_cnt > 3) {
							goto check;
						}
						gettimeofday(&motor_param_ctx.checktime, NULL);
						pthread_mutex_unlock(&motor_param_ctx.af_mutex);
						continue;
					}
				}
check:
				re_cnt = 0;
				usleep(80 * 1000);

				IMP_ISP_Tuning_GetAfHist(&af_hist);
				update_af_clarity(af_hist);
			}

			pthread_mutex_unlock(&motor_param_ctx.af_mutex);

			usleep(34 * 1000);
		} else {
			sleep(1);
		}
	}

	return NULL;
}

int module_autofocus_init(void *param)
{
	int ret = -1;
	motor_param = *((af_param_t *)param);
	if (motor_param.af_en) {
		pthread_mutex_init(&motor_param_ctx.af_mutex, NULL);

		/*Step1: open dev */
		int fd = open(VCM_DEV_NAME, 0);
		if (fd < 0) {
			printf("ERROR(%s):open %s err! \n", MODULE_TAG, VCM_DEV_NAME);
			return -1;
		}

		/*Step2: set modor default pos */
		motor_param_ctx.fd = fd;
		motor_param_ctx.cur_pos = 500;
		ioctl(fd, MOTOR_MOVE, motor_param_ctx.cur_pos);

		/*Step3: create af af_control_process thread */
		pthread_t tid_af;
		pthread_attr_t attr_af;
		pthread_attr_init(&attr_af);
		pthread_attr_setdetachstate(&attr_af, PTHREAD_CREATE_DETACHED);
		ret = pthread_create(&tid_af, &attr_af, af_control_process, NULL);
		if (ret != 0) {
			printf("ERROR(%s): pthread_create af_control_process err \n", MODULE_TAG);
			return -1;
		}

		/*Step4: create af af_check_process thread */
		pthread_t tid_af_check;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		ret = pthread_create(&tid_af_check, &attr, af_check_process, NULL);
		if (ret != 0) {
			printf("ERROR(%s): pthread_create af_check_process err \n", MODULE_TAG);
			return -1;
		}
	}


	return 0;

}

void module_autofocus_deinit()
{
	if (motor_param.af_en)
		close(motor_param_ctx.fd);

	printf("INFO(%s): module_autofocus_deinit ...ok\n", MODULE_TAG);
}
