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


#include "imp-common.h"

#define TAG "Imp-Common"

int g_Audio = 0;
int g_Audio_Ns = -1;
int g_Fps_Num = SENSOR_FRAME_RATE_NUM_25;
int g_VideoWidth = 1280;
int g_VideoHeight = 720;
int g_i2c_addr = 0x1a;
int g_wdr = 0;
int g_RcMode = IMP_ENC_RC_MODE_CBR;
int g_BitRate = 1000;
int g_gop = SENSOR_FRAME_RATE_NUM_25;
int g_adb = 0;
int g_rndis = 0;
int g_Speak = 0;
int g_dmic = 0;
int g_led = 0;
int g_touch = 0;
int g_touch_old = 0;
int g_QP = 80;
int g_Volume = 100;
int g_Dynamic_Fps = 0;
int g_Power_save = 1;
char g_Sensor_Name[16] = "imx307";

typedef struct _pal_buf_info {
	int stop;
	int pal_size;
	int max_size;
	uint64_t ts;
	pthread_t pid;
	pthread_mutex_t mutex;
	sem_t sem_stop;
	void *buf;

} pal_buf_info_t;

static pal_buf_info_t g_pal_buf;
static int g_vol = 0;

/*#define SHOW_FRM_BITRATE*/                                                                                                                                                                  
#ifdef SHOW_FRM_BITRATE 
#define FRM_BIT_RATE_TIME 2
#define STREAM_TYPE_NUM 3
static int frmrate_sp[STREAM_TYPE_NUM] = { 0 };
static int statime_sp[STREAM_TYPE_NUM] = { 0 };
static int bitrate_sp[STREAM_TYPE_NUM] = { 0 };
#endif

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
	}
};

extern int IMP_OSD_SetPoolSize(int size);
//extern int IMP_Encode_SetPoolSize(int size);
IMPSensorInfo sensor_info;
IMPSensorInfo sensor_info_del;
IMPSensorInfo sensor_info_add;

int sample_reload_sensor_init(char *sensor_name_del, char *sensor_name_add)
{
	int ret;	

	memset(&sensor_info_del, 0, sizeof(IMPSensorInfo));
	memcpy(sensor_info_del.name, sensor_name_del, 7);
	sensor_info_del.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info_del.i2c.type, sensor_name_del, 7);
	sensor_info_del.i2c.addr = g_i2c_addr;
	printf("============>>><<<<=============== %d, %s\n", __LINE__, __func__);
	ret = IMP_ISP_DelSensor(&sensor_info_del);
	if(ret < 0){
		printf("============>>><<<<=============== %d, %s\n", __LINE__, __func__);
		IMP_LOG_ERR(TAG, "failed to DelSensor\n");
		return -1;
	}
	printf("============>>><<<<=============== %d, %s\n", __LINE__, __func__);
	memset(&sensor_info_add, 0, sizeof(IMPSensorInfo));
	memcpy(sensor_info_add.name,  sensor_name_add, 7);
	sensor_info_add.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info_add.i2c.type, sensor_name_add, 7);
	sensor_info_add.i2c.addr = g_i2c_addr;
	printf("============>>><<<<=============== %d, %s\n", __LINE__, __func__);
	ret = IMP_ISP_AddSensor(&sensor_info_add);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}
	printf("============>>><<<<=============== %d, %s\n", __LINE__, __func__);
	return 0;
}


int sample_system_init()
{
	int ret = 0;

	memset(&sensor_info, 0, sizeof(IMPSensorInfo));
	memcpy(sensor_info.name, g_Sensor_Name, sizeof(g_Sensor_Name));
	sensor_info.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
	memcpy(sensor_info.i2c.type, g_Sensor_Name, sizeof(g_Sensor_Name));
	sensor_info.i2c.addr = g_i2c_addr;

	IMP_LOG_DBG(TAG, "sample_system_init start\n");

	IMP_OSD_SetPoolSize(1*1024);
//	IMP_Encode_SetPoolSize(1*1024);
	ret = IMP_ISP_Open();
	if(ret){
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;
	}

	if (g_wdr) {
		IMP_LOG_DBG(TAG, "WDR mode enable\n");
		IMPISPTuningOpsMode mode;
		mode = IMPISP_TUNING_OPS_MODE_ENABLE;
		IMP_ISP_WDR_ENABLE(mode);
	}

	ret = IMP_ISP_AddSensor(&sensor_info);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}


	ret = IMP_System_Init();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_System_Init failed\n");
		return -1;
	}

	if (!g_Power_save) {
		ret = IMP_ISP_EnableSensor();
		if(ret < 0){
			IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
			return -1;
		}

		ret = IMP_ISP_EnableTuning();
		if(ret < 0){
			IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
			return -1;
		}
	}
	IMP_LOG_DBG(TAG, "ImpSystemInit success\n");

	return 0;
}

