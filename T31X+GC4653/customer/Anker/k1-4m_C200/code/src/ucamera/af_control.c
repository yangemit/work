/*
 * af_control.c
 *
 * Copyright (C) 2021 Ingenic Semiconductor Co.,Ltd
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <pthread.h>

#include <imp-common.h>
#include <af_control.h>
#include <cdc_log_info.h>
#include <usbcamera.h>

#define TAG "AF-CONTROL"

#define AUTO_FOCUS_CHECK_THRESH   (1 * 1000000)
#define STABLE_CHECK_THRESH       (15 * 1000000)
#define AUTO_FOCUS_CHECK_STAND    (5 * 1000000)

static int g_dac_pos = 300;
static int focus_trigger_value_now = 30;
static int cur_pos = 0;

static vcm_motor_param_t motor_param;
static pthread_mutex_t af_mutex;
static int g_fd = -1;
static struct timeval g_checktime;
static struct timeval g_focustime;
static int g_clarity_cur;
static int g_clarity_ori;
static int g_center_pos = 500;
static int ts_thresh = 0;

static int autoFocusClimb(int fd, int motor_step)
{
	int ret = -1;

	int dac_left = 0;
	int dac_right = 0;
	int dac_hi  = 0;
	int dac_cur = 0;
	int dac_new = 0;

	int dac_clarity_left = 0;
	int dac_clarity_right = 0;
	int dac_clarity_hi  = 0;
	int dac_clarity_cur = 0;
	int dac_clarity_new = 0;

	int right_rate = 0;
	int left_rate = 0;
	int diff_rate = 0;
	int diff_hi_rate = 0;
	int trigger_rate = 3;

	/* 0: top, 1: right, -1: left */
	int direction = 0;

	int i = 0;
	int pos_buf[50] = {0};
	int pos_cla[50] = {0};
	int hi_buf[50] = {0};
	int hi_cla[50] = {0};
	int pos_cnt = 0;
	int hi_cnt = 0;

	int dac_max = motor_param.motor_step_max;
	int dac_min = motor_param.motor_step_min;
	IMPISPAFHist af_hist;

	dac_cur = cur_pos;

	Ucamera_DEBUG("INFO: ************** climb: cur_pos = %d , g_center_pos = %d, ************\n", cur_pos, g_center_pos);

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

	/* right */
	ioctl(fd , MOTOR_MOVE , dac_right);
	usleep(motor_param.motor_sleep_time);
	IMP_ISP_Tuning_GetAfHist(&af_hist);
	dac_clarity_right = af_hist.af_stat.af_metrics_alt + 1;

	/* current */
	ioctl(fd , MOTOR_MOVE , dac_cur);
	usleep(motor_param.motor_sleep_time);
	IMP_ISP_Tuning_GetAfHist(&af_hist);
	dac_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;

	/* left */
	ioctl(fd , MOTOR_MOVE , dac_left);
	usleep(motor_param.motor_sleep_time);
	IMP_ISP_Tuning_GetAfHist(&af_hist);
	dac_clarity_left = af_hist.af_stat.af_metrics_alt + 1;

	/* caculate fill rate */
	right_rate = (dac_clarity_right - dac_clarity_cur) * 100 / (dac_clarity_cur + 1);
	left_rate = (dac_clarity_left - dac_clarity_cur) * 100/ (dac_clarity_cur + 1);

	/* condition 1: flat region (1-1) */
	if ((abs(right_rate) <= trigger_rate && (abs(left_rate) <= trigger_rate))) {
		if ((cur_pos - g_center_pos) > 0)
			direction = -1;
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
		Ucamera_DEBUG("************ climb : stay stable right_rate = %d, left_rate = %d,  ************\n", right_rate, left_rate);
		ioctl(fd , MOTOR_MOVE ,dac_cur);
		usleep(motor_param.motor_sleep_time);
		IMP_ISP_Tuning_GetAfHist(&af_hist);
		dac_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;

		cur_pos = dac_cur;
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
	Ucamera_DEBUG("************* climb: direction = %d, right_rate = %d, left_rate = %d  *************\n", direction, right_rate, left_rate);
	/* station 3: climb */
	dac_cur = dac_left;
	IMP_ISP_Tuning_GetAfHist(&af_hist);
	dac_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;
	dac_hi = dac_cur;
	dac_clarity_hi = dac_clarity_cur;

	do {
		pos_buf[pos_cnt] = dac_cur;
		pos_cla[pos_cnt] = dac_clarity_cur;
		pos_cnt++;

		if (pos_cnt >= 49) {
			Ucamera_DEBUG("************ climb: pos_buf is over 50 , return ************\n", dac_new);
			ioctl(fd, MOTOR_MOVE, g_center_pos);
			return -1;
		}

		dac_new = dac_cur + direction * motor_step;
		if (dac_new >= dac_max || dac_new <= dac_min) {
			Ucamera_DEBUG("************ climb: dac_new = %d, return ************\n", dac_new);
			direction = -direction;
			continue;
		}
		ioctl(fd, MOTOR_MOVE, dac_new);
		usleep(motor_param.motor_sleep_time);
		ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
		if (ret < 0) {
			Ucamera_ERR("IMP_ISP_Tuning_GetAfHist error!\n");
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

			cur_pos = dac_cur;
			Ucamera_DEBUG("************** climb: hi_pos = %d, dac_new = %d, hi_clarity = %d, dac_clarity_new = %d ***************\n", dac_hi, dac_new, dac_clarity_hi, dac_clarity_new);
			return dac_clarity_cur;
		}

		Ucamera_DEBUG("************** climb: dac_cur: %d, dac_new: %d, dac_clarity_cur: %d, dac_clarity_new: %d, diff_rate: %d, diff_hi_rate: %d ***************\n", 
				dac_cur, dac_new, dac_clarity_cur, dac_clarity_new, diff_rate, diff_hi_rate);

		dac_cur = dac_new;
		dac_clarity_cur  = dac_clarity_new;

		/* update hi position */
		if (dac_clarity_cur > dac_clarity_hi) {
			dac_hi = dac_cur;
			dac_clarity_hi = dac_clarity_cur;
		}
	} while (!(diff_rate < 0 && abs(diff_rate) > (trigger_rate + 1)));

	int ev_value = sample_get_ucamera_ev_cur();
	if (ev_value < 20000) {
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
	} else {
#if 1
		dac_cur = dac_hi;
		dac_clarity_cur = dac_clarity_hi;
#else
		for (i = 0; i < pos_cnt; i++) {
			diff_rate = (dac_clarity_hi - pos_cla[i]) * 100 / (dac_clarity_hi + 1);
			if (diff_rate < 1) {
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
#endif
	}

	/* station 4: end process */

	ioctl(fd, MOTOR_MOVE, dac_cur);

	Ucamera_DEBUG("************* climb: pos = %d, clarity = %d *************\n", dac_cur, dac_clarity_cur);
	cur_pos = dac_cur;

	return dac_clarity_cur;
}

static int autoFocusDobuleCheck(int fd, int motor_step)
{
	int ret = -1;

	int dac_left = 0;
	int dac_right = 0;
	int dac_hi  = 0;
	int dac_cur = 0;
	int dac_new = 0;

	int dac_clarity_left = 0;
	int dac_clarity_right = 0;
	int dac_clarity_hi  = 0;
	int dac_clarity_cur = 0;
	int dac_clarity_new = 0;

	int right_rate = 0;
	int left_rate = 0;
	int diff_rate = 0;
	int diff_hi_rate = 0;
	int trigger_rate = 0;

	/* 0: top, 1: right, -1: left */
	int direction = 0;

	int i = 0;
	int pos_buf[50] = {0};
	int pos_cla[50] = {0};
	int hi_buf[50] = {0};
	int hi_cla[50] = {0};
	int pos_cnt = 0;
	int hi_cnt = 0;

	int dac_max = motor_param.motor_step_max;
	int dac_min = motor_param.motor_step_min;
	IMPISPAFHist af_hist;

	dac_cur = cur_pos;

	Ucamera_DEBUG("INFO: @@@@@@@@@@@@@@ check : cur_pos = %d , g_center_pos = %d, @@@@@@@@@@@@\n", cur_pos, g_center_pos);

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
		if ((cur_pos - g_center_pos) > 0)
			direction = -1;
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
		Ucamera_DEBUG("@@@@@@@@@@@@@@ check : stay stable right_rate = %d, left_rate = %d,  @@@@@@@@@@@\n", right_rate, left_rate);
		ioctl(fd , MOTOR_MOVE ,dac_cur);
		usleep(motor_param.motor_sleep_time);
		IMP_ISP_Tuning_GetAfHist(&af_hist);
		dac_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;

		cur_pos = dac_cur;
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
	Ucamera_DEBUG("@@@@@@@@@@@@@ check: direction = %d, right_rate = %d, left_rate = %d  @@@@@@@@@@@@@\n", direction, right_rate, left_rate);

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
			Ucamera_DEBUG("@@@@@@@@@@@@ climb: pos_buf is over 50 , return @@@@@@@@@@@@\n", dac_new);
			ioctl(fd, MOTOR_MOVE, g_center_pos);
			return -1;
		}

		dac_new = dac_cur + direction * motor_step;
		if (dac_new >= dac_max || dac_new <= dac_min) {
			Ucamera_DEBUG("@@@@@@@@@@@@ check: dac_new = %d, edge!!! return @@@@@@@@@@@@\n", dac_new);
			direction = -direction;
			continue;
		}
		ioctl(fd, MOTOR_MOVE, dac_new);
		usleep(motor_param.motor_sleep_time);
		ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
		if (ret < 0) {
			Ucamera_ERR("IMP_ISP_Tuning_GetAfHist error!\n");
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

			cur_pos = dac_cur;
			Ucamera_DEBUG("@@@@@@@@@@@@@@ check: hi_pos = %d, dac_new = %d, hi_clarity = %d, dac_clarity_new = %d @@@@@@@@@@@@@@@\n", dac_hi, dac_new, dac_clarity_hi, dac_clarity_new);
			return dac_clarity_cur;
		}

		Ucamera_DEBUG("@@@@@@@@@@@@@@ check: dac_cur: %d, dac_new: %d, dac_clarity_cur: %d, dac_clarity_new: %d, diff_rate: %d, diff_hi_rate: %d @@@@@@@@@@@@@@@\n", 
				dac_cur, dac_new, dac_clarity_cur, dac_clarity_new, diff_rate, diff_hi_rate);

		dac_cur = dac_new;
		dac_clarity_cur  = dac_clarity_new;

		/* update hi position */
		if (dac_clarity_cur > dac_clarity_hi) {
			dac_hi = dac_cur;
			dac_clarity_hi = dac_clarity_cur;
		}
	} while (!(diff_rate < 0 && abs(diff_rate) > trigger_rate + 1));

	int ev_value = sample_get_ucamera_ev_cur();
	if (ev_value < 20000) {
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
	} else {
#if 1
		dac_cur = dac_hi;
		dac_clarity_cur = dac_clarity_hi;
#else
		for (i = 0; i < pos_cnt; i++) {
			diff_rate = (dac_clarity_hi - pos_cla[i]) * 100 / (dac_clarity_hi + 1);
			if (diff_rate < 1) {
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
#endif
	}

	/* station 4: end process */

	ioctl(fd, MOTOR_MOVE, dac_cur);

	Ucamera_DEBUG("@@@@@@@@@@@@@ check: pos = %d, clarity = %d @@@@@@@@@@@@@@@@@\n", dac_cur, dac_clarity_cur);
	cur_pos = dac_cur;

	return dac_clarity_cur;
}

/*static int checkback_flag = 0;*/
static int autoFocusCheck(int fd, int motor_step)
{
	int ret = -1;

	int dac_left = 0;
	int dac_right = 0;
	int dac_hi  = 0;
	int dac_cur = 0;
	int dac_new = 0;

	int dac_clarity_left = 0;
	int dac_clarity_right = 0;
	int dac_clarity_hi  = 0;
	int dac_clarity_cur = 0;
	int dac_clarity_new = 0;

	int right_rate = 0;
	int left_rate = 0;
	int diff_rate = 0;
	int diff_hi_rate = 0;
	int trigger_rate = 1;

	/* 0: top, 1: right, -1: left */
	int direction = 0;

	int i = 0;
	int pos_buf[50] = {0};
	int pos_cla[50] = {0};
	int hi_buf[50] = {0};
	int hi_cla[50] = {0};
	int pos_cnt = 0;
	int hi_cnt = 0;

	int dac_max = motor_param.motor_step_max;
	int dac_min = motor_param.motor_step_min;
	IMPISPAFHist af_hist;

	dac_cur = cur_pos;

	Ucamera_DEBUG("INFO: ############## check : cur_pos = %d , g_center_pos = %d, ############\n", cur_pos, g_center_pos);

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
		if ((cur_pos - g_center_pos) > 0)
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
		Ucamera_DEBUG("############## check : stay stable right_rate = %d, left_rate = %d, ###########\n", right_rate, left_rate);
		ioctl(fd , MOTOR_MOVE ,dac_cur);
		usleep(motor_param.motor_sleep_time);
		IMP_ISP_Tuning_GetAfHist(&af_hist);
		dac_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;

		cur_pos = dac_cur;
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
	Ucamera_DEBUG("############# check: direction = %d, right_rate = %d, left_rate = %d  #############\n", direction, right_rate, left_rate);

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
			Ucamera_DEBUG("############ climb: pos_buf is over 50 , return ############\n", dac_new);
			ioctl(fd, MOTOR_MOVE, g_center_pos);
			return -1;
		}

		dac_new = dac_cur + direction * motor_step;
		if (dac_new >= dac_max || dac_new <= dac_min) {
			Ucamera_DEBUG("############ check: dac_new = %d, edge!!! return ############\n", dac_new);
			direction = -direction;
			continue;
		}
		ioctl(fd, MOTOR_MOVE, dac_new);
		usleep(motor_param.motor_sleep_time);
		ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
		if (ret < 0) {
			Ucamera_ERR("IMP_ISP_Tuning_GetAfHist error!\n");
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

			cur_pos = dac_cur;
			Ucamera_DEBUG("############## check: hi_pos = %d, dac_new = %d, hi_clarity = %d, dac_clarity_new = %d ###############\n", dac_hi, dac_new, dac_clarity_hi, dac_clarity_new);
			return dac_clarity_cur;
		}

		Ucamera_DEBUG("############## check: dac_cur: %d, dac_new: %d, dac_clarity_cur: %d, dac_clarity_new: %d, diff_rate: %d, diff_hi_rate: %d ###############\n", 
				dac_cur, dac_new, dac_clarity_cur, dac_clarity_new, diff_rate, diff_hi_rate);

		dac_cur = dac_new;
		dac_clarity_cur  = dac_clarity_new;

		/* update hi position */
		if (dac_clarity_cur > dac_clarity_hi) {
			dac_hi = dac_cur;
			dac_clarity_hi = dac_clarity_cur;
		}
	} while (!(diff_rate < 0 && abs(diff_rate) > (trigger_rate + 1)));

	int ev_value = sample_get_ucamera_ev_cur();
	if (ev_value < 20000) {
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
	} else {
#if 1
		/*dark process */
		dac_cur = dac_hi;
		dac_clarity_cur = dac_clarity_hi;
#else
		for (i = 0; i < pos_cnt; i++) {
			diff_rate = (dac_clarity_hi - pos_cla[i]) * 100 / (dac_clarity_hi + 1);
			if (diff_rate < 1) {
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
#endif
	}

	/* station 4: end process */

	ioctl(fd, MOTOR_MOVE, dac_cur);

	Ucamera_DEBUG("############# check: pos = %d, clarity = %d #############\n", dac_cur, dac_clarity_cur);
	cur_pos = dac_cur;

	return dac_clarity_cur;
}

#if 0
static int autoFocusScan(int fd, int motor_step)
{
	int i;
	int ret = -1;
	int max_value = 0;
	IMPISPAFHist af_hist;

	int max_i = 0;
	int motor_pos_max = cur_pos + 10;
	int motor_sleep_time = motor_param.motor_sleep_time;
	int motor_step_min = cur_pos - 10;

	int dac_clarity_buf[32] = {0};
	int dac_value = 0;

	/* step1: motor reset motor_pos_max */
#if 0
	int reset_stride = (motor_pos_max - dac_pos)/motor_step;
	float n = 0;
	if (reset_stride < 2)
		n = 1;
	else if (reset_stride == 2)
		n = 1.5;
	else
		n = 2;
	/*motor_reset(dac_pos, fd, motor_pos_max);*/
#endif
	ret = ioctl(fd , MOTOR_MOVE , motor_pos_max);
	usleep(motor_sleep_time);

	ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
	if (ret < 0) {
		Ucamera_LOG("IMP_ISP_Tuning_GetAFMetrices err! \n");
		return -1;
	}
	dac_clarity_buf[motor_pos_max / motor_step] = af_hist.af_stat.af_metrics_alt;
	Ucamera_DEBUG("clarity_buf[%d] is %d \n" , (motor_pos_max / motor_step) ,dac_clarity_buf[motor_pos_max / motor_step]);

	/* step2: motor move from 350 to 250 (step : 25), get max clarity */
	for (i = (motor_pos_max / motor_step - 1); i >= (motor_step_min / motor_step); i--) {
		dac_value = motor_step * i;
		/*motor_reset(dac_pos, fd, dac_value);*/
		ret = ioctl(fd , MOTOR_MOVE , dac_value);
		usleep(motor_sleep_time);

		ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
		if (ret < 0) {
			Ucamera_LOG("IMP_ISP_Tuning_GetAFMetrices err! \n");
			return -1;
		}
		dac_clarity_buf[i] = af_hist.af_stat.af_metrics_alt;
		Ucamera_DEBUG("clarity_buf[%d] is %d \n", i, dac_clarity_buf[i]);
	}

	max_value = dac_clarity_buf[motor_step_min / motor_step];
	max_i = motor_step_min / motor_step;
	for (i = (max_i + 1); i <= (motor_pos_max / motor_step); i++) {
		if (dac_clarity_buf[i] > max_value) {
			max_value = dac_clarity_buf[i];
			max_i = i;
		}
	}


	if (max_value == 0)
		max_value = 1;


	/* step4 : move motor to suitable position */
	dac_value = max_i * motor_step;
	ret = ioctl(fd , MOTOR_MOVE , dac_value);
	Ucamera_DEBUG("motor_postion is =%d , last cur_pos = %d\n", dac_value, cur_pos);
	cur_pos = dac_value;

	return max_value;
}
#endif

static int motor_restart = 0;
void *sample_af_control_process(void *args)
{
	int stream_on = 0;
	int trigger_count = 0;
	int ret = -1;
	int fd = g_fd;

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
	focus_trigger_value_now = motor_param.focus_trigger_value;

	while(IMP_ISP_Tuning_GetAfHist(&af_hist) < 0){
		usleep(1000 * 1000);
	}

	Ucamera_LOG("af_enable is %u \n" , af_hist.af_enable);

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
#if 0
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,1,1,1,1,1,1,1,0,0,0,0},
			{0,0,0,0,1,3,3,3,3,3,1,0,0,0,0},
			{0,0,0,0,1,3,8,8,8,3,1,0,0,0,0},
			{0,0,0,0,1,3,8,8,8,3,1,0,0,0,0},
			{0,0,0,0,1,3,8,8,8,3,1,0,0,0,0},
			{0,0,0,0,1,3,3,3,3,3,1,0,0,0,0},
			{0,0,0,0,1,1,1,1,1,1,1,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
#endif

#if 0
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{1,1,3,3,3,3,3,3,3,3,3,3,3,1,1},
			{1,1,3,1,4,4,4,4,4,4,4,1,3,1,1},
			{1,1,3,1,4,8,8,8,8,8,4,1,3,1,1},
			{1,1,3,1,4,8,8,8,8,8,4,1,3,1,1},
			{1,1,3,1,4,8,8,8,8,8,4,1,3,1,1},
			{1,1,3,1,4,8,8,8,8,8,4,1,3,1,1},
			{1,1,3,1,4,8,8,8,8,8,4,1,3,1,1},
			{1,1,3,1,4,4,4,4,4,4,4,1,3,1,1},
			{1,1,3,3,3,3,3,3,3,3,3,3,3,1,1},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
#endif

#if 1
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
#endif

#if 0
			{4,1,1,1,1,1,1,4,1,1,1,1,1,1,4},
			{1,4,0,0,1,1,0,4,0,1,1,0,0,4,1},
			{1,0,4,0,1,1,0,4,0,1,1,0,4,0,1},
			{1,0,0,4,1,1,0,4,0,1,1,4,0,0,1},
			{1,1,1,1,4,1,1,4,1,1,4,1,1,1,1},
			{1,1,1,1,1,8,8,8,8,8,1,1,1,1,1},
			{1,0,0,0,1,8,8,8,8,8,1,0,0,0,1},
			{4,4,4,4,4,8,8,8,8,8,4,4,4,4,4},
			{1,0,0,0,1,8,8,8,8,8,1,0,0,0,1},
			{1,1,1,1,1,8,8,8,8,8,1,1,1,1,1},
			{1,1,1,1,4,1,1,4,1,1,4,1,1,1,1},
			{1,0,0,4,1,1,0,4,0,1,1,4,0,0,1},
			{1,0,4,0,1,1,0,4,0,1,1,0,4,0,1},
			{1,4,0,0,1,1,0,4,0,1,1,0,0,4,1},
			{4,1,1,1,1,1,1,4,1,1,1,1,1,1,4},
#endif

#if 0
			{4,4,4,4,4,0,0,0,0,0,4,4,4,4,4},
			{4,4,4,4,4,0,0,0,0,0,4,4,4,4,4},
			{4,4,4,4,4,0,0,0,0,0,4,4,4,4,4},
			{4,4,4,4,4,0,0,0,0,0,4,4,4,4,4},
			{4,4,4,4,4,0,0,0,0,0,4,4,4,4,4},
			{0,0,0,0,0,8,8,8,8,8,0,0,0,0,0},
			{0,0,0,0,0,8,8,8,8,8,0,0,0,0,0},
			{0,0,0,0,0,8,8,8,8,8,0,0,0,0,0},
			{0,0,0,0,0,8,8,8,8,8,0,0,0,0,0},
			{0,0,0,0,0,8,8,8,8,8,0,0,0,0,0},
			{4,4,4,4,4,0,0,0,0,0,4,4,4,4,4},
			{4,4,4,4,4,0,0,0,0,0,4,4,4,4,4},
			{4,4,4,4,4,0,0,0,0,0,4,4,4,4,4},
			{4,4,4,4,4,0,0,0,0,0,4,4,4,4,4},
			{4,4,4,4,4,0,0,0,0,0,4,4,4,4,4},
#endif
		};

		for(int i = 0; i < 15; i++) {
			for(int j = 0; j < 15; j++) {
				af_weight.weight[i][j] = set_weight[i][j];
			}
		}

	} else {
		Ucamera_LOG("set motor_pixel err! \n");
		return NULL;
	}

	/* step2: set af weight  */
	ret = IMP_ISP_Tuning_SetAfWeight(&af_weight);
	if (ret < 0) {
		Ucamera_LOG("IMP_ISP_Tuning_SetAfWeight err! \n");
		CDC_LOG(IMP_SET_AF_WEIGHT_ERROR, "IMP_ISP_Tuning_SetAfWeight error");
		return NULL;
	}

	ret = IMP_ISP_Tuning_GetAfWeight(&af_weight);
	if (ret < 0) {
		Ucamera_LOG("IMP_ISP_Tuning_GetAfHist err! \n");
		CDC_LOG(IMP_GET_AF_WEIGHT_ERROR, "IMP_ISP_Tuning_GetAfWeight error");
		return NULL;
	}

#if UCAMERA_DEBUG
	Ucamera_DEBUG("af_weight is : \n");
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			printf("%u " , af_weight.weight[i][j]);
		}
		printf("\n");
	}
