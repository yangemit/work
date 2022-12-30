/*
 * uac_control.c
 *
 * Copyright (C) 2022 Ingenic Semiconductor Co.,Ltd
 */

#include <stdio.h>
#include <string.h>
#include <usbcamera.h>
#include <imp-common.h>

#include "uac_control.h"
#include "global_config.h"

#define MODULE_TAG "UAC_CONTROL"

static struct Ucamera_Audio_CB_Func a_func;

static audio_info_t uac_ctx;

static int audio_switch = 0;

void set_audio_switch(int on_off){
	
	audio_switch = on_off;
	
}

static int sample_audio_dmic_pcm_get(short *pcm)
{

	int ret;
	IMPDmicChnFrame g_chnFrm;

	ret = IMP_DMIC_PollingFrame(0, 0, 1000);
	if (ret != 0) {
		IMP_LOG_ERR(MODULE_TAG, "dmic polling frame data error.\n");
		return -1;
	}
	ret = IMP_DMIC_GetFrame(0, 0, &g_chnFrm, BLOCK);
	if(ret < 0) {
		IMP_LOG_ERR(MODULE_TAG, "IMP_DMIC_GetFrame failed\n");
		return -1;
	}

	memcpy((void*)pcm,g_chnFrm.rawFrame.virAddr, g_chnFrm.rawFrame.len);
	ret = IMP_DMIC_ReleaseFrame(0, 0, &g_chnFrm) ;
	if (ret < 0) {
		IMP_LOG_ERR(MODULE_TAG, "IMP_DMIC_ReleaseFrame failed.\n");
		return -1;
	}
	return g_chnFrm.rawFrame.len;
}

static int sample_audio_amic_pcm_get(short *pcm)
{

	/* Step 6: get audio record frame. */
	int ret = 0;
	int devID = 1;
	int chnID = 0;
	IMPAudioFrame frm;
	if(audio_switch == 0){
	if(IMP_AI_SetVolMute(devID, chnID, 0) != 0){
		IMP_LOG_ERR(MODULE_TAG, "Audio SetVolMuteSetVolMute disable error\n");
		}
	}else{
	if(IMP_AI_SetVolMute(devID, chnID, 1) != 0 ){
		IMP_LOG_ERR(MODULE_TAG, "Audio SetVolMuteSetVolMute enable error\n");
		}	
	}
	ret = IMP_AI_PollingFrame(devID, chnID, 1000);
	if (ret != 0 ) {
		IMP_LOG_ERR(MODULE_TAG, "Audio Polling Frame Data error\n");
	}
	ret = IMP_AI_GetFrame(devID, chnID, &frm, BLOCK);
	if(ret != 0) {
		IMP_LOG_ERR(MODULE_TAG, "Audio Get Frame Data error\n");
		return 0;
	}

	/* Step 7: Save the recording data to a file. */
	memcpy((void *)pcm, frm.virAddr, frm.len);

	//struct timeval start;
	//gettimeofday(&start,NULL);
	//unsigned long long start_1 = 1000000 * start.tv_sec  +  start.tv_usec;
	//printf("%s: timestamp = %llu star_1 = %llu.\n",__func__, frm.timeStamp, start_1);
	/* Step 8: release the audio record frame. */
	ret = IMP_AI_ReleaseFrame(devID, chnID, &frm);
	if(ret != 0) {
		IMP_LOG_ERR(MODULE_TAG, "Audio release frame data error\n");
		return 0;
	}
	
	return frm.len;

}

static int g_vol = 0;
static int sample_set_mic_volume(int db)
{
	int ret, vol;
	vol = db * 1.2 - 30;
	g_vol = vol;

	printf("set mic volume db:0x%x percent:%d\n", db, vol);
	if (uac_ctx.dmic_en)
		ret = IMP_DMIC_SetVol(0, 0, vol);
	else
		ret = IMP_AI_SetVol(1, 0, vol);

	if(ret != 0) {
		IMP_LOG_ERR(MODULE_TAG, "Audio Record set volume failed\n");
		return -1;
	}

	return 0;
}

