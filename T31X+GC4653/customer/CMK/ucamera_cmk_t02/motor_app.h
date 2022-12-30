#ifndef _MOTOR_APP_H_
#define _MOTOR_APP_H
typedef enum{
	eMOTOR_POS_UNKNOW=0,
	eMOTOR_POS_LEFT,
	eMOTOR_POS_RIGHT,
}eMOTOR_POS;
typedef enum{
	eMOTOR_ACTION_IDLE=0,
	eMOTOR_ACTION_LEFT,
	eMOTOR_ACTION_RIGHT,
}eMOTOR_ACTION;
extern int open_motor_dev(void);
extern int motor_set_speed(unsigned int speed);
extern int motor_close_door(void);
extern int motor_open_door(void);
extern int motor_get_pos(void);
extern int motor_stop(void);
extern int motor_get_status(void);
extern char motor_door_opened(void);
extern char motor_door_closed(void);
#endif
