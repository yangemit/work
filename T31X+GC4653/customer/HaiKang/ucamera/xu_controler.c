/*************************************************************************
* Copyright (C) 2005~2022 Ingenic Semiconductor Co.,Ltd
*
* File Name    : xu_controler.c
* Author       : wxyang
* Mail         : wxyang@ingenic.com
* Created Time : 2022-05-21 14:56
*************************************************************************/
#include <string.h>
#include <stdio.h>
#include "ucam/usbcamera.h"
#include "xu_controler.h"

#define	SET_CUR  0x01
#define	GET_CUR  0x81
#define	GET_MIN  0x82
#define	GET_MAX  0x83
#define	GET_RES  0x84
#define	GET_LEN  0x85
#define	GET_INFO 0x86
#define	GET_DEF  0x87

static struct Ucamera_Video_EU_Control euctl;

/*
 * eu_cs6 数据保存，对于同一类协议，只需一个buf 即可
 * 扩展协议根据厂商，自定义
 * len: 扩展协议一次传输数据长度（byte）
 * type: 扩展协议通道 和 驱动匹配
 * UVC_MAX: 最大值
 * UVC_MIN: 最小值
 * UVC_CUR: 当前协议设置值
 *
 */
static struct Ucamera_Video_EU_Control eu_cs6 = {
	.type = UVC_EU_CMD_USR6,
	.len = EU_CS6_BUFFER_LEN,
	.data[UVC_MAX] = {
		0x01, 0x03, 0x03,
	},
	.data[UVC_MIN] = {
		0x00,
	},
	.data[UVC_DEF] = {
		0x01, 0x03, 0x01,
	},
	.data[UVC_RES] = {
		0x00,
	},
	.data[UVC_CUR] = {
		0x01, 0x03, 0x01,
	},
};

static int sample_video_eu_set(int cmd, void *data, int len)
{
	int i = 0;
	unsigned char data_cpy[32] = {0};
	memcpy(data_cpy, (char *)data, len);

	switch(cmd){
	case UVC_EU_CMD_USR1:
	case UVC_EU_CMD_USR2:
	case UVC_EU_CMD_USR3:
	case UVC_EU_CMD_USR4:
	case UVC_EU_CMD_USR5:
	case UVC_EU_CMD_USR6:
		{

			for(i = 0; i < len; i++) {
				/*0x01 0x03 0x01 */
				 printf("SET_CUR CS6: data_cpy[%d] = 0x%x\n", i, data_cpy[i]);
			}
			memcpy(eu_cs6.data[UVC_CUR], data_cpy, len);
		}
		break;
	default:
		Ucamera_LOG("Unkown uvc eu cmd:%d\n", cmd);
	}
	return 0;
}

static int sample_video_eu_get(int cmd, int req, void *data, int len)
{
	uint8_t *data_cpy = NULL;
	data_cpy = (uint8_t *)data;
	switch(cmd){
	/* 同一类协议处理 */
	case UVC_EU_CMD_USR1:
	case UVC_EU_CMD_USR2:
	case UVC_EU_CMD_USR3:
	case UVC_EU_CMD_USR4:
	case UVC_EU_CMD_USR5:
	case UVC_EU_CMD_USR6:
		{
			switch(req){
			case GET_LEN:
				/*printf("get max: len = %d \n", len);*/
				memset(data_cpy, 0, 60);
				memcpy(data_cpy, &eu_cs6.len, sizeof(int));
				break;
			case GET_MAX:
				/*printf("get max: len = %d \n", len);*/
				memset(data_cpy, 0, 60);
				memcpy(data_cpy, eu_cs6.data[UVC_MAX], len);
				break;
			case GET_MIN:
				/*printf("get min: len = %d \n", len);*/
				memset(data_cpy, 0, 60);
				memcpy(data_cpy, eu_cs6.data[UVC_MIN], len);
				break;
			case GET_DEF:
				/*printf("get def: len = %d \n", len);*/
				memset(data_cpy, 0, 60);
				memcpy(data_cpy, eu_cs6.data[UVC_DEF], len);
				break;
			case GET_RES:
				/*printf("get res: len = %d \n", len);*/
				memset(data_cpy, 0, 60);
				memcpy(data_cpy, eu_cs6.data[UVC_RES], len);
				break;
			case GET_CUR:
				/*printf("get cur: len = %d \n", len);*/
				memset(data_cpy, 0, 60);
				memcpy(data_cpy, eu_cs6.data[UVC_CUR], len);
				break;
			}
		}
		break;

	default:
		{
			Ucamera_LOG("Unkown uvc eu cmd:%d\n", cmd);
			switch(req){
			case GET_LEN:
				memset(data_cpy, 0, 60);
				memcpy(data_cpy, &eu_cs6.len, sizeof(int));
				break;
			case GET_MAX:
			case GET_MIN:
			case GET_DEF:
			case GET_RES:
			case GET_CUR:
				memset(data_cpy, 0, 60);
				break;
			}

		}
		break;
	}

	return len;
}

void register_xu_cb()
{
	euctl.set = sample_video_eu_set;
	euctl.get = sample_video_eu_get;
	Ucamera_Video_Regesit_Extension_Unit_CB(euctl);
}
