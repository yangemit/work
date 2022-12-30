#ifndef __MOTOR_PROCESS__H
#define __MOTOR_PROCESS__H

/* stop params */
#define MOTOR_STOP_STEP_THRESH      (34)

/* speed params */
#define SPEED_RATIO                 (2)

/* shake params */
#define MOTOR_SHAKE_STEP            (30)
#define MOTOR_SHAKE_RATIO           (0.2)
#define FRAME_RATIO                 (1.0 / 2)
#define MOTOR_DIR_CHANGE_CNT        (3)
#define MOTOR_DIR_INTERVAL_TIME     (500 * 1000)

/*@function: MotorTrackProcess
 *@brief:    the motor process for human track
 *@params:
 *           result:   algo result
 *           lens_hov: the hov of sensors
 *           run_speed_percent : adjust the speed (default 1.0)
 *@return:
 *           0: true
 *          -1: false
 */
int MotorTrackProcess(void *result, int lens_hov, float run_speed_percent);

/*@function: MotorTrackStop
 *@brief:    stop the motor and set the motor status
 *@params:   no
 *@return:   no
 */
void MotorTrackStop();

#endif
