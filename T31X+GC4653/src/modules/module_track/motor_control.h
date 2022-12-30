#ifndef __MOTOR_CONTROL__H
#define __MOTOR_CONTROL__H

#define MAX_MOTOR_MOVE_STEP     (4096)
#define MIN_MOTOR_MOVE_SPEED    (100)
#define MAX_MOTOR_MOVE_SPEED    (900)

enum motor_status {
	MOTOR_IS_STOP,
	MOTOR_IS_RUNNING,
};

/*motor_move */
typedef struct _motors_steps{
	int x;
	int y;
} motors_steps_t;

typedef struct _motor_message {
	int x;
	int y;
	enum motor_status status;
	int speed;
} motor_message_t;

typedef struct _motor_reset_data {
	unsigned int x_max_steps;
	unsigned int y_max_steps;
	unsigned int x_cur_step;
	unsigned int y_cur_step;
} motor_reset_data_t;

typedef struct _motor_set_params {
	int hmaxstep;
	int vmaxstep;
	int islimit;
} motor_set_params_t;

typedef struct _motor_control_ctx {
	int fd;
	int islimit;
	int hmaxstep;
	int motor_reset_speed;
	int motor_direction;
	pthread_mutex_t motor_mutex;
} motor_control_ctx_t;

int motor_control_move(motors_steps_t jb_motors_steps);
int motor_control_set_speed(unsigned int s_speed);
int motor_control_init(const void *param);
int motor_control_get_status(motor_message_t *jb_motor_message);
int motor_control_stop();
void setMotorPtz1(int dx, int speed);

#endif
