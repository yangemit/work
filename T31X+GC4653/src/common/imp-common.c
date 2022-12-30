/*
 * sample-common.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

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
#include <semaphore.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>

#include <imp/imp_audio.h>
#include <imp/imp_dmic.h>

#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>
#include <png.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#define PNG_BYTES_TO_CHECK 4

#include <usbcamera.h>
#include "imp-common.h"
#include "global_config.h"

#if (defined MODULE_TRACK_ENABLE || defined MODULE_FACEAE_ENABLE)
/* #include <ivs/ivs_common.h> */
/* #include <ivs/ivs_interface.h> */
#endif
#include <imp/imp_ivs.h>

#ifdef MODULE_FACEAE_ENABLE
#include <ivs/ivs_inf_faceDet.h>
#endif
#ifdef MODULE_TRACK_ENABLE
/* #include <ivs/ivs_inf_person_tracker.h> */
#endif


#define TAG "IMP-COMMON"


static IMPRgnHandle osd_hander[1] ;

typedef struct _imp_common_attr {
	int sensor_width;
	int sensor_height;
	int low_power;
	int rcmode;
	int bitrate;
	int gop;
	int qp_value;
} imp_common_attr_t;

static imp_common_attr_t imp_ctx = {
	.sensor_width = 1920,
	.sensor_height = 1080,
	.rcmode = IMP_ENC_RC_MODE_CBR,
	.bitrate = 1000,
	.gop = SENSOR_FRAME_RATE_NUM_25,
	.qp_value = 80,
};

/*#define SHOW_FRM_BITRATE*/
#ifdef SHOW_FRM_BITRATE
#define FRM_BIT_RATE_TIME 2
#define STREAM_TYPE_NUM 3
static int frmrate_sp[STREAM_TYPE_NUM] = { 0 };
static int statime_sp[STREAM_TYPE_NUM] = { 0 };
static int bitrate_sp[STREAM_TYPE_NUM] = { 0 };
#endif

struct chn_conf chn[FS_CHN_NUM] = {
	{
		.index = CH0_INDEX,
		.enable = CHN0_EN,
		.payloadType = IMP_ENC_PROFILE_AVC_MAIN,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM_25,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH_THIRD,
			.crop.height = SENSOR_HEIGHT_THIRD,

			.scaler.enable = 0,
			.scaler.outwidth = SENSOR_WIDTH_THIRD,
			.scaler.outheight = SENSOR_HEIGHT_THIRD,
			.picWidth = SENSOR_WIDTH_THIRD,
			.picHeight = SENSOR_HEIGHT_THIRD,
		   },
		.framesource_chn =	{ DEV_ID_FS, CH0_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH0_INDEX, 0},
		.imp_osd = {DEV_ID_OSD,CH0_INDEX,0},
	},
	{
		.index = CH1_INDEX,
		.enable = 1,
		/* .enable = CHN1_EN, */
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = SENSOR_FRAME_RATE_NUM_25,
			.outFrmRateDen = SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = SENSOR_WIDTH_SECOND,
			.crop.height = SENSOR_HEIGHT_SECOND,

			.scaler.enable = 1,
			.scaler.outwidth = SENSOR_WIDTH_SECOND,
			.scaler.outheight = SENSOR_HEIGHT_SECOND,

			.picWidth = SENSOR_WIDTH_SECOND,
			.picHeight = SENSOR_HEIGHT_SECOND,
		   },
		.framesource_chn =	{ DEV_ID_FS, 1, 0},
		.imp_encoder = { DEV_ID_ENC, 1, 0},
	},
};

IMPRgnHandle ivsRgnHandler[MAX_IVS_OSD_REGION] = {INVHANDLE};
int osd_init(int grpNum, int ch)
{
	if (IMP_OSD_CreateGroup(grpNum) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateGroup(%d) error !\n", grpNum);
		return -1;
	}

	/* IMPCell osdcell = {DEV_ID_OSD, grpNum, 0}; */
	/* IMPCell *dstcell = &chn[ch].imp_encoder; */
	/* if (IMP_OSD_AttachToGroup(&osdcell, dstcell) < 0) { */
	/* 	IMP_LOG_ERR(TAG, "IMP_OSD_AttachToGroup() error !\n"); */
	/* 	return -1; */
	/* } */

	IMPOSDRgnAttr rAttr = {
		.type = OSD_REG_RECT,
		.rect = {{0, 0}, {0, 0}},
		.fmt = PIX_FMT_MONOWHITE,
		.data = {
			.lineRectData = {
				.color = OSD_BLACK,
				.linewidth = 4,
			},
		},
	};

	IMPOSDGrpRgnAttr grAttr;
	memset(&grAttr, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttr.scalex = 1;
	grAttr.scaley = 1;

	int i;
	for (i = 0; i < MAX_IVS_OSD_REGION; i++) {
		ivsRgnHandler[i] = IMP_OSD_CreateRgn(&rAttr);
		if (ivsRgnHandler[i] == INVHANDLE) {
			IMP_LOG_ERR(TAG, "IVS IMP_OSD_CreateRgn %d failed\n", i);
		}
		if (IMP_OSD_RegisterRgn(ivsRgnHandler[i], grpNum, &grAttr) < 0) {
			IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn %d failed\n", ivsRgnHandler[i]);
		}
		usleep(10 * 1000);
	}

	IMP_OSD_Start(grpNum);

	return 0;

}

int osd_deinit(int grpNum)
{
	int i;
	for (i = 0; i < MAX_IVS_OSD_REGION; i++) {
		if (ivsRgnHandler[i] != INVHANDLE) {
			if (IMP_OSD_UnRegisterRgn(ivsRgnHandler[i], grpNum) < 0) {
				IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn %d failed\n", ivsRgnHandler[i]);
			}
			IMP_OSD_DestroyRgn(ivsRgnHandler[i]);
		}
		usleep(10 * 1000);
	}

	if (IMP_OSD_DestroyGroup(grpNum) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_DestroyGroup(%d) failed\n", grpNum);
		return -1;
	}

	return 0;
}
extern int IMP_OSD_SetPoolSize(int size);

static IMPSensorInfo sensor_info;
int sample_system_init(void *param)
{
	int ret = 0;

	imp_isp_attr_t *isp_param = (imp_isp_attr_t*)param;
	if (!isp_param) {
		printf("ERROR(%s): isp_param is NULL \n", TAG);
		return -1;
	}

	imp_ctx.bitrate = isp_param->encoder_info.bitrate;
	imp_ctx.gop = isp_param->encoder_info.gop;
	imp_ctx.qp_value = isp_param->encoder_info.qp_value;
	imp_ctx.rcmode = isp_param->encoder_info.rcmode;
	imp_ctx.sensor_width = isp_param->sensor_info.sensor_width;
	imp_ctx.sensor_height = isp_param->sensor_info.sensor_height;

	IMP_OSD_SetPoolSize(550*1024);

	memset(&sensor_info, 0, sizeof(IMPSensorInfo));
	memcpy(sensor_info.name, (char *)isp_param->sensor_info.sensor_name, sizeof(isp_param->sensor_info.sensor_name));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, (char *)isp_param->sensor_info.sensor_name, sizeof(isp_param->sensor_info.sensor_name));
	sensor_info.i2c.addr = isp_param->sensor_info.i2c_addr;

	IMP_LOG_DBG(TAG, "sample_system_init start\n");

	/*Step1: Open Sensor */
	ret = IMP_ISP_Open();
	if(ret){
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;

	}

	/*Step2: AddSernsor */
	ret = IMP_ISP_AddSensor(&sensor_info);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

	/*Step3: System Init */
	ret = IMP_System_Init();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_System_Init failed\n");
		return -1;
	}

	/*Step4: EnableSensor */
	ret = IMP_ISP_EnableSensor();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

	ret = IMP_ISP_EnableTuning();
	if(ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
		return -1;
	}

	if ((isp_param->hvflip > IMPISP_FLIP_NORMAL_MODE) && (isp_param->hvflip < IMPISP_FLIP_MODE_BUTT)) {
		IMPISPHVFLIP testhv = isp_param->hvflip;
		IMP_ISP_Tuning_SetHVFLIP(testhv);
		if (ret < 0){
			IMP_LOG_ERR(TAG, "failed to set HV Filp mode\n");
			return -1;
		}
		IMP_LOG_ERR(TAG, "Set HV filp mode:%d\n", isp_param->hvflip);

		usleep(100*1000);
	}

	IMP_LOG_DBG(TAG, "ImpSystemInit success\n");

	return 0;
}

