/*********************************************************
 * File Name   : imp_hsb.c
 * Author      : Elvis Wang
 * Mail        : huan.wang@ingenic.com
 * Created Time: 2021-08-03 20:03
 ********************************************************/

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/prctl.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>

#include <imp/imp_audio.h>
int g_Audio_Ns = -1;

/*
 *#ifdef T31
 *#include <imp/imp_dmic.h>
 *#endif
 *#include <imp-common.h>
 */

int sample_audio_ns_switch(int ns_switch)
{
	int ret;
	int devID = 1;
	IMPAudioIOAttr amic_attr;
	amic_attr.samplerate = AUDIO_SAMPLE_RATE_16000;// AUDIO_SAMPLE_RATE_16000;
	amic_attr.bitwidth = AUDIO_BIT_WIDTH_16;
	amic_attr.soundmode = AUDIO_SOUND_MODE_MONO;
	amic_attr.frmNum = 2;
	amic_attr.numPerFrm = 640;
	amic_attr.chnCnt = 1;
	ret = IMP_AI_SetPubAttr(devID, &amic_attr);
	if(ret != 0) {
		/*IMP_LOG_ERR(TAG, "set ai %d attr err: %d\n", devID, ret);*/
		return -1;
	}

	memset(&amic_attr, 0x0, sizeof(amic_attr));
	ret = IMP_AI_GetPubAttr(devID, &amic_attr);
	if(ret != 0) {
		/*IMP_LOG_ERR(TAG, "get ai %d attr err: %d\n", devID, ret);*/
		return -1;
	}
	if(!ns_switch){
		if (g_Audio_Ns >= 0) {
			if (g_Audio_Ns < NS_LOW || g_Audio_Ns > NS_VERYHIGH) {
				/*IMP_LOG_ERR(TAG, "Audio set unvilid NS Leave. \n");*/
		        	return -1;
		        }
			ret = IMP_AI_EnableNs(&amic_attr, g_Audio_Ns);
		        if(ret != 0) {
		        	printf("enable audio ns error.\n");
		        	return -1;
		        }
		 }
		return 0;
	}
	ret = IMP_AI_DisableNs();
	if(ret != 0) {
		printf("close audio ns error.\n");
		return -1;
	}
	return 0;

}