static int sample_set_mic_mute(int mute)
{
	int ret = -1;
	printf("set mic volume mute:%d\n", mute);

	if (uac_ctx.dmic_en) {
		if (mute == 1)
			ret = IMP_DMIC_SetVol(0, 0, -30);
		else
			ret = IMP_DMIC_SetVol(0, 0, g_vol);
	} else
		ret = IMP_AI_SetVolMute(1, 0, mute);
	if(ret != 0) {
		IMP_LOG_ERR(MODULE_TAG, "Audio Record set volume failed\n");
		return -1;
	}

	return 0;
}

static int sample_set_spk_volume(int db)
{
	int ret, vol;

	vol = db - 30;
	/*vol = aduio_spk_volume_vert(db);*/
	printf("set spk volume db:0x%x percent:%d\n", db, vol);

	ret = IMP_AO_SetVol(0, 0, vol);
	if (ret != 0) {
		IMP_LOG_ERR(MODULE_TAG, "Audio Play set volume failed\n");
		return -1;
	}
	/* speak_volume_write_config(db); */

	return 0;
}

static int sample_set_spk_mute(int mute)
{
	printf("set spk volume mute:%d\n", mute);

	int ret = IMP_AO_SetVolMute(0, 0, mute);
	if (ret != 0) {
		IMP_LOG_ERR(MODULE_TAG, "Audio Play set volume failed\n");
		return -1;
	}

	return 0;
}

static int sample_get_record_off()
{
	printf("------RECORD  OFF------\n");
	return 0;
}

static int sample_get_record_on()
{
	printf("------RECORD  ON------\n");
	return 0;
}

static int sample_get_speak_off()
{
	printf("------SPEAK  OFF------\n");
	return 0;
}

static int sample_get_speak_on()
{
	printf("------SPEAK  ON------\n");
	return 0;
}

static void register_audio_func()
{
	if (uac_ctx.dmic_en)
		a_func.get_AudioPcm = sample_audio_dmic_pcm_get;
	else
		a_func.get_AudioPcm = sample_audio_amic_pcm_get;

	a_func.set_Mic_Volume = sample_set_mic_volume;
	a_func.set_Spk_Volume = sample_set_spk_volume;
	a_func.set_Mic_Mute = sample_set_mic_mute;
	a_func.set_Spk_Mute = sample_set_spk_mute;
	a_func.get_record_on = sample_get_record_on;
	a_func.get_record_off = sample_get_record_off;
	a_func.get_speak_on = sample_get_speak_on;
	a_func.get_speak_off = sample_get_speak_off;
	Ucamera_Audio_Regesit_CB(&a_func);
}

static void imp_audio_deinit()
{
	if(uac_ctx.audio_en == 1){
		if (uac_ctx.dmic_en)
			sample_audio_dmic_exit();
		else
			sample_audio_amic_exit();
	}
}

static void imp_audio_init(void)
{
	if(uac_ctx.audio_en == 1) {
		if (uac_ctx.dmic_en) {
			sample_audio_dmic_init();
		}
		else {
			int audio_ns = uac_ctx.audio_ns;
			int vol = uac_ctx.mic_vol;
			sample_audio_amic_init(vol, audio_ns);
		}
	}
}

int uac_control_init(void *param)
{
	static int vol = 0;
	config_func_param_t *cfg_param = (config_func_param_t *)param;
	uac_ctx = cfg_param->audio_info;
	if (uac_ctx.audio_en) {
		register_audio_func();
	}

	imp_audio_init();

	if (uac_ctx.audio_en) {
		Ucamera_Audio_Start();
	}

	if (uac_ctx.spk_en) {
		 vol = uac_ctx.spk_vol;
		sample_audio_play_start(&vol);
	}

	return 0;
}

void uac_control_deinit()
{
	imp_audio_deinit();
}
