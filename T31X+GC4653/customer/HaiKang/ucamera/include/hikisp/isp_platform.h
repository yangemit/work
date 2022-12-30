/** @file isp_platform.h
*	@note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*	@brief ƽ̨���ͷ�ļ�
*	
*	@author   yaojinbo
*	@date	  2015-05-26
*	@version  1.0
*	@note ����������ѧ�궨�塢c��ͷ�ļ���ƽ̨ͷ�ļ���
*	@note History:		  
*	@note	  <author>	 <time>    <version >	<desc>
*	@note	  yaojinbo	  2015-05-28  �޸�
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
	DBG_KEY      = 0x1,						//��ӡϵͳ�ؼ���Ϣ��error��ISP��ʼ����Ϣ
	DBG_FUNCTION = 0x2,				//��ӡISPģ�鹦�ܡ�������Ϣ
	DBG_COULOR    =0x4,						//��ӡAWB��CCM��GBCE�����Ϣ
	DBG_AE_REG   = 0x8,						//��ӡ�ع�д�Ĵ�����addr��val
	DBG_AE       = 0x10,					//��ӡ�ع���Ϣ
	DBG_DAYNIGHT = 0x20,					//��ӡ��ҹ�л�����
	DBG_ADJUST 	 = 0x40						//��ӡ��Ȧ���������صȶ�̬��Ϣ
}DEBUG_INFO;

#define FH_VI_MODE_12BIT_ISP	5			//sensor�ӿ�ʹ�ܣ�12bit sensor���ݣ�ˮƽ�źŵ���Ч����ֱ�źŸ���Ч

#define GPIO_IOC_MAGIC    'G'
#define HAL_GPIO_SET_VALUE  _IOW(GPIO_IOC_MAGIC, 4, struct gpio_ctrl*)
#define HAL_GPIO_DIR_OUTPUT _IOW(GPIO_IOC_MAGIC, 2, struct gpio_ctrl*)

#define GPIO_LOW					0x0		// GPIO�ܽŵ͵�ƽ
#define GPIO_HIGH					0x1		// GPIO�ܽŸߵ�ƽ

extern void setPthreadName(char *name);


typedef struct gpio_ctrl {
	unsigned int gpio;  ///< GPIO no
	unsigned int param; ///< set / get value / pinmux
}GPIO_IOCTL;


#ifdef __cplusplus
}
#endif


#endif /** __ISP_PLATFORM_H__ */

