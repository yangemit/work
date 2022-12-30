/** @file isp_platform.h
*	@note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*	@brief 平台相关头文件
*	
*	@author   yaojinbo
*	@date	  2015-05-26
*	@version  1.0
*	@note 包含常用数学宏定义、c库头文件和平台头文件。
*	@note History:		  
*	@note	  <author>	 <time>    <version >	<desc>
*	@note	  yaojinbo	  2015-05-28  修改
*	@warning  
*/

#ifndef __ISP_PLATFORM_H__
#define __ISP_PLATFORM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <rtthread.h>
//#include <pthread.h>
//#include <termios.h>
//#include <fcntl.h> 
//#include <unistd.h>
//#include <sys/ioctl.h>
//#include <sys/shm.h>
//#include <sys/ipc.h>
//#include <sys/wait.h>    
//#include <sys/types.h>   
//#include <linux/types.h>
//#include <linux/i2c.h>
//#include <linux/i2c-dev.h>
//#include <dlfcn.h>
#include <math.h>
//#include "dfs_posix.h"
//#include "ioctl.h"
//#include "i2c.h"

//#include "isp_api.h"
//#include "isp_sensor_if.h"
//#include "debug_interface.h"
#include "isp_enum.h"
#include "hik_golbal_def.h"
#include "type_def.h"
//#include "dbi_over_udp.h"
//#include "dbi_over_tcp.h"
//#include "fh_vpu_mpi.h"


typedef enum
{
	DBG_KEY      = 0x1,						//打印系统关键信息、error、ISP初始化信息
	DBG_FUNCTION = 0x2,				//打印ISP模块功能、配置信息
	DBG_COULOR    =0x4,						//打印AWB、CCM、GBCE相关信息
	DBG_AE_REG   = 0x8,						//打印曝光写寄存器的addr和val
	DBG_AE       = 0x10,					//打印曝光信息
	DBG_DAYNIGHT = 0x20,					//打印日夜切换参数
	DBG_ADJUST 	 = 0x40						//打印光圈、红外灯相关等动态信息
}DEBUG_INFO;

#define FH_VI_MODE_12BIT_ISP	5			//sensor接口使能；12bit sensor数据；水平信号低有效；垂直信号高有效

#define GPIO_IOC_MAGIC    'G'
#define HAL_GPIO_SET_VALUE  _IOW(GPIO_IOC_MAGIC, 4, struct gpio_ctrl*)
#define HAL_GPIO_DIR_OUTPUT _IOW(GPIO_IOC_MAGIC, 2, struct gpio_ctrl*)

#define GPIO_LOW					0x0		// GPIO管脚低电平
#define GPIO_HIGH					0x1		// GPIO管脚高电平

extern void setPthreadName(char *name);


typedef struct gpio_ctrl {
	unsigned int gpio;  ///< GPIO no
	unsigned int param; ///< set / get value / pinmux
}GPIO_IOCTL;


#ifdef __cplusplus
}
#endif


#endif /** __ISP_PLATFORM_H__ */