#endif

	/*Step2: first autoFocus */
	pthread_mutex_lock(&af_mutex);

	g_clarity_ori = autoFocusClimb(fd, 25);
	g_center_pos = cur_pos;

	/* after process */
	usleep(150 * 1000);
	IMP_ISP_Tuning_GetAfHist(&af_hist);
	g_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;

	if (g_clarity_cur < g_clarity_ori) {
		g_clarity_cur = g_clarity_ori;
	}
	gettimeofday(&g_checktime, NULL);
	gettimeofday(&g_focustime, NULL);
	ts_thresh = AUTO_FOCUS_CHECK_THRESH;

	pthread_mutex_unlock(&af_mutex);

	int flag_motor_standby = 0;
	int af_on = 0;
	struct timeval cur_ts;
	uint64_t ts1 = 0, ts2 = 0;
	int first_af = 0;
	int clarity_now = 0;
	int clarity_diff = 0;
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
					Ucamera_DEBUG("vcm move 0, low power mode \n ");
					if (ret < 0) {
						Ucamera_LOG("ERROR(%s, %d): ioctl err! \n", __func__, __LINE__);
						CDC_LOG(UVC_AUTO_FOCUS_ERROR, "autoFocus error");
					}
				}
			}
			motor_restart = 1;

			usleep(1000 * 1000);
			/* update stream_on */
			stream_on = sample_get_ucamera_streamonoff();
		}

		if (motor_restart) {
			cur_pos = 500;
			ioctl(g_fd, MOTOR_MOVE, cur_pos);
			motor_restart = 0;
			usleep(100 * 1000);
		}

		if (first_af) {
			pthread_mutex_lock(&af_mutex);

			g_clarity_ori = autoFocusClimb(fd, motor_param.motor_step);
			if (g_clarity_ori < 0) {
				Ucamera_ERR("first autoFocusClimb error\n");
				CDC_LOG(UVC_AUTO_FOCUS_ERROR, "first autoFocusClimb failed");
				pthread_mutex_unlock(&af_mutex);
				continue;
			}

			g_clarity_ori = autoFocusDobuleCheck(fd, 8);
			if (g_clarity_ori < 0) {
				Ucamera_ERR("autoFocusClimb check failed \n");
				CDC_LOG(UVC_AUTO_FOCUS_ERROR, "autoFocusClimb failed");
				pthread_mutex_unlock(&af_mutex);
				continue;
			}

			g_center_pos = cur_pos;

			/* after process */
			usleep(150 * 1000);
			IMP_ISP_Tuning_GetAfHist(&af_hist);
			g_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;
			if (g_clarity_cur < g_clarity_ori) {
				g_clarity_cur = g_clarity_ori;
			}

			gettimeofday(&g_checktime, NULL);
			gettimeofday(&g_focustime, NULL);
			ts_thresh = AUTO_FOCUS_CHECK_THRESH;

			pthread_mutex_unlock(&af_mutex);

			first_af = 0;
		}

		flag_motor_standby = 1;
		af_on = sample_get_ucamera_af_onoff();
		if (af_on) {
			ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
			if (ret < 0) {
				Ucamera_LOG("IMP_ISP_Tuning_GetAFMetrices err! \n");
				CDC_LOG(IMP_GET_AF_WEIGHT_ERROR, "IMP_ISP_Tuning_GetAfWeight error");
				usleep(34 * 1000);
				continue;
			}

			clarity_now = af_hist.af_stat.af_metrics_alt + 1;
			clarity_diff = abs(g_clarity_cur - clarity_now);
			fall_rate = (100 * clarity_diff) / (g_clarity_cur + 1);
			/*Ucamera_LOG("g_clarity_cur = %d, clarity_now = %d, fall_rate = %d, focus_trigger_value_now = %d\n", g_clarity_cur, clarity_now, fall_rate, focus_trigger_value_now);*/

			if (fall_rate > focus_trigger_value_now) {
				pthread_mutex_lock(&af_mutex);
				trigger_count = 0;
				for (int i = 0; i < 15; i++) {
					usleep(34 * 1000);
					ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
					if (ret < 0) {
						Ucamera_LOG("IMP_ISP_Tuning_GetAFMetrices err! \n");
						CDC_LOG(IMP_GET_AF_WEIGHT_ERROR, "IMP_ISP_Tuning_GetAfWeight error");
						pthread_mutex_unlock(&af_mutex);
						usleep(34 * 1000);
						continue;
					}
					clarity_now = af_hist.af_stat.af_metrics_alt + 1;
					clarity_diff = abs(g_clarity_cur - clarity_now);
					fall_rate = (100 * clarity_diff) / (g_clarity_cur + 1);
					//Ucamera_LOG("g_clarity_cur = %d, clarity_now = %d, fall_rate = %d, focus_trigger_value_now = %d\n", g_clarity_cur, clarity_now, fall_rate, focus_trigger_value_now);

					/*Ucamera_DEBUG("TRIGGER FOR  :  g_clarity_cur = %d, clarity_now = %d, fall_rate = %d, focus_trigger_value_now = %d\n", g_clarity_cur, clarity_now, fall_rate, focus_trigger_value_now);*/
					if (fall_rate > focus_trigger_value_now) {
						trigger_count++;
					}
				}

				/*Ucamera_LOG("trigger_count is %d\n",trigger_count);*/
				if (trigger_count >= 3) {

					//Ucamera_DEBUG("g_clarity_cur = %d, clarity_now = %d, fall_rate = %d, focus_trigger_value_now = %d\n", g_clarity_cur, clarity_now, fall_rate, focus_trigger_value_now);
#if UCAMERA_DEBUG
					gettimeofday(&ts_s , NULL);
#endif

					int clarity1, clarity2, tmp_diff = 0;
autoclimb:
					g_clarity_ori = autoFocusClimb(fd, motor_param.motor_step);
					if (g_clarity_ori < 0) {
						Ucamera_ERR("autoFocusClimb failed \n");
						CDC_LOG(UVC_AUTO_FOCUS_ERROR, "autoFocusClimb failed");
						pthread_mutex_unlock(&af_mutex);
						continue;
					}
					clarity1 = g_clarity_ori;

					g_clarity_ori = autoFocusDobuleCheck(fd, 8);
					if (g_clarity_ori < 0) {
						Ucamera_ERR("autoFocusClimb check failed \n");
						CDC_LOG(UVC_AUTO_FOCUS_ERROR, "autoFocusClimb failed");
						pthread_mutex_unlock(&af_mutex);
						continue;
					}

					clarity2 = g_clarity_ori;
					if (clarity2 < clarity1) {
						tmp_diff = abs(clarity2 - clarity1) * 100 / (clarity1 + 1);
						if (tmp_diff > 25) {
							Ucamera_DEBUG("############## clarity1 = %d, clarity2 = %d, diff = %d, restart autoclimb  #############\n", 
									clarity1, clarity2, tmp_diff);
							goto autoclimb;
						}
					}

					/**/
					/*
					 *g_clarity_ori = autoFocusScan(fd, 10);
					 *if (g_clarity_ori < 0) {
					 *    Ucamera_ERR("autoFocusScan failed \n");
					 *    CDC_LOG(UVC_AUTO_FOCUS_ERROR, "autoFocusScan failed");
					 *    pthread_mutex_unlock(&af_mutex);
					 *    continue;
					 *}
					 */
#if UCAMERA_DEBUG
					gettimeofday(&ts_d, NULL);
#endif
					Ucamera_DEBUG("####AF time = %llums ####\n", ((((unsigned long long )ts_d.tv_sec * 1000000) + ts_d.tv_usec) - ((( unsigned long long)ts_s.tv_sec * 1000000) + ts_s.tv_usec)) / 1000);

					/* after process */
					usleep(150 * 1000);
					ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
					if(ret < 0){
						Ucamera_LOG("IMP_ISP_Tuning_GetAFMetrices err! \n");
						CDC_LOG(IMP_GET_AF_WEIGHT_ERROR, "IMP_ISP_Tuning_GetAfWeight error");
						usleep(34 * 1000);
						pthread_mutex_unlock(&af_mutex);
						continue;
					}
					g_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;

					if (g_clarity_cur < g_clarity_ori) {
						g_clarity_cur = g_clarity_ori;
					}

					/* get timestamp */
					gettimeofday(&g_checktime, NULL);
					gettimeofday(&g_focustime, NULL);
					ts_thresh = AUTO_FOCUS_CHECK_THRESH;
				}
/*
 *                } else {
 *                    g_clarity_ori = autoFocusCheck(fd, 8);
 *
 *                    [> after process <]
 *                    usleep(150 * 1000);
 *                    ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
 *                    if(ret < 0){
 *                        Ucamera_LOG("IMP_ISP_Tuning_GetAFMetrices err! \n");
 *                        CDC_LOG(IMP_GET_AF_WEIGHT_ERROR, "IMP_ISP_Tuning_GetAfWeight error");
 *                        usleep(34 * 1000);
 *                        pthread_mutex_unlock(&af_mutex);
 *                        continue;
 *                    }
 *                    g_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;
 *                    if (g_clarity_cur < g_clarity_ori) {
 *                        g_clarity_cur = g_clarity_ori;
 *                    }
 *
 *                    Ucamera_DEBUG("trigger_count is 0 ~3 \n");
 *                    [> get timestamp <]
 *                    gettimeofday(&g_checktime, NULL);
 *                    gettimeofday(&g_focustime, NULL);
 *                    ts_thresh = AUTO_FOCUS_CHECK_THRESH;
 *                }
 */
				pthread_mutex_unlock(&af_mutex);
			} else {
				usleep(34 * 1000);
			}
		} else {
			int focus_cur;
			focus_cur = sample_get_ucamera_af_cur();
			if (g_dac_pos != focus_cur) {
				g_dac_pos = focus_cur;
				ret = ioctl(fd, MOTOR_MOVE, g_dac_pos);
				if (ret < 0) {
					Ucamera_LOG("ERROR(%s, %d): ioctl err! \n", __func__, __LINE__);
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
	int fd = g_fd;
	int stream_on = 0;
	unsigned long long ts_diff = 0;
	unsigned long long ts_focus_diff = 0;
	struct timeval cur_ts;
	IMPISPAFHist af_hist;
	int re_cnt = 0;
	int tmp_clarity = 0;
	int tmp_diff = 0;

	ts_thresh = STABLE_CHECK_THRESH;

	prctl(PR_SET_NAME, "af_check_process");
	while (1) {
		stream_on = sample_get_ucamera_streamonoff();
		while (!stream_on) {
			stream_on = sample_get_ucamera_streamonoff();
			usleep(1000 * 1000);
		}

		af_on = sample_get_ucamera_af_onoff();
		if (af_on && stream_on) {
			pthread_mutex_lock(&af_mutex);

			gettimeofday(&cur_ts, NULL);
			ts_diff = ((unsigned long long)cur_ts.tv_sec * 1000000 + cur_ts.tv_usec) - ((unsigned long long)g_checktime.tv_sec * 1000000 + g_checktime.tv_usec);
			ts_focus_diff = ((unsigned long long)cur_ts.tv_sec * 1000000 + cur_ts.tv_usec) - ((unsigned long long)g_focustime.tv_sec * 1000000 + g_focustime.tv_usec);

			if ((ts_focus_diff > AUTO_FOCUS_CHECK_STAND) && (ts_thresh == AUTO_FOCUS_CHECK_THRESH)) 
				ts_thresh = STABLE_CHECK_THRESH;

			if (ts_diff > ts_thresh) {
				g_clarity_ori = autoFocusCheck(fd, 8);
				if (g_clarity_ori < 0) {
						Ucamera_ERR("autoFocusClimb check failed \n");
						CDC_LOG(UVC_AUTO_FOCUS_ERROR, "autoFocusCheck failed");
						pthread_mutex_unlock(&af_mutex);
						continue;
				}

				tmp_clarity = g_clarity_ori;
				if (tmp_clarity < g_clarity_cur) {
					tmp_diff = abs(tmp_clarity - g_clarity_cur) * 100 / (g_clarity_cur + 1);
					if (tmp_diff > 25) {
						re_cnt++;
						if (re_cnt > 3) {
							goto check; 
						}
						gettimeofday(&g_checktime, NULL);
						pthread_mutex_unlock(&af_mutex);
						continue;
					}
				}
check:
				re_cnt = 0;
				usleep(80 * 1000);
				IMP_ISP_Tuning_GetAfHist(&af_hist);
				g_clarity_cur = af_hist.af_stat.af_metrics_alt + 1;

				if (g_clarity_cur < g_clarity_ori) {
					g_clarity_cur = g_clarity_ori;
				}

				/* get timestamp */
				gettimeofday(&g_checktime, NULL);
			}

			pthread_mutex_unlock(&af_mutex);

			usleep(34 * 1000);
		} else {
			sleep(1);
		}
	}

	return NULL;
}

void sample_af_control_vcm_param_init(void *param)
{
	int ret = -1;
	motor_param = *((vcm_motor_param_t *)param);

	pthread_mutex_init(&af_mutex, NULL);

	int fd = open(VCM_DEV_NAME, 0);
	if (fd < 0) {
		Ucamera_LOG("ERROR(%s, %d):open %s err! \n", __func__, __LINE__, VCM_DEV_NAME);
		return;
	}

	g_fd = fd;
	cur_pos = 500;
	ioctl(fd, MOTOR_MOVE, cur_pos);

	pthread_t tid_af_check;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	ret = pthread_create(&tid_af_check, &attr, af_check_process, NULL);
	if (ret != 0) {
		Ucamera_LOG("pthread_create af_check_process err \n");
		return ;
	}
}