int sample_system_exit()
{
	int ret = 0;

	IMP_LOG_DBG(TAG, "sample_system_exit start\n");

	IMP_System_Exit();

	ret = IMP_ISP_DisableTuning();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_ISP_DisableTuning failed\n");
		return -1;
	}

	ret = IMP_ISP_DisableSensor();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

	ret = IMP_ISP_DelSensor(&sensor_info);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}


	if(IMP_ISP_Close()){
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;
	}

	IMP_LOG_DBG(TAG, " sample_system_exit success\n");

	return 0;
}

#ifdef MODULE_TRACK_ENABLE
int algorithm_video_init(int chn_num, int ivs_mode)
{
	int ret = -1;

	ret = sample_framesource_init(chn_num);
	if (ret < 0) {
		printf("ERROR(%s): FrameSource stream%d init failed\n",TAG,  ret);
		return -1;
	}

	/* reset framesource*/
	IMPFSChnAttr fs_chn_attr;
	ret = IMP_FrameSource_GetChnAttr(chn_num, &fs_chn_attr);
	if(ret < 0) {
		printf("%s:IMP_FrameSource_GetChnAttr failed chn_num == %d\n", TAG, chn_num);
		return -1;
	}

	fs_chn_attr.pixFmt = PIX_FMT_NV12;

	fs_chn_attr.scaler.enable = 1;
	/* fs_chn_attr.scaler.enable = CROP_EN; */
	fs_chn_attr.scaler.outwidth = SENSOR_WIDTH_SECOND;
	fs_chn_attr.scaler.outheight = SENSOR_HEIGHT_SECOND;

	fs_chn_attr.crop.enable = 0;
	fs_chn_attr.picWidth = SENSOR_WIDTH_SECOND;
	fs_chn_attr.picHeight = SENSOR_HEIGHT_SECOND;

	ret = IMP_FrameSource_SetChnAttr(chn_num, &fs_chn_attr);
	if(ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr failed\n", TAG);
		return -1;
	}

	if (ivs_mode == IVS_MODE_BIND) {
		ret = IMP_IVS_CreateGroup(chn_num);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", chn_num);
			return -1;
		}


		IMPCell framesource_cell = {DEV_ID_FS, ALGORITHM_VIDEO_CH, 0};
		IMPCell ivs_cell = {DEV_ID_IVS, 1, 0};
		ret = IMP_System_Bind(&framesource_cell, &ivs_cell);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and ivs0 failed\n", ALGORITHM_VIDEO_CH);
			return -1;
		}

	}

	ret = sample_framesource_streamon(chn_num);
	if (ret < 0) {
		printf("ImpStreamOn failed\n");
		return -1;
	}

	/* set framedepth */
	ret = IMP_FrameSource_SetFrameDepth(chn_num, 1);
	if (ret < 0) {
		printf("IMP_FrameSource_SetFrameDepth failed\n");
		return -1;
	}
	printf("algorithm_video_init ok\n");

	return 0;
}

int algorithm_video_deinit(int chn_num, int ivs_mode)
{
	int ret = -1;

	/* Stream off*/
	ret = sample_framesource_streamoff(chn_num);
	if (ret < 0) {
		printf("FrameSource StreamOff failed\n");
		return -1;
	}

	if (ivs_mode == IVS_MODE_BIND) {
		IMPCell framesource_cell = {DEV_ID_FS, ALGORITHM_VIDEO_CH, 0};
		IMPCell ivs_cell = {DEV_ID_IVS, 1, 0};
		ret = IMP_System_UnBind(&framesource_cell, &ivs_cell);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and ivs0 failed\n", chn_num);
			return -1;
		}
		ret = IMP_IVS_DestroyGroup(chn_num);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup(%d) failed\n", chn_num);
			return -1;
		}
	}

	/* Framesource exit */
	ret = sample_framesource_exit(chn_num);
	if (ret < 0) {
		printf("FrameSource exit failed\n");
		return -1;
	}
	return 0;
}
#endif

int sample_framesource_streamon(int chn_num)
{
	int ret = 0, i = 0;
	i = chn_num;
	/* Enable channels */
	if (chn[i].enable) {
		ret = IMP_FrameSource_EnableChn(chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn[i].index);
			return -1;
		}
	}
	return 0;
}

int sample_framesource_streamoff(int chn_num)
{
	int ret = 0, i = 0;
	i = chn_num;
	/* Enable channels */
	if (chn[i].enable){
		ret = IMP_FrameSource_DisableChn(chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[i].index);
			return -1;
		}
	}
	return 0;
}

int sample_framesource_init(int chn_num)
{
	int i, ret;
	i = chn_num;

	if (chn[i].enable) {
		ret = IMP_FrameSource_CreateChn(chn[i].index, &chn[i].fs_chn_attr);
		if(ret < 0){
			IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", chn[i].index);
			return -1;
		}

		ret = IMP_FrameSource_SetChnAttr(chn[i].index, &chn[i].fs_chn_attr);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n",  chn[i].index);
			return -1;
		}
	}

	return 0;
}

int sample_framesource_exit(int chn_num)
{
	int ret,i;
	i = chn_num;

	if (chn[i].enable) {
		/*Destroy channel */
		ret = IMP_FrameSource_DestroyChn(chn[i].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn(%d) error: %d\n", chn[i].index, ret);
			return -1;
		}
	}
	return 0;
}


int sample_jpeg_init(int chn_num)
{
	int i, ret;
	int nrStreamSize = MJPEG_ENCODER_SIZE;

	IMPEncoderChnAttr channel_attr;
	IMPFSChnAttr *imp_chn_attr_tmp;

	i = chn_num;
	if (chn[i].enable) {
		imp_chn_attr_tmp = &chn[i].fs_chn_attr;
		memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
		ret = IMP_Encoder_SetDefaultParam(&channel_attr, IMP_ENC_PROFILE_JPEG, IMP_ENC_RC_MODE_FIXQP,
						  imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight,
						  imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen, 0, 0, imp_ctx.qp_value, 0);
			ret = IMP_Encoder_SetStreamBufSize(3 + chn[i].index, nrStreamSize);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_SetStreamBufSize(%d) error !\n", chn[i].index);
				return -1;
			}

			/* IMP_Encoder_SetbufshareChn(3 + chn[i].index, -1); */
			/* Create Channel */
			ret = IMP_Encoder_CreateChn(3 + chn[i].index, &channel_attr);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error: %d\n",
					    chn[i].index, ret);
				return -1;
			}

			/* Resigter Channel */
			ret = IMP_Encoder_RegisterChn(i, 3 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(0, %d) error: %d\n",
					    chn[i].index, ret);
				return -1;
			}

		}

	return 0;
}