int sample_system_exit()
{
	int ret = 0;

	IMP_LOG_DBG(TAG, "sample_system_exit start\n");


	IMP_System_Exit();

	if (!g_Power_save) {
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
	}
	ret = IMP_ISP_DelSensor(&sensor_info);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to DelSensor\n");
		return -1;
	}


	if(IMP_ISP_Close()){
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return -1;
	}

	IMP_LOG_DBG(TAG, " sample_system_exit success\n");

	return 0;
}

int sample_framesource_streamon()
{
	int ret = 0, i = 0;
	/* Enable channels */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_FrameSource_EnableChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn[i].index);
				return -1;
			}
		}
	}
	return 0;
}

int sample_framesource_streamoff()
{
	int ret = 0, i = 0;
	/* Enable channels */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable){
			ret = IMP_FrameSource_DisableChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[i].index);
				return -1;
			}
		}
	}
	return 0;
}

int sample_framesource_init()
{
	int i, ret;

	for (i = 0; i < FS_CHN_NUM; i++) {
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
	}

	return 0;
}

int sample_framesource_exit()
{
	int ret,i;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			/*Destroy channel */
			ret = IMP_FrameSource_DestroyChn(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn(%d) error: %d\n", chn[i].index, ret);
				return -1;
			}
		}
	}
	return 0;
}

int sample_jpeg_init()
{
	int i, ret;
	IMPEncoderChnAttr channel_attr;
	IMPFSChnAttr *imp_chn_attr_tmp;

	for (i = 0; i <  FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			imp_chn_attr_tmp = &chn[i].fs_chn_attr;
            memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
            ret = IMP_Encoder_SetDefaultParam(&channel_attr, IMP_ENC_PROFILE_JPEG, IMP_ENC_RC_MODE_FIXQP,
                    imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight,
                    imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen, 0, 0, g_QP, 0);

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
	}

	return 0;
}

int sample_encoder_init()
{
	int i, ret, chnNum = 0;
	IMPFSChnAttr *imp_chn_attr_tmp;
	IMPEncoderChnAttr channel_attr;

    for (i = 0; i <  FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            imp_chn_attr_tmp = &chn[i].fs_chn_attr;
            chnNum = chn[i].index;

            memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
            ret = IMP_Encoder_SetDefaultParam(&channel_attr, chn[i].payloadType, g_RcMode,
                    imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight,
                    imp_chn_attr_tmp->outFrmRateNum, imp_chn_attr_tmp->outFrmRateDen,
                    imp_chn_attr_tmp->outFrmRateNum * 2 / imp_chn_attr_tmp->outFrmRateDen, 1,
                    (g_RcMode == IMP_ENC_RC_MODE_FIXQP) ? 35 : -1,
                    (uint64_t)g_BitRate * (imp_chn_attr_tmp->picWidth * imp_chn_attr_tmp->picHeight) / (1280 * 720));
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
    }

	return 0;
}

int sample_jpeg_exit(void)
{
    int ret = 0, i = 0, chnNum = 0;
    IMPEncoderChnStat chn_stat;

	for (i = 0; i <  FS_CHN_NUM; i++) {
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
    }

    return 0;
}

int sample_encoder_exit(void)
{
    int ret = 0, i = 0, chnNum = 0;
    IMPEncoderChnStat chn_stat;

	for (i = 0; i <  FS_CHN_NUM; i++) {
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
    }

    return 0;
}

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

int sample_get_h264_snap(char *img_buf)
{
#if 1
	int chnNum, ret, len;
	chnNum = chn[0].index;

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

	IMP_Encoder_ReleaseStream(chnNum, &stream);
#else
	int len = 0;
	if (g_pal_buf.pal_size) {
		struct timeval obtain_time;
		uint64_t now_ts;
		uint64_t offset_ts;
		gettimeofday(&obtain_time, NULL);
		now_ts = (obtain_time.tv_sec * (uint64_t)1000000 + (uint64_t)obtain_time.tv_usec);
		offset_ts = now_ts - g_pal_buf.ts;
		if (offset_ts * g_Fps_Num > 1000000 ) {
			pthread_mutex_lock(&g_pal_buf.mutex);
			len = g_pal_buf.pal_size ;
			memcpy(img_buf, g_pal_buf.buf, len);
			pthread_mutex_unlock(&g_pal_buf.mutex);
			g_pal_buf.ts = now_ts;
		} else {
			uint64_t wait_time;
			wait_time = (1.0 * 1000000 / g_Fps_Num) - offset_ts;
			usleep(wait_time);
			
			pthread_mutex_lock(&g_pal_buf.mutex);
			len = g_pal_buf.pal_size ;
			memcpy(img_buf, g_pal_buf.buf, len);
			pthread_mutex_unlock(&g_pal_buf.mutex);
			
			gettimeofday(&obtain_time, NULL);
			now_ts = (obtain_time.tv_sec * (uint64_t)1000000 + (uint64_t)obtain_time.tv_usec);
			g_pal_buf.ts = now_ts;
		} 
	}
#endif

	return len;

}

