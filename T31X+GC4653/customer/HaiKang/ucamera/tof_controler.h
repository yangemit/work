#ifndef __TOF_CONTROLER_H__
#define __TOF_CONTROLER_H__	
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <linux/input.h>
#include "imp-common.h"
#include "tof/vl53lx_def.h"
/* our driver kernel interface */
#include "tof/stmvl53lx_if.h"
#include "tof/stmvl53lx_internal_if.h"
#include <getopt.h>
#include "vcm_controler.h"
#include "imp-common.h"

#define ST_OK		0
#define ST_TOF1_OPEN_ERR	-1
#define ST_TOF2_OPEN_ERR	-2
#define ST_TOF1_2_OPEN_ERR	-3
#define ST_TOF1_START_ERR	-1
#define ST_TOF1_START_ERR	-2
#define ST_TOF1_2_START_ERR	-3
#define ST_TOF1_GET_ERR		-1
#define ST_TOF2_GET_ERR		-2
#define ST_TOF1_2_GET_ERR	-3
#define ST_TOF1_STOP_ERR	-1
#define ST_TOF1_STOP_ERR	-2
#define ST_TOF1_2_STOP_ERR	-3

// tof的距离信息
struct tof_millInfo
{
	unsigned int rangeStatus;		// 范围状态
	unsigned int milliMeter;	// Tof1的返回距离
	unsigned int minMilliMeter;	// 最小距离
	unsigned int maxMilliMeter;	// 最大距离
	unsigned int sigmaMilliMeter;
	unsigned int signalRateRtnMegaCps;
	unsigned int ambientRateRtnMegaCps;
};

struct tof_data
{
	unsigned int type;			// DTof 或者 ITof，1为DTof， 2为ITof
	int check_status;				// tof检测的状态
	unsigned int tof1_timeStamp;	// 时间戳
	unsigned int tof1_streamCount;	// 流计数
	float tof1_effectiveSpadRtnCount;	// 返回信号有效铲数
	int tof1_numberOfObjectsFound;		// tof1测量到的距离个数
	// unsigned int tof1_rangeStatus;		// 范围状态
	// unsigned int tof1_milliMeter;	// Tof1的返回距离
	// unsigned int tof1_minMilliMeter;	// 最小距离
	// unsigned int tof1_maxMilliMeter;	// 最大距离
	// unsigned int tof1_sigmaMilliMeter;
	// unsigned int tof1_signalRateRtnMegaCps;
	// unsigned int tof1_ambientRateRtnMegaCps;
	struct tof_millInfo tof1[4];
	unsigned int tof2_timeStamp;	// 时间戳
	unsigned int tof2_streamCount;	// 流计数
	float tof2_effectiveSpadRtnCount;	// 返回信号有效铲数
	int tof2_numberOfObjectsFound;		// tof1测量到的距离个数
	struct tof_millInfo tof2[4];
	// unsigned int tof2_rangeStatus;		// 范围状态
	// unsigned int tof2_milliMeter;	// Tof2的返回距离
	// unsigned int tof2_minMilliMeter;	// 最小距离
	// unsigned int tof2_maxMilliMeter;	// 最大距离
	// unsigned int tof2_sigmaMilliMeter;
	// unsigned int tof2_signalRateRtnMegaCps;
	// unsigned int tof2_ambientRateRtnMegaCps;
};

extern struct tof_data tof_check;

struct tof_tag
{
	int tag;
};

int Tof_Init();
int Tof_Start();
int Tof_Stop();
int Tof_Get(struct tof_data *data);
void Tof_Print(struct tof_data *data);
int Tof_Uninit();
void thread_tof_run(void *arg);

#endif