int sample_encoder_init(int chn_num, int enc_type)
{
	int i, ret, chnNum = 0;
	IMPFSChnAttr *imp_chn_attr_tmp;
	IMPEncoderChnAttr channel_attr;

	i = chn_num;
	if (chn[i].enable) {
		imp_chn_attr_tmp = &chn[i].fs_chn_attr;
		chnNum = chn[i].index;

		chn[i].payloadType = enc_type;
#if 0
		if (1 == g_ucfg->h264_en)
			chn[i].payloadType = IMP_ENC_PROFILE_AVC_MAIN;
		else if (1 == g_ucfg->h265_en)
			chn[i].payloadType = IMP_ENC_PROFILE_HEVC_MAIN;
#endif

		memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
		ret = IMP_Encoder_SetDefaultParam(&channel_attr, chn[i].payloadType, imp_ctx.rcmode,
						  imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight,
						  imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen,
						  imp_chn_attr_tmp->outFrmRateNum * 2 / imp_chn_attr_tmp->outFrmRateDen, 1,
						  (imp_ctx.rcmode == IMP_ENC_RC_MODE_FIXQP) ? 35 : -1,
						  (uint64_t)imp_ctx.bitrate * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720));
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_SetDefaultParam(%d) error !\n", chnNum);
			return -1;
		}

		ret = IMP_Encoder_CreateChn(chnNum, &channel_attr);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error !\n", chnNum);
			return -1;
		}

		ret = IMP_Encoder_RegisterChn(chn[i].index, chnNum);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n", chn[i].index, chnNum, ret);
			return -1;
		}
	}

	return 0;
}

int sample_jpeg_exit(int chn_num)
{
    int ret = 0, i = 0, chnNum = 0;
    IMPEncoderChnStat chn_stat;

    i = chn_num;
    if (chn[i].enable) {
	    chnNum = 3 + chn[i].index;
            memset(&chn_stat, 0, sizeof(IMPEncoderChnStat));
            ret = IMP_Encoder_Query(chnNum, &chn_stat);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_Query(%d) error: %d\n", chnNum, ret);
                return -1;
            }

            if (chn_stat.registered) {
                ret = IMP_Encoder_UnRegisterChn(chnNum);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_Encoder_UnRegisterChn(%d) error: %d\n", chnNum, ret);
                    return -1;
                }

                ret = IMP_Encoder_DestroyChn(chnNum);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyChn(%d) error: %d\n", chnNum, ret);
                    return -1;
                }
            }
        }

    return 0;
}

int sample_encoder_exit(int chn_num)
{
    int ret = 0, i = 0, chnNum = 0;
    IMPEncoderChnStat chn_stat;

    i = chn_num;
    if (chn[i].enable) {
            if (chn[i].payloadType == IMP_ENC_PROFILE_JPEG) {
                chnNum = 3 + chn[i].index;
            } else {
                chnNum = chn[i].index;
            }
            memset(&chn_stat, 0, sizeof(IMPEncoderChnStat));
            ret = IMP_Encoder_Query(chnNum, &chn_stat);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_Query(%d) error: %d\n", chnNum, ret);
                return -1;
            }

            if (chn_stat.registered) {
                ret = IMP_Encoder_UnRegisterChn(chnNum);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_Encoder_UnRegisterChn(%d) error: %d\n", chnNum, ret);
                    return -1;
                }

                ret = IMP_Encoder_DestroyChn(chnNum);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "IMP_Encoder_DestroyChn(%d) error: %d\n", chnNum, ret);
                    return -1;
                }
            }
        }

    return 0;
}
//#################ivs-osd############################

extern int osd_width;
extern int osd_height;

int set_osd_ivs_switch(int on_off)
{
	int ret = -1;

	ret = IMP_OSD_ShowRgn(osd_hander[0], OSD_CONTROL_GROUP, on_off);
	if (ret != 0) {
		printf("ERROR(%s): IMP_OSD_ShowRgn() error\n",TAG);
		return -1;
	}

	return 0;
}

static int init_face_box_osd(int grp_num)
{
	int ret = -1;
	IMPOSDGrpRgnAttr grAttr;
	IMPOSDRgnAttr rAttr;

	if (IMP_OSD_CreateGroup(grp_num) < 0) {
		printf("ERROR(%s): IMP_OSD_CreateGroup(%d) error !\n", TAG, grp_num);
		return -1;
	}

	for(int i = 0; i < OSD_HANDLE_NUM; i++){

		osd_hander[i] = IMP_OSD_CreateRgn(NULL);
		if (osd_hander[i] == INVHANDLE) {
			printf("ERROR(%s): IMP_OSD_CreateRgn Logo error!\n", TAG);
			return -1;
		}

		ret = IMP_OSD_RegisterRgn(osd_hander[i], grp_num, NULL);
		if (ret < 0) {
			printf("ERROR(%s): IVS IMP_OSD_RegisterRgn failed\n", TAG);
			return -1;
		}
		
		memset(&rAttr, 0, sizeof(IMPOSDRgnAttr));
	
		rAttr.type = OSD_REG_COVER;
		rAttr.rect.p0.x = 0;
		rAttr.rect.p0.y = 0;
		rAttr.rect.p1.x = osd_width -1;
		rAttr.rect.p1.y = osd_height -1 ;
		rAttr.fmt = PIX_FMT_BGRA;//PIX_FMT_MONOWHITE
		rAttr.data.coverData.color = OSD_BLACK;
		ret = IMP_OSD_SetRgnAttr(osd_hander[i], &rAttr);
		if (ret < 0) {
			printf("ERROR(%s), IMP_OSD_SetRgnAttr Rect error !\n", TAG);
			return -1;
		}

		ret = IMP_OSD_GetGrpRgnAttr(osd_hander[i], grp_num, &grAttr);
		if (ret < 0) {
			printf("ERROR(%s): IMP_OSD_GetGrpRgnAttr Logo error!\n", TAG);
			return -1;
		}
		memset(&grAttr, 0, sizeof(IMPOSDGrpRgnAttr));
		grAttr.layer = 2;
		grAttr.show = 0;
		grAttr.gAlphaEn = 1;
		grAttr.fgAlhpa = 0xff;

		ret = IMP_OSD_SetGrpRgnAttr(osd_hander[i], grp_num, &grAttr);
		if (ret < 0) {
			printf("ERROR(%s): IMP_OSD_SetGrpRgnAttr Logo error!\n", TAG);
			return -1;
		}

		ret = IMP_OSD_ShowRgn(osd_hander[i], grp_num, 0);
		if (ret != 0) {
			printf("ERROR(%s): IMP_OSD_ShowRgn() Logo error\n",TAG);
			return -1;
		}
	}

	ret = IMP_OSD_Start(grp_num);
	if (ret < 0) {
		printf("ERROR(%s),IMP_OSD_Start Rect error !\n",TAG);
		return -1;
	}

	return 0;
}

static int osd_init_ivs(void)
{
	int ret = -1;

	ret = init_face_box_osd(0);
	if (ret < 0) {
		printf("ERROR(%s): init four corner osd failed. \n", TAG);
		return -1;
	}

	return 0;
}

static int osd_exit(void)
{
	int ret = -1;
	int i = 0;

	for (i = 0; i < OSD_HANDLE_NUM; i++) {
		/* close region */
		ret = IMP_OSD_ShowRgn(osd_hander[i], 0, 0);
		if (ret < 0) {
			printf("ERROR(%s): IMP_OSD_ShowRgn close error\n", TAG);
			return -1;
		}

		/* unregister region */
		ret = IMP_OSD_UnRegisterRgn(osd_hander[i], 0);
		if (ret < 0) {
			printf("ERROR(%s): IMP_OSD_UnRegisterRgn error\n", TAG);
			return -1;
		}

		/* destroy region */
		IMP_OSD_DestroyRgn(osd_hander[i]);
	}

	ret = IMP_OSD_DestroyGroup(0);
	if (ret < 0) {
		printf("ERROR(%s): IMP_OSD_DestroyGroup(%d) error\n",TAG, 0);
		return -1;
	}
 
	return 0;
}


int module_osd_control_init(void)
{
	int ret = -1;

	ret = osd_init_ivs();
	if (ret) {
		printf("ERROR(%s): osd_init error!\n", TAG);
		return -1;
	}
	printf("%s...OK\n",__func__);

	return 0;
}