int sample_get_jpeg_snap(char *img_buf)
{
#if 0
	int i, ret, len = 0;

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_Encoder_StartRecvPic(3 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", 3 + chn[i].index);
				return -1;
			}

			/* Polling JPEG Snap, set timeout as 1000msec */
			ret = IMP_Encoder_PollingStream(3 + chn[i].index, 10000);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Polling stream timeout\n");
				continue;
			}

			IMPEncoderStream stream;
			/* Get JPEG Snap */
			ret = IMP_Encoder_GetStream(chn[i].index + 3, &stream, 1);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream() failed\n");
				return -1;
			}

			len = get_stream(img_buf, &stream);

			IMP_Encoder_ReleaseStream(3 + chn[i].index, &stream);

			ret = IMP_Encoder_StopRecvPic(3 + chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic() failed\n");
				return -1;
			}
		}
	}
	return len;
#endif
	int len = 0;
	struct timeval obtain_time;
	uint64_t now_ts;
	if (g_pal_buf.pal_size) {
		uint64_t offset_ts;
		gettimeofday(&obtain_time, NULL);
		now_ts = (obtain_time.tv_sec * (uint64_t)1000000 + (uint64_t)obtain_time.tv_usec);
		offset_ts = now_ts - g_pal_buf.ts;
		if (offset_ts * g_Fps_Num > 1000000 ) {
			pthread_mutex_lock(&g_pal_buf.mutex);
			len = g_pal_buf.pal_size ;
			memcpy(img_buf, g_pal_buf.buf, len);
			pthread_mutex_unlock(&g_pal_buf.mutex);
			g_pal_buf.ts = now_ts;
		} else {
			uint64_t wait_time;
			wait_time = (1.0 * 1000000 / g_Fps_Num) - offset_ts;
			usleep(wait_time);
			
			gettimeofday(&obtain_time, NULL);
			now_ts = (obtain_time.tv_sec * (uint64_t)1000000 + (uint64_t)obtain_time.tv_usec);
			pthread_mutex_lock(&g_pal_buf.mutex);
			len = g_pal_buf.pal_size ;
			memcpy(img_buf, g_pal_buf.buf, len);
			pthread_mutex_unlock(&g_pal_buf.mutex);
			g_pal_buf.ts = now_ts;
		} 
	} else {
			while(!g_pal_buf.pal_size){
				usleep(20 * 10000);
			}
			gettimeofday(&obtain_time, NULL);
			/*printf("##########%d,%s#########\n",__LINE__, __func__);*/
			now_ts = (obtain_time.tv_sec * (uint64_t)1000000 + (uint64_t)obtain_time.tv_usec);
			pthread_mutex_lock(&g_pal_buf.mutex);
			len = g_pal_buf.pal_size ;
			/*printf("##########%d,%s#########\n",__LINE__, __func__);*/
			memcpy(img_buf, g_pal_buf.buf, len);
			pthread_mutex_unlock(&g_pal_buf.mutex);
			g_pal_buf.ts = now_ts;

		}


	return len;
}
int sample_get_nv12_snap(char *img_buf)
{
    int len = 0;
	/*YUV > 1280*70 skip */
	if (g_Fps_Num < 25) {
		int ret;
		IMPFrameInfo *frame_bak;
		IMPFSChnAttr *imp_chn_attr_tmp;

		imp_chn_attr_tmp = &chn[0].fs_chn_attr;
		ret = IMP_FrameSource_GetFrame(0, &frame_bak);
    	if (ret < 0) {
		IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_GetFrame failed\n", __func__, __LINE__);
		return -1;
    	}
		// len = imp_chn_attr_tmp->picWidth *imp_chn_attr_tmp->picHeight * 2;
		len = frame_bak->size;
		memcpy(img_buf, (void*)(frame_bak->virAddr), len);
		IMP_FrameSource_ReleaseFrame(0, frame_bak);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_ReleaseFrame failed\n", __func__, __LINE__);
			return -1;
		}

	} else {
		struct timeval obtain_time;
		uint64_t now_ts;
		/*30 fps and 25fps */
		if (g_pal_buf.pal_size) {
			uint64_t offset_ts;
			gettimeofday(&obtain_time, NULL);
			now_ts = (obtain_time.tv_sec * (uint64_t)1000000 + (uint64_t)obtain_time.tv_usec);
			offset_ts = now_ts - g_pal_buf.ts;
			if (offset_ts * g_Fps_Num > 1000000 ) {
				pthread_mutex_lock(&g_pal_buf.mutex);
				len = g_pal_buf.pal_size ;
				memmove(img_buf, g_pal_buf.buf, len);
				pthread_mutex_unlock(&g_pal_buf.mutex);
				g_pal_buf.ts = now_ts;
			} else {
				uint64_t wait_time;
				wait_time = (1.0 * 1000000 / g_Fps_Num) - offset_ts;
				usleep(wait_time);

				gettimeofday(&obtain_time, NULL);
				now_ts = (obtain_time.tv_sec * (uint64_t)1000000 + (uint64_t)obtain_time.tv_usec);
				g_pal_buf.ts = now_ts;

				pthread_mutex_lock(&g_pal_buf.mutex);
				len = g_pal_buf.pal_size ;
				memcpy(img_buf, g_pal_buf.buf, len);
				pthread_mutex_unlock(&g_pal_buf.mutex);

			}
		} else {
			while(!g_pal_buf.pal_size){
				usleep(20 * 10000);
			}
			gettimeofday(&obtain_time, NULL);
			printf("##########%d,%s#########\n",__LINE__, __func__);
			now_ts = (obtain_time.tv_sec * (uint64_t)1000000 + (uint64_t)obtain_time.tv_usec);
			pthread_mutex_lock(&g_pal_buf.mutex);
			len = g_pal_buf.pal_size ;
			printf("##########%d,%s#########\n",__LINE__, __func__);
			memmove(img_buf, g_pal_buf.buf, len);
			pthread_mutex_unlock(&g_pal_buf.mutex);
			g_pal_buf.ts = now_ts;

		}

	}

	return len;
}


