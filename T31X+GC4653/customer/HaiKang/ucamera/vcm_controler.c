#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "vcm_controler.h"
#include "imp-common.h"
#include "ucam/usbcamera.h"
#include "hik_config.h"


int g_iFd = -1;
int g_iCoord = -1;
int g_iRet = -1;
FILE *g_fVCM = NULL;
FILE *g_fVCM_EN = NULL;
int af_state = 0;
int af_thread_cancel = 0;
int g_VcmValue = 0;
int g_VcmEn = 1;

int AF_Init()
{
	g_iFd = open("/dev/bu64981", O_RDWR);
	if(g_iFd < 0)
	{
		Ucamera_ERR("init vcm failure... \n");
		return -1;
	}
	g_iCoord = AF_GetCoord(0);
	if(g_iCoord < 0)
	{
		g_iCoord = 0;
	}
	Ucamera_LOG("init g_iCoord = %d... \n", g_iCoord);
	return 0;
}

int AF_Uninit()
{
	if(g_iFd >= 0)
	{
		g_iRet = close(g_iFd);
		if(g_iRet > 0)
		{
			g_iFd = -1;
			return 0;
		}
		else
		{
			Ucamera_LOG("uninit vcm failure... \n");
			return -1;
		}
	}
	return -1;
}


int AF_GetMaxCoord()
{
	return 400;
}

int AF_GetMinCoord()
{
	return 0;
}

void AF_MotorCtrl(int chan, int dir, int step)
{
	// g_iCoord = 0.72 * value - 180
	if(dir > 0)
	{
		g_iCoord = g_iCoord + step;
	}
	else
	{
		g_iCoord = g_iCoord -step;
	}

	if(g_iCoord > AF_GetMaxCoord())
	{
		g_iCoord = AF_GetMaxCoord();
	}
	if(g_iCoord < AF_GetMinCoord())
	{
		g_iCoord = AF_GetMinCoord();
	}
	int value = 7 * g_iCoord / 4.0 + 100;
	g_iRet = ioctl(g_iFd, MOTOR_MOVE, value);
	if (g_iRet < 0)
	{
		Ucamera_ERR("set coord failure: step = %d, coord = %d...", step, g_iCoord);
	}
	return;
}

int AF_GetCoord(int chan)
{
	g_iRet = ioctl(g_iFd, MOTOR_READ, 0);
#if(VCM_TYPE == VCM_TYPE_BU)
	int hByte = g_iRet&0x3;
	int lByte = g_iRet>>8;
	int value = hByte<<8 | lByte;
#elif(VCM_TYPE == VCM_TYPE_AW)
	int value = g_iRet;
#else
	int value = 0;
#endif
	g_iCoord = 4 * (value - 100) / 7.0 + 0.5;
	if(g_iCoord < 0)
	{
		g_iCoord = 0;
	}
	return g_iCoord;
}

int AF_IsStreamOn(int chan)
{
	return sample_video_isOnStream();
}

int set_value_ctl_vcm(int value)
{
	if(g_iFd < 0)
	{
		AF_Init();
	}
	g_iRet = ioctl(g_iFd, MOTOR_MOVE, value);
	if (g_iRet < 0)
	{
		Ucamera_ERR("set coord failure\n");
		return -1;
	}
	return 0;
}

int get_value_ctl_vcm()
{
	if(g_iFd < 0)
	{
		AF_Init();
	}
	g_iRet = ioctl(g_iFd, MOTOR_READ, 0);
#if(VCM_TYPE == VCM_TYPE_BU)
	int hByte = g_iRet&0x3;
	int lByte = g_iRet>>8;
	int value = hByte<<8 | lByte;
#elif(VCM_TYPE == VCM_TYPE_AW)
	int value = g_iRet;
#else
	int value = 0;
#endif
	return value;
}

int read_vcm_value()
{
	if((g_fVCM = fopen("/media/vcm.attr", "r")) != NULL)
	{
		fscanf(g_fVCM, "%d", &g_VcmValue);
	}
	else
	{
		g_fVCM = fopen("/media/vcm.attr", "w+");
		fprintf(g_fVCM, "%d", 0);
		g_VcmValue = 0;
	}
	fclose(g_fVCM);
	return g_VcmValue;
}

void write_vcm_value(int value)
{
	if((g_fVCM = fopen("/media/vcm.attr", "w+")) != NULL)
	{
		fprintf(g_fVCM, "%d", value);
		g_VcmValue = value;
	}
	else
	{
		Ucamera_ERR("====>>ERROR: %d open vcm file<<====\n", __LINE__);
	}
	fclose(g_fVCM);
}