int module_osd_control_deinit(void)
{
	int ret = -1;

	ret = osd_exit();
	if (ret < 0) {
		printf("ERROR:(%s) osd exit error! \n",TAG);
		return -1;
	}

	printf("%s...OK\n",__func__);
	return 0;
}


int IVS_OSD_Init(int grp_num)
{
	int ret = 0;

	ret = IMP_IVS_CreateGroup(grp_num);
	if (ret < 0) {
		printf("IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}
	return 0;
}


typedef struct {
	IMPFrameInfo    frameInfo;                         /**< 帧尺寸信息,只需要配置width和height */
	IMPFrameInfo    *result;
} IVS_OSD_BaseParam_t;

static int BaseLCDOSDInit(IMPIVSInterface *inf)
{
       /* inf->param = calloc(1, sizeof(IVS_OSD_BaseParam_t));*/
	/*printf("#$#$#$#$#$#$# %s:%d -> %p\n", __func__, __LINE__, inf->param);*/
	return 0;
}

static void BaseLCDOSDExit(IMPIVSInterface *inf)
{
       /* if(inf->param != NULL){*/
	
		/*free(inf->param);*/
		/*inf->param = NULL;*/
	/*}*/
	/*printf("#$#$#$#$#$#$# %s:%d -> %p\n", __func__, __LINE__, inf->param);*/
	return;
}

static int BaseLCDOSDPreprocessSync(IMPIVSInterface *inf, IMPFrameInfo *frame)
{
	return 0;
}


static int BaseLCDOSDProcessAsync(IMPIVSInterface *inf, IMPFrameInfo *frame)
{
	int ret = 0;
#if 0
	int fd = 0;
	printf("========= Frame info: addr:[%p], w: %d, h: %d\n", frame->virAddr, frame->width, frame->height);
	if (!wf) {
		fd = open("./lcdosd.rbg", O_RDWR | O_CREAT | O_APPEND, 0777);
		if(fd < 0) {
			IMP_LOG_ERR(TAG, "fd error !\n");
			return -1;
		}

		write(fd, (void *)frame->virAddr, frame->width * frame->height);

		close(fd);
		wf = 1;
	}
#endif
	((IVS_OSD_BaseParam_t *)(inf->param))->result = frame;
	return ret;
}

static int BaseLCDOSDGetResult(IMPIVSInterface *inf, void **result)
{
	*result = (void *)((IVS_OSD_BaseParam_t *)(inf->param))->result;
	return 0;
}

static int BaseLCDOSDReleaseResult(IMPIVSInterface *inf, void *result)
{

	IMPFrameInfo *frame = ((IVS_OSD_BaseParam_t *)(inf->param))->result;
	IMP_IVS_ReleaseData((void *)frame->virAddr);
	return 0;
}

static int BaseLCDOSDGetParam(IMPIVSInterface *inf, void *param)
{
	return 0;
}

static int BaseLCDOSDSetParam(IMPIVSInterface *inf, void *param)
{
	return 0;
}

static int BaseLCDOSDFlushFrame(IMPIVSInterface *inf)
{
	return 0;
}

IMPIVSInterface *baseMoveInterface = NULL;
IMPIVSInterface *IMP_IVS_CreateBaseIVSOSDInterface(IVS_OSD_BaseParam_t *param)
{
#if 1
	baseMoveInterface = calloc(1, sizeof(IMPIVSInterface) + sizeof(IVS_OSD_BaseParam_t));
	if (NULL == baseMoveInterface) {
		printf("calloc baseMoveInterface is NULL!\n");
		return NULL;
	}

	baseMoveInterface->param = (char*)baseMoveInterface + sizeof(IMPIVSInterface);
	memcpy(baseMoveInterface->param, param, sizeof(IVS_OSD_BaseParam_t));
#endif
	baseMoveInterface->paramSize = sizeof(IVS_OSD_BaseParam_t);
	baseMoveInterface->pixfmt = PIX_FMT_BGRA;
	baseMoveInterface->init = BaseLCDOSDInit;
	baseMoveInterface->exit = BaseLCDOSDExit;
	baseMoveInterface->preProcessSync = BaseLCDOSDPreprocessSync;

	baseMoveInterface->processAsync = BaseLCDOSDProcessAsync;
	baseMoveInterface->getResult    = BaseLCDOSDGetResult;
	baseMoveInterface->releaseResult= BaseLCDOSDReleaseResult;
	baseMoveInterface->getParam     = BaseLCDOSDGetParam;
	baseMoveInterface->setParam     = BaseLCDOSDSetParam;
	baseMoveInterface->flushFrame   = BaseLCDOSDFlushFrame;

	return baseMoveInterface;
}


int IVS_OSD_InterfaceInit(int grp_num, int chn_num, IMPIVSInterface **interface) {

	int ret = 0;
	IVS_OSD_BaseParam_t param;

	memset(&param, 0, sizeof(IVS_OSD_BaseParam_t));
#if 1
	param.frameInfo.width = osd_width;
	param.frameInfo.height = osd_height;
#endif
	printf("osd_width = %d  osd_height = %d \n", param.frameInfo.width, param.frameInfo.height);

	*interface = IMP_IVS_CreateBaseIVSOSDInterface(&param);
	if (*interface == NULL) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}

	ret = IMP_IVS_CreateChn(chn_num, *interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_RegisterChn(grp_num, chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
		return -1;
	}

	ret = IMP_IVS_StartRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StartRecvPic(%d) failed\n", chn_num);
		return -1;
	}

	return 0;
}


int IVS_OSD_Start(void) {

	IMPIVSInterface *interface;
	IVS_OSD_InterfaceInit(1, 1, &interface); 
	return 0;
}

int IVS_OSD_InterfaceDeinit(int grp_num, int chn_num)
{
	int ret;

	ret = IMP_IVS_StopRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StopRecvPic(%d) failed\n", chn_num);
		return -1;
	}
	sleep(1);

	ret = IMP_IVS_UnRegisterChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_UnRegisterChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_DestroyChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_DestroyChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_DestroyGroup(grp_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup(1) failed\n");
		return -1;
	}

	return 0;
}

int IVS_OSD_Stop(void) {

	IVS_OSD_InterfaceDeinit(1, 1); 
	if (baseMoveInterface != NULL) {
	
		free(baseMoveInterface);
		baseMoveInterface = NULL;
	}
	return 0;
}
//########################end ivs-osd ########################

//############################osd ############################