void sample_prepare_pal_buf()
{
	int ret = -1;
	memset(&g_pal_buf, 0, sizeof(pal_buf_info_t));
	g_pal_buf.buf = NULL;
	g_pal_buf.max_size = g_VideoWidth * g_VideoHeight * 2;
	g_pal_buf.pal_size = 0;

	g_pal_buf.buf = (char*)malloc(g_pal_buf.max_size);
	if (!g_pal_buf.buf) {
		printf("%s(%d):malloc data error failed\n", __func__, __LINE__);
		return;
	}
	memset(g_pal_buf.buf, 0, g_pal_buf.max_size);
}

void sample_deprepare_pal_buf()
{
	if (g_pal_buf.buf) {
		free(g_pal_buf.buf);
		g_pal_buf.buf = NULL;
	}
}
#if 0
#include "node_list.h"
static int h264_start_flag = 0;
extern nl_context_t frame_list;

static void *h264_stream_thread(void *m)
{
	int chnNum, ret, len, i;
	chnNum = chn[0].index;

		ret = IMP_Encoder_StartRecvPic(chn[0].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", chn[0].index);
			return NULL;
		}
	while (1) {
		if (0 == h264_start_flag) {
			printf("stop h264 stream!\n");
			break;
		}
		ret = IMP_Encoder_PollingStream(chnNum, 1000);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_PollingStream(%d) timeout\n", chnNum);
			return NULL;
		}

		IMPEncoderStream stream;
		/* Get H264 or H265 Stream */
		ret = IMP_Encoder_GetStream(chnNum, &stream, 1);

		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream(%d) failed\n", chnNum);
			return NULL;
		}

		//len = get_stream(img_buf, &stream);
		int nr_pack = stream.packCount;
		len = 0;
		node_t *node = Get_Free_Node(frame_list);
		for (i = 0; i < nr_pack; i++) {

			IMPEncoderPack *pack = &stream.pack[i];
			if(pack->length){
				uint32_t remSize = stream.streamSize - pack->offset;
				if(remSize < pack->length){
					memcpy(node->data + len, (void *)(stream.virAddr + pack->offset), remSize);
					memcpy(node->data + remSize + len, (void *)stream.virAddr, pack->length - remSize);
				}else {
					memcpy((void *)(node->data + len), (void *)(stream.virAddr + pack->offset), pack->length);
				}
				len += pack->length;
			}
		}
		node->size = len;
		Put_Use_Node(frame_list, node);

#ifdef SHOW_FRM_BITRATE
	    int i, len = 0;
	    for (i = 0; i < stream.packCount; i++) {
	      len += stream.pack[i].length;
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
#endif

		IMP_Encoder_ReleaseStream(chnNum, &stream);

	}
		ret = IMP_Encoder_StopRecvPic(chnNum);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic(%d) failed\n", chnNum);
			return NULL;
		}

	return NULL;
}

int sample_get_h264_start(void)
{
	pthread_t tid; /* Stream capture in another thread */
	int ret;

	h264_start_flag = 1;
	ret = pthread_create(&tid, NULL, h264_stream_thread, NULL);
	if (ret) {
		IMP_LOG_ERR(TAG, "h264 stream create error\n");
		return -1;
	}
}

