#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_person_tracker.h>
#include <motor_process.h>
#include "motor_control.h"

#define PI                          (3.1415926535898)
#define DEC                         (PI / 180.0f) //度->弧度
#define RAD                         (180.0f / PI) //弧度->度
#define SENSOR_WIDTH_SECOND         (640)

#define MOTOR_DIRECTION_POSITIVE    (1)
#define MOTOR_DIRECTION_NEGATIVE    (-1)

/* motor status */
#define MOTOR_STATUS_STOP           (1)
#define MOTOR_STATUS_RUNNING        (2)

static int motor_status = MOTOR_STATUS_STOP;
static int motor_dir = 0;
static int speed_percent = 1.0;

static int calulate_motor_steps(int dx, int h_angle)
{
	if (!(h_angle > -180 && h_angle < 180)) {
		printf("INFO(%s): the angle is overflow 180\n", __func__);
		return -1;
	}

	double h = 0.0;
	double dx_angle = 0.0;
	double dx_steps = 0.0;

	h = (1.0 * SENSOR_WIDTH_SECOND / 2) / tan(DEC * h_angle / 2);
	dx_angle = RAD * atan(abs(dx) / h);                            /* degree */

	dx_steps = 1.0 * MAX_MOTOR_MOVE_STEP * dx_angle / 360;
	if (dx < 0) {
		dx_steps = -dx_steps;
	}

	return (int)dx_steps;
}

static void motor_set_speed(int motor_steps)
{
	int speed = 0;
	float speed_ratio = SPEED_RATIO;

	if (abs(motor_steps) <= 100) {
		speed_ratio = SPEED_RATIO;                   /* little move */
	} else if (abs(motor_steps) > 100 && abs(motor_steps) <= 300) {
		speed_ratio = SPEED_RATIO + 0.5;             /* normal */
	} else if (abs(motor_steps) > 300) {
		speed_ratio = SPEED_RATIO + 1;               /* track failed or near dist */
	}

	/* calulate speed and set */
	speed = abs(motor_steps) * speed_ratio;
	speed = speed * speed_percent;
	setMotorPtz1(motor_steps, speed);

	if (motor_steps > 0) {
		motor_dir = MOTOR_DIRECTION_POSITIVE;
	} else {
		motor_dir = MOTOR_DIRECTION_NEGATIVE;
	}

	return;
}

static void motor_stop(void)
{
	/* stop motor */
	motor_control_stop();

	/* update motor logic status */
	motor_status = MOTOR_STATUS_STOP;

	return;
}

static void motor_start(int motor_steps)
{
	/* start motor */
	motor_set_speed(motor_steps);

	/* update motor status */
	motor_status = MOTOR_STATUS_RUNNING;

	return;
}

void MotorTrackStop()
{
	motor_stop();
}

int MotorTrackProcess(void *result, int lens_hov, float run_speed_percent)
{
	int ret = -1;
	double pratio = 0.0;
	double shake_ratio = MOTOR_SHAKE_RATIO;
	int pwidth = 0;
	int motor_steps = 0;

	if (!result) {
		return -1;
	}

	speed_percent = run_speed_percent;

        person_tracker_param_output_t *r = (person_tracker_param_output_t*)result;
	motor_message_t jb_motor_message;

	motor_steps = calulate_motor_steps(r->dx, lens_hov);

	pwidth = r->rect[0].br.x - r->rect[0].ul.x;
	if (pwidth <= 0) {
		printf("ERROR:pwidth == %d!!!\n", pwidth);
	} else {
		pratio = fabs(1.0 * motor_steps / pwidth);
	}

	memset(&jb_motor_message, 0, sizeof(motor_message_t));
	ret = motor_control_get_status(&jb_motor_message);
	if (ret < 0) {
		printf("ERROR(%s): ioctl MOTOR_GET_STATUS error \n", __func__);
		return -1;
	}
	/* printf("pwidth == %d, pratio == %f, motor_steps == %d, motor_status == %d\n", pwidth, pratio, motor_steps, jb_motor_message.status); */

	if (jb_motor_message.status == MOTOR_IS_STOP) {
		motor_status = MOTOR_STATUS_STOP;
	}

	/* motor start or stop */
	if (motor_status == MOTOR_STATUS_RUNNING) {
		/* printf("r->tracking == %d, r->capture_lost == %d\n", r->tracking, r->capture_lost); */
		/* condition1 : algorithm track miss */
		if((r->tracking == 0 && r->capture_lost == 0) || (r->tracking == 1 && r->capture_lost == 1)) {
			/* stop motor */
			MotorTrackStop();

			return -2;
		}
	} else if (motor_status == MOTOR_STATUS_STOP) {
		/* defend shake */
		if (abs(motor_steps) >= MOTOR_SHAKE_STEP && pratio >= shake_ratio) {
			motor_start(motor_steps);
			return 0;
		}
	}

	/* filter false motor change direction */
	if ((motor_steps > 0 && motor_dir == MOTOR_DIRECTION_POSITIVE) ||
	    (motor_steps < 0 && motor_dir == MOTOR_DIRECTION_NEGATIVE)) {
		if (abs(motor_steps) < MOTOR_STOP_STEP_THRESH)
			return 0;
	}

	/* update motor speed */
	if (motor_status == MOTOR_STATUS_RUNNING) {
		if (abs(motor_steps) >= MOTOR_SHAKE_STEP && pratio >= shake_ratio) {
			shake_ratio = MOTOR_SHAKE_RATIO;
			motor_set_speed(motor_steps);
		}
	}

	return 0;
}