IMPRgnHandle *sample_osd_init(int grpNum)
{
	int ret = 0;
	int i = 0;
	IMPRgnHandle *prHander = NULL;

	prHander = malloc(OSD_HANDLE_NUM * sizeof(IMPRgnHandle));
	if (prHander <= 0) {
		IMP_LOG_ERR(TAG, "malloc() error !\n");
		return NULL;
	}

	for(i = 0; i < OSD_HANDLE_NUM;i++){
	prHander[i] = IMP_OSD_CreateRgn(NULL);
	if (prHander[i] == INVHANDLE) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn TimeStamp error %d !\n",i);
		return NULL;
	}
	//query osd rgn create status
	/*IMPOSDRgnCreateStat stStatus;
	memset(&stStatus,0x0,sizeof(IMPOSDRgnCreateStat));
	ret = IMP_OSD_RgnCreate_Query(prHander[i],&stStatus);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_OSD_RgnCreate_Query error !\n");
		return NULL;
	}*/


	ret = IMP_OSD_RegisterRgn(prHander[i], grpNum, NULL);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}

	//query osd rgn register status
	/*IMPOSDRgnRegisterStat stRigStatus;
	memset(&stRigStatus,0x0,sizeof(IMPOSDRgnRegisterStat));
	ret = IMP_OSD_RgnRegister_Query(prHander[i], grpNum,&stRigStatus);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_RgnRegister_Query failed\n");
		return NULL;
	}*/

	
	IMPOSDRgnAttr rAttrCover;
	memset(&rAttrCover, 0, sizeof(IMPOSDRgnAttr));
	rAttrCover.type = OSD_REG_COVER;
	rAttrCover.rect.p0.x = 0;
	rAttrCover.rect.p0.y = 0;
	rAttrCover.rect.p1.x = rAttrCover.rect.p0.x+40 -1;
	rAttrCover.rect.p1.y = rAttrCover.rect.p0.y+10 -1 ;
	rAttrCover.fmt = PIX_FMT_BGRA;
	rAttrCover.data.coverData.color = OSD_BLACK;
	ret = IMP_OSD_SetRgnAttr(prHander[i], &rAttrCover);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Cover error !\n");
		return NULL;
	}
	IMPOSDGrpRgnAttr grAttrCover;

	if (IMP_OSD_GetGrpRgnAttr(prHander[i], grpNum, &grAttrCover) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAtt:r Cover error !\n");
		return NULL;

	}
	memset(&grAttrCover, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrCover.gAlphaEn = 1;
	grAttrCover.fgAlhpa = 0xff;
	grAttrCover.layer = 2;
	
	/* Disable Cover global alpha, it is absolutely no transparent. */
	if (IMP_OSD_SetGrpRgnAttr(prHander[i], grpNum, &grAttrCover) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Cover error !\n");
		return NULL;
	}



	ret = IMP_OSD_Start(grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_Start TimeStamp, Logo, Cover and Rect error !\n");
		return NULL;
	}
}
	return prHander;
}

int osd_show(IMPRgnHandle *prHander,int grpNum)
{
	int ret;

	ret = IMP_OSD_ShowRgn(prHander[0], grpNum, 1);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn() timeStamp error\n");
		return -1;
	}
	
	return 0;
}

int sample_osd_exit(IMPRgnHandle *prHander,int grpNum)
 	{
		int ret;
		int i ;
		for(i = 0; i < OSD_HANDLE_NUM ;i++)
		{
			//反区显示
			ret = IMP_OSD_ShowRgn(prHander[i], grpNum, 0);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn close timeStamp error\n");
			}

			//反注册
			ret = IMP_OSD_UnRegisterRgn(prHander[i], grpNum);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn timeStamp error\n");
			}
			//摧毁区
			IMP_OSD_DestroyRgn(prHander[i]);
		}
		//摧毁组
		ret = IMP_OSD_DestroyGroup(grpNum);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_OSD_DestroyGroup(0) error\n");
			return -1;
		}
		free(prHander);
		prHander = NULL;

		return 0;
	}
	
//############################osd################################


static int get_stream(char *buf, IMPEncoderStream *stream)
{
	int i, len = 0;
	int nr_pack = stream->packCount;

	//printf("pack count:%d\n", nr_pack);
	for (i = 0; i < nr_pack; i++) {

		IMPEncoderPack *pack = &stream->pack[i];
		if(pack->length){
			uint32_t remSize = stream->streamSize - pack->offset;
			if(remSize < pack->length){
				memcpy(buf + len, (void *)(stream->virAddr + pack->offset), remSize);
				memcpy(buf + remSize + len, (void *)stream->virAddr, pack->length - remSize);
			}else {
				memcpy((void *)(buf + len), (void *)(stream->virAddr + pack->offset), pack->length);
			}
			len += pack->length;
		}
	}
	return len;
}

#ifdef SHOW_FRM_BITRATE
int frame_bitrate_show(IMPEncoderStream *stream)
{
	int chnNum = 0;
		int i, len = 0;
		for (i = 0; i < stream->packCount; i++) {
			len += stream->pack[i].length;
		}
		bitrate_sp[chnNum] += len;
		frmrate_sp[chnNum]++;

		int64_t now = IMP_System_GetTimeStamp() / 1000;
		if(((int)(now - statime_sp[chnNum]) / 1000) >= FRM_BIT_RATE_TIME){
			double fps = (double)frmrate_sp[chnNum] / ((double)(now - statime_sp[chnNum]) / 1000);
			double kbr = (double)bitrate_sp[chnNum] * 8 / (double)(now - statime_sp[chnNum]);

			printf("streamNum[%d]:FPS: %0.2f,Bitrate: %0.2f(kbps)\n", chnNum, fps, kbr);
			//fflush(stdout);

			frmrate_sp[chnNum] = 0;
			bitrate_sp[chnNum] = 0;
			statime_sp[chnNum] = now;
		}
		return 0;
}
#endif
//##############################黑图#########################
int sample_get_black(char *buf)
{
	FILE *fp;
	int size = 0;
	//二进制方式打开文件
	fp = fopen("/system/config/osd.config","rb");
	if(NULL == fp)
	{
		printf("Error:Open input.c file fail!\n");
		return -1;
	}
 
	//求得文件的大小
	/*fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	memset(buf,0,size);
	printf("#####size %d ######\n",size);
	rewind(fp);*/
	
	//求得文件的大小变动
	//fseek(fp, 0, SEEK_END);
	size = osd_width * osd_height * 2;
	memset(buf,0,size);
	//printf("#####size %d ######\n",size);
	//rewind(fp);
	//读文件
	fread(buf,1,size,fp);//每次读一个，共读size次
 
	fclose(fp);
	
	return size;
}
//##############################end 黑图#########################
//拿取黑图
// int sample_set_black(char *buf,int len)
// {
	// FILE *fp;
 
	// //二进制方式打开文件
	// fp = fopen("/system/config/snap-1.bin","wb");
	// if(NULL == fp)
	// {
		// printf("Error:Open input.c file fail!\n");
		// return -1;
	// }

	// //读文件
	// fwrite(buf,1,len,fp);//每次读一个，共读size次
 
	// fclose(fp);
	// return 0;
// }

int sample_get_h264_snap(int chn_num, char *img_buf)
{
	int chnNum, ret, len;
	chnNum = chn[chn_num].index;

	ret = IMP_Encoder_PollingStream(chnNum, 1000);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_PollingStream(%d) timeout\n", chnNum);
		return -1;
	}

	IMPEncoderStream stream;
	/* Get H264 or H265 Stream */
	ret = IMP_Encoder_GetStream(chnNum, &stream, 1);

	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream(%d) failed\n", chnNum);
		return -1;
	}

	len = get_stream(img_buf, &stream);
#ifdef SHOW_FRM_BITRATE
	frame_bitrate_show(&stream);
#endif

	IMP_Encoder_ReleaseStream(chnNum, &stream);

	return len;

}

int sample_get_jpeg_snap(int chn_num, char *img_buf)
{
	int i, ret, len = 0;

	i = chn_num;
	if (chn[i].enable) {
		/* Polling JPEG Snap, set timeout as 1000msec */
		ret = IMP_Encoder_PollingStream(3 + chn[i].index, 10000);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "Polling stream timeout\n");
			return 0;
		}

			IMPEncoderStream stream;
			/* Get JPEG Snap */
			ret = IMP_Encoder_GetStream(chn[i].index + 3, &stream, 1);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream() failed\n");
				return -1;
			}
			
			len = get_stream(img_buf, &stream);
			
			//printf("########## len %d \n ",len);
			
			IMP_Encoder_ReleaseStream(3 + chn[i].index, &stream);
	 
#ifdef SHOW_FRM_BITRATE
			frame_bitrate_show(&stream);
#endif
	}
	return len;
}