int sample_get_h264_stop(void)
{
	h264_start_flag = 0;
	return 0;
}
#endif

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

int sample_get_yuv_snap(char *img_buf)
{

	int len = 0;
	/*YUV > 1280*70 skip */
	if (g_Fps_Num < 25) {
		int ret ;
		IMPFrameInfo *frame_bak;
		IMPFSChnAttr *imp_chn_attr_tmp;

		imp_chn_attr_tmp = &chn[0].fs_chn_attr;
		ret = IMP_FrameSource_GetFrame(0, &frame_bak);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_GetFrame failed\n", __func__, __LINE__);
			return -1;
		}
		len = imp_chn_attr_tmp->picWidth *imp_chn_attr_tmp->picHeight *2;
		nv12toyuy2((char *)frame_bak->virAddr, img_buf, imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight);
		//len = frame_bak->size;
		//memcpy(img_buf, frame_bak->virAddr, len);
		IMP_FrameSource_ReleaseFrame(0, frame_bak);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_ReleaseFrame failed\n", __func__, __LINE__);
			return -1;
		}

	} else {
		struct timeval obtain_time;
		uint64_t now_ts;
		/*30 fps and 25fps */
		if (g_pal_buf.pal_size) {
			uint64_t offset_ts;
			gettimeofday(&obtain_time, NULL);
			now_ts = (obtain_time.tv_sec * (uint64_t)1000000 + (uint64_t)obtain_time.tv_usec);
			offset_ts = now_ts - g_pal_buf.ts;
			if (offset_ts * g_Fps_Num > 1000000 ) {
				pthread_mutex_lock(&g_pal_buf.mutex);
				len = g_pal_buf.pal_size ;
				memmove(img_buf, g_pal_buf.buf, len);
				pthread_mutex_unlock(&g_pal_buf.mutex);
				g_pal_buf.ts = now_ts;
			} else {
				uint64_t wait_time;
				wait_time = (1.0 * 1000000 / g_Fps_Num) - offset_ts;
				usleep(wait_time);

				gettimeofday(&obtain_time, NULL);
				now_ts = (obtain_time.tv_sec * (uint64_t)1000000 + (uint64_t)obtain_time.tv_usec);
				g_pal_buf.ts = now_ts;

				pthread_mutex_lock(&g_pal_buf.mutex);
				len = g_pal_buf.pal_size ;
				memcpy(img_buf, g_pal_buf.buf, len);
				pthread_mutex_unlock(&g_pal_buf.mutex);

			}
		} else {
			while(!g_pal_buf.pal_size){
				usleep(20 * 10000);
			}
			gettimeofday(&obtain_time, NULL);
			now_ts = (obtain_time.tv_sec * (uint64_t)1000000 + (uint64_t)obtain_time.tv_usec);
			pthread_mutex_lock(&g_pal_buf.mutex);
			len = g_pal_buf.pal_size ;
			memmove(img_buf, g_pal_buf.buf, len);
			pthread_mutex_unlock(&g_pal_buf.mutex);
			g_pal_buf.ts = now_ts;

		}

	}

	return len;
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

void sample_audio_amic_init(void)
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
	int chnVol;
    ret = IMP_AI_SetPubAttr(devID, &attr);
    if(ret != 0) 
	{
        IMP_LOG_ERR(TAG, "set ai %d attr err: %d\n", devID, ret);
        return;
    }

    memset(&attr, 0x0, sizeof(attr));
    ret = IMP_AI_GetPubAttr(devID, &attr);
    if(ret != 0) 
	{
        IMP_LOG_ERR(TAG, "get ai %d attr err: %d\n", devID, ret);
        return;
    }
    /* Step 2: enable AI device. */
    ret = IMP_AI_Enable(devID);
    if(ret != 0) 
	{
        IMP_LOG_ERR(TAG, "enable ai %d err\n", devID);
        return;
    }
    /* Step 3: set audio channel attribute of AI device. */
    int chnID = 0;
    IMPAudioIChnParam chnParam;
    chnParam.usrFrmDepth = 2;
    ret = IMP_AI_SetChnParam(devID, chnID, &chnParam);
    if(ret != 0) 
	{
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

    /* Step 5: Set audio channel volume. */
    chnVol = 1.2 * g_Volume - 30;
    if(78<chnVol)
	{
		chnVol = 78; // 解决最大音量超限--210324 hhj
	}
	ret = IMP_AI_SetVol(devID, chnID, chnVol);
    if(ret != 0) 
	{
        IMP_LOG_ERR(TAG, "Audio Record set volume failed\n");
        return;
    }

    ret = IMP_AI_GetVol(devID, chnID, &chnVol);
    if(ret != 0) 
	{
        IMP_LOG_ERR(TAG, "Audio Record get volume failed\n");
        return;
    }
    IMP_LOG_INFO(TAG, "Audio In GetVol    vol : %d\n", chnVol);

    int aigain = 31;
    ret = IMP_AI_SetGain(devID, chnID, aigain);
    if(ret != 0) 
	{
        IMP_LOG_ERR(TAG, "Audio Record Set Gain failed\n");
        return;
    }

    if (g_Audio_Ns >= 0) 
	{
	    if (g_Audio_Ns < NS_LOW || g_Audio_Ns > NS_VERYHIGH) 
		{
		    IMP_LOG_ERR(TAG, "Audio set unvilid NS Leave. \n");
		    return ;
	    }
	    ret = IMP_AI_EnableNs(&attr, g_Audio_Ns);
	    if(ret != 0) 
		{
		    printf("enable audio ns error.\n");
		    IMP_LOG_INFO(TAG, "enable audio ns error.\n");
		    return ;
	    }

    }
    return;
}

