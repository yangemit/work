
/*
 *  Copyright (c) 2014-5-15 SE7EN
 *
 *  Four phases step motor support
 */
 
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef _MOTOR_CTRL_H_

#define  _MOTOR_CTRL_H_

#define	MOTOR_IOC_MAGIC	'm'
#define	MOTOR_IOCGETDATA	_IOR(MOTOR_IOC_MAGIC,1,int)
#define	MOTOR_IOCSETDATA	_IOW(MOTOR_IOC_MAGIC,2,int)
#define	MOTOR_IOCSETLEFT	_IOW(MOTOR_IOC_MAGIC,3,int)
#define	MOTOR_IOCSETRIGHT	_IOW(MOTOR_IOC_MAGIC,4,int)
#define LED_LIGHT_ON       _IOW(MOTOR_IOC_MAGIC,5,int)
#define LED_LIGHT_OFF       _IOW(MOTOR_IOC_MAGIC,6,int)
#define	MOTOR_IOCGET_POS	 _IOW(MOTOR_IOC_MAGIC,7,int)

#define	MOTOR_IOC_MAXNR		7
 
typedef enum{
	eMOTOR_ACTION_IDLE=0,
	eMOTOR_ACTION_LEFT,
	eMOTOR_ACTION_RIGHT,
}eMOTOR_ACTION;
typedef enum{
	eMOTOR_POS_UNKNOW=0,
	eMOTOR_POS_LEFT,
	eMOTOR_POS_RIGHT,
}eMOTOR_POS;
#define MAX_STEP_CNT  250
#endif