/*convert NV12 to YUYV422*/
void nv12toyuy2(char * image_in, char* image_out, int width, int height)
{
    int i, j;
    char *uv = image_in + width * height;
     for (j = 0; j < height;j++)
        {
            for (i = 0; i < width / 2;i++)
            {
                image_out[0] = image_in[2 * i];//y
                image_out[1] = uv[2 * i];//u
                image_out[2] = image_in[2 * i + 1];//y
                image_out[3] = uv[2 * i + 1];//v
                image_out += 4;
            }
            image_in += width;
            if (j & 1)
            {
                uv += width;
            }
        }
    return;
}
//#################yuy2 黑图开关###############
static int yuyv_switch = 0;
void set_yuyv_switch(int on_off)
{
	yuyv_switch = on_off;
}
//############################################
int cap_cnt = 0; //nv12->yuy2 图像保存/tmp/
int sample_get_yuv_snap(int chn_num, char *img_buf)
{
	int ret , len = 0;
	IMPFrameInfo *frame_bak;
	IMPFSChnAttr *imp_chn_attr_tmp;
	
	imp_chn_attr_tmp = &chn[chn_num].fs_chn_attr;
	if(yuyv_switch == 0){
	#if 0    //ivs-osd enable
		ret = IMP_IVS_PollingResult(1, -1);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult failed\n");
			return -1;
		}
		ret = IMP_IVS_GetResult(1, (void *)&frame_bak);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_GetResult failed\n");
			return -1;
		}
	#else 
		ret = IMP_FrameSource_GetFrame(chn_num, &frame_bak);
			if (ret < 0) {
			IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_GetFrame failed\n", __func__, __LINE__);
			return -1;
			}
	#endif
		len = imp_chn_attr_tmp->picWidth *imp_chn_attr_tmp->picHeight *2;
		
		if(cap_cnt == 1) {

			int fd0 = open("/tmp/osd.nv12", O_RDWR | O_CREAT, 0777);
			if(fd0 < 0) {
				IMP_LOG_ERR(TAG, "fd error !\n");
				return -1;
			}

			write(fd0, (void *)frame_bak->virAddr, frame_bak->width * frame_bak->height * 3 / 2);

			close(fd0);
		}
		nv12toyuy2((char *)frame_bak->virAddr, img_buf, imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight);
		if(cap_cnt == 1) {
			int fd1 = open("/tmp/osd.yuy2", O_RDWR | O_CREAT, 0777);
			if(fd1 < 0) {
				IMP_LOG_ERR(TAG, "fd1 error !\n");
				return -1;
			}

			write(fd1, img_buf, frame_bak->width * frame_bak->height * 2);

			close(fd1);
		}
		cap_cnt = 0;
		//len = frame_bak->size;
		//memcpy(img_buf, frame_bak->virAddr, len);
	#if 0 //ivs-osd enable
		ret = IMP_IVS_ReleaseResult(1, (void *)&frame_bak);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 1);
			return -1;
		}
	#else
		IMP_FrameSource_ReleaseFrame(chn_num, frame_bak);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_ReleaseFrame failed\n", __func__, __LINE__);
			return -1;
		}
	}else{
			len = sample_get_black(img_buf);
	}
#endif
	return len;
}

int sample_uvc_framesource_init(int ch,int fps, int format, int width, int height)
{
	int ret = -1;
	IMPFSChnAttr *imp_chn_attr_tmp;
	imp_chn_attr_tmp = &chn[ch].fs_chn_attr;

	/* set base info */
	imp_chn_attr_tmp->outFrmRateNum = fps;
	imp_chn_attr_tmp->picWidth = width;
	imp_chn_attr_tmp->picHeight = height;
	imp_chn_attr_tmp->nrVBs = 2;

	/* PS : resolution must be standard 16:9 or 4:3 */
	/* T21/T31 : scaler, then crop */
	if (imp_ctx.sensor_width == imp_ctx.sensor_height * 4 / 3) {
		/* sensor setting - width : height = 4 : 3 */
		/*zoom need to open the scaler*/
		imp_chn_attr_tmp->scaler.enable    = 1;
		imp_chn_attr_tmp->scaler.outwidth  = width;
		imp_chn_attr_tmp->scaler.outheight = width * 3 / 4;

		/* set crop attr */
		if (width == (height * 16 / 9)) {
			imp_chn_attr_tmp->crop.enable = 1;
			imp_chn_attr_tmp->crop.left   = 0;
			imp_chn_attr_tmp->crop.top    = height / 6;
			imp_chn_attr_tmp->crop.width  = width;
			imp_chn_attr_tmp->crop.height = height;
		} else {
			imp_chn_attr_tmp->crop.enable = 0;
		}
	} else {
		/* sensor setting - width : height = 16 : 9 */
		/*zoom need to open the scaler*/
		imp_chn_attr_tmp->scaler.enable    = 1;
		imp_chn_attr_tmp->scaler.outwidth  = height * 16 / 9;
		imp_chn_attr_tmp->scaler.outheight = height;

		/* set crop attr */
		if (width == (height * 4 / 3)) {
			imp_chn_attr_tmp->crop.enable = 1;
			imp_chn_attr_tmp->crop.left   = width / 6;
			imp_chn_attr_tmp->crop.top    = 0;
			imp_chn_attr_tmp->crop.width  = width;
			imp_chn_attr_tmp->crop.height = height;
		} else {
			imp_chn_attr_tmp->crop.enable = 0;
		}
	}

	/* correct crop,scaler,top,left value to match soc platform */
	/* T31: width(scaler) 2 */
	if (imp_chn_attr_tmp->scaler.enable) {
		if (imp_chn_attr_tmp->scaler.outwidth % 2)
			imp_chn_attr_tmp->scaler.outwidth--;
	}

	if (format == V4L2_PIX_FMT_YUYV || format == V4L2_PIX_FMT_NV12) {
		imp_chn_attr_tmp->pixFmt = PIX_FMT_NV12;//PIX_FMT_YUYV422
	}

	/* Step.2 FrameSource init */
	ret = sample_framesource_init(ch);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

	return 0;
}

//#define USE_DMIC
void sample_audio_dmic_init(void)
{

	int ret = -1;

	/* Step 1: set dmic user info:if need aec function*/
	ret = IMP_DMIC_SetUserInfo(0, 1, 0);  /*不需要AEC功能*/
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "dmic set user info error.\n");
		return;
	}

	/* step 2: set dmic audio attr*/
	IMPDmicAttr attr;
	attr.samplerate = DMIC_SAMPLE_RATE_16000;
	attr.bitwidth = DMIC_BIT_WIDTH_16;
	attr.soundmode = DMIC_SOUND_MODE_MONO;
	attr.chnCnt = 4;
	attr.frmNum = 2;
	attr.numPerFrm = 640;

	ret = IMP_DMIC_SetPubAttr(0, &attr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "DMIC_SetPubAttr failed.\n");
		return;
	}

	/*step 3: enable DMIC device*/
	ret = IMP_DMIC_Enable(0);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "DMIC Enable failed.\n");
		return;
	}

	/*step 4: set dmic channel attr*/
	IMPDmicChnParam chnParam;
	chnParam.usrFrmDepth = 2;
	ret = IMP_DMIC_SetChnParam(0, 0, &chnParam);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "DMIC SetChnParam failed.\n");
		return;
	}

	/*step 5: enable dmic channel*/
	ret = IMP_DMIC_EnableChn(0, 0);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "DMIC Enable Channel failed.\n");
		return;
	}

	/*step 6: set dmic volume*/
	ret = IMP_DMIC_SetVol(0, 0, 60);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "DMIC Set vol failed.\n");
		return;
	}

	/*step 7: set dmic gain*/
	ret = IMP_DMIC_SetGain(0, 0, 22);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "DMIC Set Gain failed.\n");
		return;
	}
	//sleep(2);
	//sample_audio_dmic_test();
	return;

}