int sample_audio_dmic_pcm_get(short *pcm)
{

	int ret, k;
	short *pdata = NULL;
	int len = 640;

	IMPDmicChnFrame g_chnFrm;

	ret = IMP_DMIC_PollingFrame(0, 0, 1000);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "dmic polling frame data error.\n");
	}
	ret = IMP_DMIC_GetFrame(0, 0, &g_chnFrm, BLOCK);
	if(ret < 0) {
		printf("IMP_DMIC_GetFrame failed\n");
		return 0;
	}

	pdata = (short*)(g_chnFrm.rawFrame.virAddr);
	// memcpy((void *)pcm, g_chnFrm.virAddr, g_chnFrm.len);

	for(k = 0; k < len; k++) {
		pcm[k] = pdata[k*4];
	}
	ret = IMP_DMIC_ReleaseFrame(0, 0, &g_chnFrm) ;
	if (ret < 0) {
		printf("IMP_DMIC_ReleaseFrame failed.\n");
		return 0;
	}
	return len*2;
	// return g_chnFrm.len;

}

int sample_audio_amic_pcm_get(short *pcm)
{

	/* Step 6: get audio record frame. */
	int ret = 0;
	int devID = 1;
	int chnID = 0;
	//int i = 0;

	ret = IMP_AI_PollingFrame(devID, chnID, 1000);
	if (ret != 0 ) {
		IMP_LOG_ERR(TAG, "Audio Polling Frame Data error\n");
	}
	IMPAudioFrame frm;
	ret = IMP_AI_GetFrame(devID, chnID, &frm, BLOCK);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Get Frame Data error\n");
		return 0;
	}

	/* Step 7: Save the recording data to a file. */
	memcpy((void *)pcm, frm.virAddr, frm.len);

	//for (i = 0; i < frm.len/2; ++i)
	//{
	//	*(pcm+2*i+ 1) = *(pcm+2*i);
	//}

	//struct timeval start;
	//gettimeofday(&start,NULL);
	//unsigned long long start_1 = 1000000 * start.tv_sec  +  start.tv_usec;
	//printf("%s: timestamp = %llu star_1 = %llu.\n",__func__, frm.timeStamp, start_1);
	/* Step 8: release the audio record frame. */
	ret = IMP_AI_ReleaseFrame(devID, chnID, &frm);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio release frame data error\n");
		return 0;
	}
	return frm.len;

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

const int16_t spk_vol_table[10] = {-4721, -3230, -2155, -1313, -622, -35, 540, 1216, 2034, 3072};

static int aduio_spk_volume_vert(int db)
{
	int vol = 0;
	int16_t tmp = (int16_t)db;

	if (tmp >= spk_vol_table[9])
		vol = 100;
	else if (tmp >= spk_vol_table[8])
		vol = 90;
	else if (tmp >= spk_vol_table[7])
		vol = 80;
	else if (tmp >= spk_vol_table[6])
		vol = 70;
	else if (tmp >= spk_vol_table[5])
		vol = 60;
	else if (tmp >= spk_vol_table[4])
		vol = 50;
	else if (tmp >= spk_vol_table[3])
		vol = 40;
	else if (tmp >= spk_vol_table[2])
		vol = 30;
	else if (tmp >= spk_vol_table[1])
		vol = 20;
	else if (tmp >= spk_vol_table[0])
		vol = 10;
	else
		vol = 0;
	return vol - 30;
}

const int16_t mic_vol_table[7] = {0xfa00, 0xfc01, 0xfe00, 0x0, 0x0200, 0x0400, 0x0600};