int read_vcm_en()
{
	if((g_fVCM_EN = fopen("/media/vcm.en", "r")) != NULL)
	{
		fscanf(g_fVCM_EN, "%d", &g_VcmEn);
	}
	else
	{
		g_fVCM_EN = fopen("/media/vcm.en", "w+");
		fprintf(g_fVCM_EN, "%d", 1);
		g_VcmEn = 1;
	}
	fclose(g_fVCM_EN);
	return g_VcmEn;
}

void write_vcm_en(int value)
{
	if((g_fVCM_EN = fopen("/media/vcm.en", "w+")) != NULL)
	{
		fprintf(g_fVCM_EN, "%d", value);
	}
	else
	{
		Ucamera_ERR("====>>ERROR: %d open vcm file<<====\n", __LINE__);
	}
	fclose(g_fVCM_EN);
}

void thread_vcm_check(void *arg)
{
	Ucamera_LOG("start vcm check thread...\n");
	write_vcm_value(search_peak());	
	write_vcm_en(1);
	sleep(1);
	start_auto_focus();
}

int sample_vcm_cleck()
{
	int sVcmValue = 100;
	int startAF = 0;
	int endAf = 0;
	int ret = set_value_ctl_vcm(sVcmValue);
	if(ret < 0)
	{
		Ucamera_ERR("====>>%d : vcm cleck failed<<====\n", __LINE__);
		return -1;
	}
	ret = IMP_ISP_Tuning_GetAFMetrices(&startAF);

	while(sVcmValue < 400)
	{
		sVcmValue = sVcmValue + 2;
		set_value_ctl_vcm(sVcmValue);
		IMP_ISP_Tuning_GetAFMetrices(&endAf);
	}
	Ucamera_LOG("end\n");
	return	0;
}

long long int get_daf(int chan)
 {
	 IMPISPZone af_zone = {0};
	int i, j;
	long long int	afd_value=0;
	long long int afd_clarity=0;
	int ret = IMP_ISP_Tuning_GetAfZone(&af_zone);
	if(ret < 0)
	{
		Ucamera_ERR("IMP_ISP_Tuning_GetAfZone error \r\n");
	}
	else
	{
		for (i = 0; i < 15; ++i)
		{
			for (j = 0; j < 15; ++j)
			{
				afd_clarity +=(long long int)af_zone.zone[i][j];
				printf("%d ", af_zone.zone[i][j]);
			}
			printf("\r\n");
		}
	}
	afd_value=afd_clarity;
	return afd_value;
 }

void init_af_win(int chan)
{
	int ret;
	IMPISPAFHist af_hist;
	ret=IMP_ISP_Tuning_GetAfHist(&af_hist);
	if(ret < 0)
	{
		printf("IMP_ISP_Tuning_GetAfHist err! \n");
	}
	Ucamera_LOG("af_hist.af_hstart:%d,af_hist.af_vstart:%d,af_hist.af_stat_nodeh:%d,af_hist.af_stat_nodev:%d \r\n",af_hist.af_hstart,af_hist.af_vstart,af_hist.af_stat_nodeh,af_hist.af_stat_nodev );
	af_hist.af_hstart = 0; 
	af_hist.af_vstart = 0;
	af_hist.af_stat_nodeh = 15;
	af_hist.af_stat_nodev = 15;
	af_hist.af_metrics_shift = 1;
	ret=IMP_ISP_Tuning_SetAfHist(&af_hist);
	if(ret < 0)
	{
		Ucamera_ERR("IMP_ISP_Tuning_SetAfHist err! \n");;
	}

	ret=IMP_ISP_Tuning_GetAfHist(&af_hist);
	if(ret < 0)
	{
		Ucamera_ERR("IMP_ISP_Tuning_GetAfHist err \n");;
	}
	Ucamera_LOG("af_hist.af_hstart:%d,af_hist.af_vstart:%d,af_hist.af_stat_nodeh:%d,af_hist.af_stat_nodev:%d \r\n",af_hist.af_hstart,af_hist.af_vstart,af_hist.af_stat_nodeh,af_hist.af_stat_nodev );
	return;
}

int search_peak()
{
	AF_Init();
	stop_auto_focus();
	Ucamera_LOG("+++++++++++++++++");
	InitAFVCMSetAFWin(0);
	Ucamera_LOG("------------------");
	sleep(1);
	return SearchPeakTestT2(0);
}