void sample_audio_amic_init(int vol, int audio_ns)
{
    int ret = -1;

    /* Step 1: set public attribute of AI device. */
    int devID = 1;
    IMPAudioIOAttr attr;
    attr.samplerate = AUDIO_SAMPLE_RATE_16000;// AUDIO_SAMPLE_RATE_16000;
    attr.bitwidth = AUDIO_BIT_WIDTH_16;
    attr.soundmode = AUDIO_SOUND_MODE_MONO;
    attr.frmNum = 2;
    attr.numPerFrm = 640;
    attr.chnCnt = 1;
    ret = IMP_AI_SetPubAttr(devID, &attr);
    if(ret != 0) {
        IMP_LOG_ERR(TAG, "set ai %d attr err: %d\n", devID, ret);
        return;
    }

    memset(&attr, 0x0, sizeof(attr));
    ret = IMP_AI_GetPubAttr(devID, &attr);
    if(ret != 0) {
        IMP_LOG_ERR(TAG, "get ai %d attr err: %d\n", devID, ret);
        return;
    }

#if 0
    IMP_LOG_INFO(TAG, "Audio In GetPubAttr samplerate : %d\n", attr.samplerate);
    IMP_LOG_INFO(TAG, "Audio In GetPubAttr   bitwidth : %d\n", attr.bitwidth);
    IMP_LOG_INFO(TAG, "Audio In GetPubAttr  soundmode : %d\n", attr.soundmode);
    IMP_LOG_INFO(TAG, "Audio In GetPubAttr     frmNum : %d\n", attr.frmNum);
    IMP_LOG_INFO(TAG, "Audio In GetPubAttr  numPerFrm : %d\n", attr.numPerFrm);
    IMP_LOG_INFO(TAG, "Audio In GetPubAttr     chnCnt : %d\n", attr.chnCnt);
#endif

    /* Step 2: enable AI device. */
    ret = IMP_AI_Enable(devID);
    if(ret != 0) {
        IMP_LOG_ERR(TAG, "enable ai %d err\n", devID);
        return;
    }


    /* Step 3: set audio channel attribute of AI device. */
    int chnID = 0;
    IMPAudioIChnParam chnParam;
    chnParam.usrFrmDepth = 2;
    ret = IMP_AI_SetChnParam(devID, chnID, &chnParam);
    if(ret != 0) {
        IMP_LOG_ERR(TAG, "set ai %d channel %d attr err: %d\n", devID, chnID, ret);
        return;
    }

    memset(&chnParam, 0x0, sizeof(chnParam));
    ret = IMP_AI_GetChnParam(devID, chnID, &chnParam);
    if(ret != 0) {
        IMP_LOG_ERR(TAG, "get ai %d channel %d attr err: %d\n", devID, chnID, ret);
        return;
    }

    IMP_LOG_INFO(TAG, "Audio In GetChnParam usrFrmDepth : %d\n", chnParam.usrFrmDepth);

    /* Step 4: enable AI channel. */
    ret = IMP_AI_EnableChn(devID, chnID);
    if(ret != 0) {
        IMP_LOG_ERR(TAG, "Audio Record enable channel failed\n");
        return;
    }

/*    ret = IMP_AI_EnableAec(devID, chnID, 0, 0); */
    /*if(ret != 0) {*/
	    /*IMP_LOG_ERR(TAG, "Audio Record enable AEC failed\n");*/
	    /*return;*/
    /*}*/

    /* Step 5: Set audio channel volume. */
    int chnVol = vol;
    chnVol = 1.2 * vol - 30;
    ret = IMP_AI_SetVol(devID, chnID, chnVol);
    if(ret != 0) {
        IMP_LOG_ERR(TAG, "Audio Record set volume failed\n");
        return;
    }

    ret = IMP_AI_GetVol(devID, chnID, &chnVol);
    if(ret != 0) {
        IMP_LOG_ERR(TAG, "Audio Record get volume failed\n");
        return;
    }
    IMP_LOG_INFO(TAG, "Audio In GetVol    vol : %d\n", chnVol);

    int aigain = 31;
    ret = IMP_AI_SetGain(devID, chnID, aigain);
    if(ret != 0) {
        IMP_LOG_ERR(TAG, "Audio Record Set Gain failed\n");
        return;
    }

    ret = IMP_AI_GetGain(devID, chnID, &aigain);
    if(ret != 0) {
        IMP_LOG_ERR(TAG, "Audio Record Get Gain failed\n");
        return;
    }
    IMP_LOG_INFO(TAG, "Audio In GetGain    gain : %d\n", aigain);

    if (audio_ns >= 0) {
	    if (audio_ns < NS_LOW || audio_ns > NS_VERYHIGH) {
		    IMP_LOG_ERR(TAG, "Audio set unvilid NS Leave. \n");
		    return ;
	    }
	    ret = IMP_AI_EnableNs(&attr, audio_ns);
	    if(ret != 0) {
		    printf("enable audio ns error.\n");
		    IMP_LOG_INFO(TAG, "enable audio ns error.\n");
		    return ;
		    printf("########### audio_ns #############");
	    }
	    

    }

    return;

}

void sample_audio_dmic_exit()
{
	int ret = -1;

	ret = IMP_DMIC_DisableChn(0, 0);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "DMIC DisableChn error.\n");
		return ;
	}

	ret = IMP_DMIC_Disable(0);
	if (ret != 0){
		IMP_LOG_ERR(TAG, "DMIC Disable error.\n");
		return ;
	}

	return;
}

void sample_audio_amic_exit()
{
	int ret = -1;

	/* Step 1: set public attribute of AI device. */
	int devID = 1;
	int chnID = 0;
	/* Step 9: disable the audio channel. */
	ret = IMP_AI_DisableChn(devID, chnID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio channel disable error\n");
		return;
	}

	/* Step 10: disable the audio devices. */
	ret = IMP_AI_Disable(devID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio device disable error\n");
		return;
	}
	return;
}

static void *_ao_test_play_thread(void *argv)
{
	/*int size = 0;*/
	int ret = -1;
	int vol = *((int *)argv);

	/* Step 1: set public attribute of AO device. */
	int devID = 0;
	IMPAudioIOAttr attr;

	prctl(PR_SET_NAME, "audio_speak");

	attr.samplerate = AUDIO_SAMPLE_RATE_48000;
	attr.bitwidth = AUDIO_BIT_WIDTH_16;
	attr.soundmode = AUDIO_SOUND_MODE_MONO;
	attr.frmNum = 4;
	attr.numPerFrm = 960;
	attr.chnCnt = 1;
	ret = IMP_AO_SetPubAttr(devID, &attr);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "set ao %d attr err: %d\n", devID, ret);
		return NULL;
	}

	memset(&attr, 0x0, sizeof(attr));
	ret = IMP_AO_GetPubAttr(devID, &attr);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "get ao %d attr err: %d\n", devID, ret);
		return NULL;
	}

	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr samplerate:%d\n", attr.samplerate);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr   bitwidth:%d\n", attr.bitwidth);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr  soundmode:%d\n", attr.soundmode);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr     frmNum:%d\n", attr.frmNum);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr  numPerFrm:%d\n", attr.numPerFrm);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr     chnCnt:%d\n", attr.chnCnt);

	/* Step 2: enable AO device. */
	ret = IMP_AO_Enable(devID);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "enable ao %d err\n", devID);
		return NULL;
	}

	/* Step 3: enable AI channel. */
	int chnID = 0;
	ret = IMP_AO_EnableChn(devID, chnID);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio play enable channel failed\n");
		return NULL;
	}

	/* Step 4: Set audio channel volume. */
	int chnVol = vol;
	chnVol = vol - 30;
	ret = IMP_AO_SetVol(devID, chnID, chnVol);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Play set volume failed\n");
		return NULL;
	}

	ret = IMP_AO_GetVol(devID, chnID, &chnVol);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Play get volume failed\n");
		return NULL;
	}
	IMP_LOG_INFO(TAG, "Audio Out GetVol    vol:%d\n", chnVol);

	int aogain = 28;
	ret = IMP_AO_SetGain(devID, chnID, aogain);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record Set Gain failed\n");
		return NULL;
	}

	ret = IMP_AO_GetGain(devID, chnID, &aogain);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record Get Gain failed\n");
		return NULL;
	}
	IMP_LOG_INFO(TAG, "Audio Out GetGain    gain : %d\n", aogain);

	struct Ucamera_Audio_Frame u_frame;
	/*struct timeval time_cur;*/
	/*uint32_t time_recv;*/

	while (1) {
		ret = Ucamera_Audio_Get_Frame(&u_frame);
		if (ret || u_frame.len <= 0)
		{
			usleep(10000);
			continue;
		}

		/* Step 5: send frame data. */
		IMPAudioFrame frm;
		frm.virAddr = (uint32_t *)u_frame.data;
		frm.len = u_frame.len;
		ret = IMP_AO_SendFrame(devID, chnID, &frm, BLOCK);
		if (ret != 0) {
			IMP_LOG_ERR(TAG, "send Frame Data error\n");
			usleep(10000);
			continue;
		}

		Ucamera_Audio_Release_Frame(&u_frame);

	}
	ret = IMP_AO_FlushChnBuf(devID, chnID);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_AO_FlushChnBuf error\n");
		return NULL;
	}
	/* Step 6: disable the audio channel. */
	ret = IMP_AO_DisableChn(devID, chnID);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio channel disable error\n");
		return NULL;
	}

	/* Step 7: disable the audio devices. */
	ret = IMP_AO_Disable(devID);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio device disable error\n");
		return NULL;
	}

	pthread_exit(0);
}