static int aduio_mic_volume_vert(int db)
{
	int vol = 0;
	int16_t tmp = (int16_t)db;

	if (tmp >= mic_vol_table[6])
		vol = 100;
	else if (tmp >= mic_vol_table[5])
		vol = 85;
	else if (tmp >= mic_vol_table[4])
		vol = 70;
	else if (tmp >= mic_vol_table[3])
		vol = 50;
	else if (tmp >= mic_vol_table[2])
		vol = 35;
	else if (tmp >= mic_vol_table[1])
		vol = 10;
	else if (tmp >= mic_vol_table[0])
		vol = 0;
	return vol - 30;
}

int sample_set_mic_volume(int db)
{
	int ret, vol;
	vol = 1.2 * db - 30;
	g_vol = vol;
	if (78 < vol)
	{
		vol = 78; // 解决最大音量超限--210324 hhj
	}
	if (g_dmic)
	{
		ret = IMP_DMIC_SetVol(0, 0, vol);
	}
	else
	{
		ret = IMP_AI_SetVol(1, 0, vol);
	}
	if(ret != 0) 
	{
		IMP_LOG_ERR(TAG, "Audio Record set volume failed\n");
		return -1;
	}
	return 0;
}

int sample_set_mic_mute(int mute)
{
	int ret;
	printf("set mic volume mute:%d\n", mute);

	if (g_dmic) {
		if (mute == 1)
			ret = IMP_DMIC_SetVol(0, 0, -30);
		else
			ret = IMP_DMIC_SetVol(0, 0, g_vol);
	} else
		ret = IMP_AI_SetVolMute(1, 0, mute);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record set volume failed\n");
		return -1;
	}

	return 0;
}

int sample_set_spk_volume(int db)
{
	int ret, vol;
	vol = aduio_spk_volume_vert(db);
	printf("set spk volume db:0x%x percent:%d\n", db, vol);

	ret = IMP_AO_SetVol(0, 0, vol);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Play set volume failed\n");
		return -1;
	}

	return 0;
}

int sample_set_spk_mute(int mute)
{
	printf("set spk volume mute:%d\n", mute);

	int ret = IMP_AO_SetVolMute(0, 0, mute);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Play set volume failed\n");
		return -1;
	}

	return 0;
}

#include "ucam/usbcamera.h"

#define AO_TEST_SAMPLE_RATE 48000
#define AO_TEST_SAMPLE_TIME 40
#define AO_TEST_BUF_SIZE (AO_TEST_SAMPLE_RATE * sizeof(short) * AO_TEST_SAMPLE_TIME / 1000)
#define AO_BASIC_TEST_PLAY_FILE  "./ao_paly.pcm"

static void *_ao_test_play_thread(void *argv)
{
	int ret = -1;


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
	int chnVol = 40;
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

int sample_audio_play_start(void)
{
	int ret = -1;

	pthread_t play_thread_id;


	ret = pthread_create(&play_thread_id, NULL, _ao_test_play_thread, NULL);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "[ERROR] %s: pthread_create Audio Record failed\n", __func__);
		return -1;
	}
	pthread_join(play_thread_id, NULL);
	return ret;
}

int sample_ucamera_gpio_init(int gpio, int direction)
{
	char direction_path[64] = {0};
	char value_path[64] = {0};
	FILE *p = NULL;

	/* Robot ARM Leds.	*/
	p = fopen("/sys/class/gpio/export","w");
	if (!p)
		return -1;
	fprintf(p,"%d",gpio);
	fclose(p);

	sprintf(direction_path, "/sys/class/gpio/gpio%d/direction", gpio);
	sprintf(value_path, "/sys/class/gpio/gpio%d/value", gpio);

	p = fopen(direction_path, "w");
	if (!p)
		return -1;
	if(0 == direction)
	{
		fprintf(p, "out");
	}
	else
	{
		fprintf(p, "in");
	}
	fclose(p);

	p = fopen(value_path, "w");
	if (!p)
	{
		return -1;
	}
	if(gpio == g_touch || gpio == g_touch_old)
	{
		fprintf(p, "%d", 0);
		printf("===>>gpio == g_touch<<====\n");
	}
	else 
	{
		fprintf(p, "%d", 1);
	}
	fclose(p);
	return 0;
}

int sample_ucamera_gpio_ctl(int gpio, int value)
{
	char value_path[64] = {0};
	FILE *p = NULL;

	sprintf(value_path, "/sys/class/gpio/gpio%d/value", gpio);

	p = fopen(value_path, "w");
	if (!p)
		return -1;
	fprintf(p,"%d", value);
	fclose(p);
	return 0;
}

int sample_ucamera_gpio_read(int gpio)
{
	char value_path[64] = {0};
	FILE *p = NULL;
	int value;

	sprintf(value_path, "/sys/class/gpio/gpio%d/value", gpio);

	p = fopen(value_path, "r");
	if (!p)
		return 0;
	fscanf(p,"%d", &value);
	printf("======>>sample_ucamera_gpio_read %d = %d <<=====\n", gpio, value);
	fclose(p);
	return value;
}

