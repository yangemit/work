#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <global_config.h>

#include "motor_control.h"

#define MODULE_TAG               "motor_control"

#define MOTOR_DEV_NAME          "/dev/motor"
#define MOTOR_POSITIVE          (1)

#define MOTOR_CMD_STOP		0x1
#define MOTOR_CMD_RESET		0x2
#define MOTOR_CMD_MOVE		0x3
#define MOTOR_GET_STATUS	0x4
#define MOTOR_SET_SPEED	        0x5
#define MOTOR_PWR	        0x8
#define MOTOR_SET_PARAMS	0x9

static motor_control_ctx_t motor_ctx;
static int motor_reset(unsigned int speed)
{
	int ret = -1;

	pthread_mutex_lock(&motor_ctx.motor_mutex);
	ret = ioctl(motor_ctx.fd, MOTOR_SET_SPEED, (unsigned int) (&speed));
	pthread_mutex_unlock(&motor_ctx.motor_mutex);
	if (ret < 0) {
		printf("ERROR(%s): ioctl MOTOR_SET_SPEED error \n", MODULE_TAG);
		return -1;
	}

	motor_reset_data_t motor_reset_data;
	memset(&motor_reset_data, 0, sizeof(motor_reset_data_t));
	pthread_mutex_lock(&motor_ctx.motor_mutex);
	ret = ioctl(motor_ctx.fd, MOTOR_CMD_RESET,&motor_reset_data);
	pthread_mutex_unlock(&motor_ctx.motor_mutex);
	if (ret < 0) {
		printf("INFO(%s): ioctl MOTOR_CMD_RESET error \n", MODULE_TAG);
		return -1;
	}
	printf("INFO(%s): Motor Reset Ok\n", MODULE_TAG);
	return 0;
}

int motor_control_move(motors_steps_t jb_motors_steps)
{
	int ret = -1;

	pthread_mutex_lock(&motor_ctx.motor_mutex);
	ret = ioctl(motor_ctx.fd, MOTOR_CMD_MOVE, (unsigned long) &jb_motors_steps);
	pthread_mutex_unlock(&motor_ctx.motor_mutex);
	if (ret < 0) {
		printf("ERROR(%s): ioctl MOTOR_CMD_STOP error \n", MODULE_TAG);
		return -1;
	}

	return 0;
}

int motor_control_set_speed(unsigned int s_speed)
{
	int ret = -1;
	pthread_mutex_lock(&motor_ctx.motor_mutex);
	ret = ioctl(motor_ctx.fd, MOTOR_SET_SPEED, (unsigned int)(&s_speed));
	pthread_mutex_unlock(&motor_ctx.motor_mutex);
	if (ret < 0) {
		printf("WARNNING(%s): ioctl MOTOR_SET_SPEED error \n", MODULE_TAG);
	}

	return 0;
}

static int motor_init(int speed)
{
	int ret = -1;
	/*Step1: open dev/motor*/
	motor_ctx.fd = open(MOTOR_DEV_NAME, O_RDONLY);
	if (motor_ctx.fd < 0) {
		printf("ERROR(%s): open %s failed \n", MODULE_TAG, MOTOR_DEV_NAME);
		return -1;
	}

	/* Step2: motor power ctrl */
	int tmp = 1;
	pthread_mutex_lock(&motor_ctx.motor_mutex);
	ret = ioctl(motor_ctx.fd, MOTOR_PWR, (unsigned int) (&tmp));
	pthread_mutex_unlock(&motor_ctx.motor_mutex);
	if (ret < 0) {
		printf("ERROR(%s): ioctl MOTOR_PWR error \n", MODULE_TAG);
		return -1;
	}


	/* Step3: motor params set */
	motor_set_params_t motor_params;
	memset(&motor_params, 0, sizeof(motor_set_params_t));
	motor_params.hmaxstep = motor_ctx.hmaxstep;
	motor_params.islimit = motor_ctx.islimit;
	pthread_mutex_lock(&motor_ctx.motor_mutex);
	ret = ioctl(motor_ctx.fd, MOTOR_SET_PARAMS, (unsigned int) (&motor_params));
	pthread_mutex_unlock(&motor_ctx.motor_mutex);
	if (ret < 0) {
		printf("ERROR(%s): ioctl MOTOR_SET_PARAMS error \n", MODULE_TAG);
		return -1;
	}
	/* Step4: motor reset*/
	ret = motor_reset(speed);
	if (ret < 0) {
		printf("ERROR(%s): motor reset failed \n", MODULE_TAG);
		return -1;
	}

	return 0;
}

int motor_control_get_status(motor_message_t *jb_motor_message)
{
	int ret = -1;
	pthread_mutex_lock(&motor_ctx.motor_mutex);
	ret = ioctl(motor_ctx.fd, MOTOR_GET_STATUS, (unsigned long)jb_motor_message);
	pthread_mutex_unlock(&motor_ctx.motor_mutex);
	if (ret < 0) {
		printf("ERROR(%s): ioctl MOTOR_GET_STATUS error \n", MODULE_TAG);
		return -1;
	}

	return 0;
}

int motor_control_stop(void)
{
	int ret = -1;

	pthread_mutex_lock(&motor_ctx.motor_mutex);
	ret = ioctl(motor_ctx.fd, MOTOR_CMD_STOP);
	pthread_mutex_unlock(&motor_ctx.motor_mutex);
	if (ret < 0) {
		printf("WARNNING(%s): ioctl MOTOR_CMD_STOP error \n", MODULE_TAG);
		return -1;
	}

	return 0;
}

int motor_control_init(const void *param)
{
	int ret = -1;
	/* param init */
	/* config_function_param *_param = (config_function_param *)param; */
	motor_track_attr_t *_param = (motor_track_attr_t *)param;

	memset(&motor_ctx, 0, sizeof(motor_control_ctx_t));

	motor_ctx.motor_reset_speed = _param->reset_speed;
	motor_ctx.islimit = _param->islimit;
	motor_ctx.hmaxstep = _param->hmaxstep;
	motor_ctx.motor_direction = _param->direction;

	pthread_mutex_init(&motor_ctx.motor_mutex, NULL);

	ret = motor_init(motor_ctx.motor_reset_speed);
	if (ret < 0) {
		printf("ERROR(%s): motor_init failed !\n", MODULE_TAG);
		close(motor_ctx.fd);
		return -1;
	}

	return 0;
}

void motor_control_deinit()
{
	close(motor_ctx.fd);
}

void setMotorPtz1(int step, int speed)
{
	motors_steps_t jb_motors_steps;
	memset(&jb_motors_steps, 0, sizeof(motors_steps_t));

	if (motor_ctx.motor_direction != MOTOR_POSITIVE) {
		step = -step;
	}

	jb_motors_steps.x = step;
	if (speed > MAX_MOTOR_MOVE_SPEED) {
		speed = MAX_MOTOR_MOVE_SPEED;
	} else if (speed < MIN_MOTOR_MOVE_SPEED) {
		speed = MIN_MOTOR_MOVE_SPEED;
	}

	motor_control_set_speed(speed);
	motor_control_move(jb_motors_steps);
}