int sample_audio_play_start(void *param)
{
	int ret = -1;

	pthread_t play_thread_id;


	ret = pthread_create(&play_thread_id, NULL, _ao_test_play_thread, param);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "[ERROR] %s: pthread_create Audio Record failed\n", __func__);
		return -1;
	}
	return ret;
}


/*
	将png -> argb 
*/
/*int getFileSizeSystemCall(const char *strFileName) */
/*{*/
	/*struct stat temp;*/
	/*stat(strFileName, &temp);*/
	/*return temp.st_size;*/
/*}*/

void Stretch_pic(int srcWidth, int srcHeight, int picWidth, int picHeight, int *need_Width, int *need_Height)
{
	// float tmp_Width=0.0,tmp_Height=0.0;
	*need_Width  = (int)((float)picWidth / (float)srcWidth * 200.0);//这个是确定在显示器中的显示大小
	*need_Height = (int)((float)picHeight / (float)srcHeight * 200.0);
	if(*need_Width % 2)
	{
		*need_Width = *need_Width + 1;
	}

	if(*need_Height % 2)
	{
		*need_Height = *need_Height + 1;
	}

	if(*need_Width >= *need_Height)
	{
		*need_Height = *need_Width;
	}else
	{
		*need_Width = *need_Height;
	}
	//printf("[INFO] need_Width -> %d need_Height -> %d \n",*need_Width,*need_Height);
}

int load_png_image( const char *filepath)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep* row_pointers;
	char buf[PNG_BYTES_TO_CHECK];
	int w, h, x = 0, y, temp, color_type;
	int  fd = 0;//size = 0;
	unsigned char * buf_f = (unsigned char *)malloc(4);
	
	//printf("[INFO] png_argb %s \n",filepath);
	fp = fopen( filepath, "rb" );
	if( fp == NULL ) { 
		printf("open filepath error\n");
		return -1;
	}
	/*
	   读取png 文件大小                                             
	   */
	fd = open("/tmp/ARGB", O_RDWR|O_CREAT|O_TRUNC, 0777);
	//printf("[INFO] argb_file %s \n","/tmp/ARGB");
	//size = getFileSizeSystemCall(filepath);
	//printf("png file write size :%d\n", size);

	png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );
	info_ptr = png_create_info_struct( png_ptr );

	setjmp( png_jmpbuf(png_ptr) ); 

	temp = fread( buf, 1, PNG_BYTES_TO_CHECK, fp );

	if( temp < PNG_BYTES_TO_CHECK ) {
		fclose(fp);
		png_destroy_read_struct( &png_ptr, &info_ptr, 0);                    
		return -1;
	}

	temp = png_sig_cmp( (png_bytep)buf, (png_size_t)0, PNG_BYTES_TO_CHECK );     

	//printf("7\n");
	if( temp != 0 ) {
		fclose(fp);
		png_destroy_read_struct( &png_ptr, &info_ptr, 0);
		return -1;
	}

	rewind( fp );
	png_init_io( png_ptr, fp ); 
	png_read_png( png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0 );
	color_type = png_get_color_type( png_ptr, info_ptr );
	w = png_get_image_width( png_ptr, info_ptr );
	h = png_get_image_height( png_ptr, info_ptr );
	row_pointers = png_get_rows( png_ptr, info_ptr );
	switch( color_type ) {
		case PNG_COLOR_TYPE_RGB_ALPHA:
			for( y=0; y<h; ++y ) {
				for( x=0; x<w*4; x++) {
					buf_f[1] = /**(bufv + y*w + x-1) =*/ row_pointers[y][x++]; // red
					buf_f[2] = /**(bufv + y*w + x-1) =*/ row_pointers[y][x++]; // green
					buf_f[3] = /**(bufv + y*w + x-1) =*/ row_pointers[y][x++]; // blue
					buf_f[0] = /**(bufv + y*w + x-1) =*/ row_pointers[y][x]; // alpha
					/*
					   将转换的argb 数据写到
					   ARGB 文件里
					   */
					if(write(fd, buf_f, 4) != 4)
						printf("write        error\n");
				}
			}
			//printf("[argb file] y = %d, x = %d\n", y, (x/4));                
			close(fd);
			break;
		case PNG_COLOR_TYPE_RGB:
			for( y=0; y<h; ++y ) {
				for( x=0; x<w*3; ) {
				}
			}
			break;
		default:
			fclose(fp);
			png_destroy_read_struct( &png_ptr, &info_ptr, 0);
			return 0;
	}
	png_destroy_read_struct( &png_ptr, &info_ptr, 0);
	fclose(fp);
	return 0;                                                                    
}
/*
	将图像缩放成对应比例
	str_dir：需要处理的 png图像
	strout : 处理后生成的图像 （/tmp/tmp.png）
*/
int resize_png(char const *str_dir,int nDestWidth, int nDestHeight)
{
	char const *str = NULL;
	char const *strout = "/tmp/tmp.png";

	int iw=0, ih=0, n=0;
	int ow=0, oh=0;
	unsigned char *odata = NULL;
	unsigned char *idata = NULL;

	str = (char const *)str_dir;

	/*
		加载png 图像,
		获取图像的信息：
		图像的宽：iw
		图像的高：ih
		图像的颜色通道：n
	*/
	idata = stbi_load(str, &iw, &ih, &n, 0);
	//printf("iw = %d ih = %d n = %d \n", iw,ih,n);

	/*
		输出的图像宽高
	*/
	ow = nDestWidth;
	oh = nDestHeight;
	//printf("ow = %d oh = %d \n", ow,oh);
	odata = malloc(ow * oh * n);
	/*
		处理png 图像（进行缩放）
	*/
	stbir_resize(idata, iw, ih, 0, odata, ow, oh, 0,
			STBIR_TYPE_UINT8, n, 0, 0,
			STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
			STBIR_FILTER_BOX, STBIR_FILTER_BOX,
			STBIR_COLORSPACE_SRGB, NULL
		    );

	/*
		将缩放后的 png 图像放入 strout中
	*/
	stbi_write_png(strout, ow, oh, n, odata, 0);

	/*
		释放产生的buff
	*/
	stbi_image_free(idata);
	stbi_image_free(odata);
	//printf("[INFO] str %s \n",str);
	//printf("[INFO] strout %s \n",strout);
	//printf("Resize success!!! \n");
	return 0;
}