static void *uvc_stream_process(void *arg)
{
	int fmt = *((int *)arg);
	int encoder = 0;
	int chnNum = 0, ret = -1, len = 0;
	if (fmt == V4L2_PIX_FMT_H264) {
		chnNum = chn[0].index;
		encoder = 1;
	} else if (fmt == V4L2_PIX_FMT_MJPEG) {
		chnNum = chn[0].index + 3;
		encoder = 1;
	} else {
		encoder = 0;
	}

	while(!g_pal_buf.stop) {

		if (!g_pal_buf.buf) {
			usleep(100 * 1000);
			continue;
		}

		if (encoder == 1) {
			if (fmt == V4L2_PIX_FMT_MJPEG) {
				ret = IMP_Encoder_PollingStream(chnNum, 10000);
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

				pthread_mutex_lock(&g_pal_buf.mutex);
				len = get_stream(g_pal_buf.buf, &stream);
				g_pal_buf.pal_size = len;
				pthread_mutex_unlock(&g_pal_buf.mutex);

				IMP_Encoder_ReleaseStream(chnNum, &stream);
			} else {
				/*h264 skip*/
				/*usleep(1000000/g_Fps_Num);*/
				g_pal_buf.stop = 1;
			}
		} else if(encoder == 0){

			if (g_Fps_Num < 25) {
				/*yuv 1280x720, skip*/
				/*usleep(1000000/g_Fps_Num);*/
				g_pal_buf.stop = 1;
			} else {
				IMPFrameInfo *frame_bak = NULL;
				IMPFSChnAttr *imp_chn_attr_tmp = NULL;

				imp_chn_attr_tmp = &chn[0].fs_chn_attr;
				ret = IMP_FrameSource_GetFrame(0, &frame_bak);
				if (ret < 0) {
					printf("%s(%d):IMP_FrameSource_GetFrame failed\n", __func__, __LINE__);
					continue;
				}
				if(fmt == V4L2_PIX_FMT_YUYV)
				{
					len = imp_chn_attr_tmp->picWidth *imp_chn_attr_tmp->picHeight *2;
					/* nv12tog_pal_bufyuy2((char *)frame_bak->virAddr, img_buf, imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight); */
					if (g_pal_buf.buf && frame_bak) {
						pthread_mutex_lock(&g_pal_buf.mutex);
						nv12toyuy2((char *)frame_bak->virAddr, (char *)g_pal_buf.buf, imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight);
						g_pal_buf.pal_size = len;
						pthread_mutex_unlock(&g_pal_buf.mutex);
					}
				}
				else if(fmt == V4L2_PIX_FMT_NV12)
				{
					len = imp_chn_attr_tmp->picWidth *imp_chn_attr_tmp->picHeight *1.5;
					// nv12tog_pal_bufyuy2((char *)frame_bak->virAddr, img_buf, imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight);
					if (g_pal_buf.buf && frame_bak) {
						pthread_mutex_lock(&g_pal_buf.mutex);
						// nv12toyuy2((char *)frame_bak->virAddr, (char *)g_pal_buf.buf, imp_chn_attr_tmp->picWidth, imp_chn_attr_tmp->picHeight);
						memcpy(g_pal_buf.buf, frame_bak->virAddr, len);
						g_pal_buf.pal_size = len;
						pthread_mutex_unlock(&g_pal_buf.mutex);
					}
				}
				//len = frame_bak->size;
				//memcpy(img_buf, frame_bak->virAddr, len);
				IMP_FrameSource_ReleaseFrame(0, frame_bak);
				if (ret < 0) {
					printf("%s(%d):IMP_FrameSource_ReleaseFrame failed\n", __func__, __LINE__);
					continue;
				}

			} 
		} else {
			printf("##########encoder error %d#########\n",encoder);
		}

	}
	sem_post(&g_pal_buf.sem_stop);
	return NULL;
}

int sample_get_stream_process_init(int fmt)
{
	int ret = -1;
	pthread_t pid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
	g_pal_buf.pid = pid;
	g_pal_buf.stop = 0;
	g_pal_buf.pal_size = 0;

	pthread_mutex_init(&g_pal_buf.mutex, NULL);
	ret = pthread_create(&pid, &attr, &uvc_stream_process, (void*)&fmt);
	if (ret) {
		printf("ERROR(%s): create thread for uvc_stream_process failed!\n", TAG);
		return -1;
	}

	return 0;
}

void  sample_get_stream_process_deinit(int fmt)
{

	g_pal_buf.stop = 1;
	g_pal_buf.pal_size = 0;
	/* pthread_cancel(g_pal_buf.pid); */
	pthread_mutex_destroy(&g_pal_buf.mutex);
	g_pal_buf.pid = 0;
	sem_wait(&g_pal_buf.sem_stop);

}