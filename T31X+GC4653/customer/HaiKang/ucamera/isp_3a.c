/***********************************************************
 * @file isp_3a.c
 * @note HangZhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 * @brief isp模块3A相关函数实现
 *
 * @Modification History:
 * @<version>   <time>      <author>         <desc>
 * @  1.0.0 - 2015/07/17 - [Ding sisi] - created file
 * 
 * @warning:
 *     All rights reserved. No Part of this file may be reproduced, stored
 *     in a retrieval system, or transmitted, in any form, or by any means,
 *     electronic, mechanical, photocopying, recording, or otherwise,
 *     without the prior consent of Hikvision, Inc.
 *
 ***********************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include "imp-common.h"

#define TAG "Sample-UCamera"

#include "ucam/usbcamera.h"
#include "anticopy/AntiCopy_Verify.h"

#include "hikisp/isp_inner.h"
#include "hik_config.h"

extern int g_fps;
//2020.8.27 HK_ADD
extern IMAGE_QUALITY	g_img_quality;
extern HIK_AAA_PARAM g_aaa_param;
extern struct video_isp_class g_isp_object;
//此宏需与seting库保持一致
#define ISP_AUTO_LINEAR		0
#define ISP_AUTO_WDR	 	1
#define ISP_AUTO_OTHER		-1

// #define HIK_USB_OS02D20 1
// #define HIK_USB_OS04C10 2
// //宏选择OS04C10和OS02D20
// #define HIK_USB_PRODUCT HIK_USB_OS02D20

/*白平衡enBayerType和channel映射表*/
const unsigned char g_awb_fmt2ch[4][4] = {
	{ 0, 1, 3, 2},	//RGGB
	{ 1, 0, 2, 3},	//GRBG
	{ 3, 2, 0, 1},	//BGGR
	{ 2, 3, 1, 0},	//GBRG
};

//int g_average = 45;
#if 0
static U16 hist_weight_auto[256]= 
{
	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,
	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,
	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,
	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	3,	3,
	3,	4,	4,	4,	5,	5,	5,	5,	6,	6,	6,	7,	7,	7,	8,	8,
	8,	9,	9,	9,	10,	10,	10,	11,	11,	11,	12,	12,	12,	12,	13,	13,
	13,	14,	14,	14,	15,	15,	15,	16,	16,	16,	17,	17,	17,	18,	18,	18,
	19,	19,	19,	19,	20,	20,	20,	21,	21,	21,	22,	22,	22,	23,	23,	23,
	24,	24,	24,	25,	25,	25,	26,	26,	26,	26,	27,	27,	27,	28,	28,	28,
	29,	29,	29,	30,	30,	30,	31,	31,	31,	32,	32,	32,	33,	33,	33,	33,
	34,	34,	34,	35,	35,	35,	36,	36,	36,	37,	37,	37,	38,	38,	38,	39,
	39,	39,	40,	40,	40,	40,	41,	41,	41,	42,	42,	42,	43,	43,	43,	44,
	44,	44,	45,	45,	45,	46,	46,	46,	47,	47,	47,	47,	48,	48,	48,	49,
	49,	49,	50,	50,	50,	51,	51,	51,	52,	52,	52,	53,	53,	53,	54,	54,
	54,	54,	55,	55,	55,	56,	56,	56,	57,	57,	57,	58,	58,	58,	59,	59,
	59,	60,	60,	60,	61,	61,	61,	61,	62,	62,	62,	63,	63,	63,	64,	64,
};
#endif

isp_ae_weight_table_t g_isp_ae_weight_table[8] =
{
	/* 0:禁用 */
	{
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 3, 3, 5, 5, 5, 5, 3, 3, 1, 1, 1, 1,
			1, 1, 1, 1, 3, 3, 5, 5, 5, 5, 3, 3, 1, 1, 1, 1,
			1, 1, 1, 1, 3, 3, 5, 5, 5, 5, 3, 3, 1, 1, 1, 1,
			1, 1, 1, 1, 3, 3, 5, 5, 5, 5, 3, 3, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		}
	},
	/* 1:上 */
	{
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
		}
	},
	/* 2:下 */
	{
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		}
	},
	/* 3:左 */
	{
		{
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
		}
	},
	/* 4:右 */
	{
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
		}
	},
	/* 5:中 */
	{
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
			0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	/* 6:自定义 */
	{
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	/* 7:自动 */
	{
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		}
	},
};


isp_ae_weight_table_t g_isp_ae_weight_table_fisheye[8] =
{
	/* 0:禁用 */
	{
		{
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 3, 3, 3, 3, 3, 3, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 3, 3, 5, 5, 5, 5, 3, 3, 1, 1, 0, 0,
			0, 0, 1, 1, 3, 3, 5, 5, 5, 5, 3, 3, 1, 1, 0, 0,
			0, 0, 1, 1, 3, 3, 5, 5, 5, 5, 3, 3, 1, 1, 0, 0,
			0, 0, 1, 1, 3, 3, 5, 5, 5, 5, 3, 3, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 3, 3, 3, 3, 3, 3, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
		}
	},
	/* 1:上 */
	{
		{
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
		}
	},
	/* 2:下 */
	{
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
		}
	},
	/* 3:左 */
	{
		{
			0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
			0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
			0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
		}
	},
	/* 4:右 */
	{
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 
		}
	},
	/* 5:中 */
	{
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	/* 6:自定义 */
	{
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		}
	},
	/* 7:自动 */
	{
		{
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
		}
	},
};

/***************************2021.3.17******************************/
// add severa group gamma
IMPISPGamma g_gam_mid = {{
   0, 122, 243, 363, 482, 599, 716, 832,
 947,1062,1176,1282,1379,1466,1546,1624,
1699,1772,1843,1911,1976,2038,2096,2152,
2204,2252,2299,2344,2387,2429,2470,2509,
2546,2582,2617,2651,2683,2715,2746,2776,
2805,2833,2861,2887,2913,2938,2962,2985,
3008,3030,3051,3072,3093,3113,3132,3151,
3170,3188,3205,3223,3239,3255,3271,3286,
3301,3315,3329,3342,3355,3367,3379,3391,
3403,3415,3427,3439,3451,3463,3475,3487,
3500,3512,3524,3536,3548,3560,3572,3585,
3597,3609,3621,3634,3646,3658,3670,3683,
3695,3707,3720,3732,3744,3757,3769,3781,
3794,3806,3819,3831,3843,3856,3868,3881,
3893,3906,3918,3931,3943,3956,3968,3981,
3993,4006,4019,4031,4044,4057,4069,4082,
4095}};

IMPISPGamma g_gam_high = {{
0,   36,   67,  111,  152,  191,  243,  295,
343,  395,  451,  507,  563,  627,  695,  759,
827,  899,  971, 1047, 1139, 1227, 1311, 1399,
1495, 1587, 1687, 1775, 1863, 1947, 2035, 2115,
2187, 2275, 2351, 2423, 2499, 2571, 2640, 2704,
2768, 2832, 2896, 2948, 3004, 3056, 3112, 3168,
3216, 3260, 3308, 3348, 3392, 3429, 3469, 3501,
3537, 3565, 3601, 3629, 3653, 3681, 3701, 3728,
3744, 3767, 3779, 3803, 3819, 3835, 3851, 3867,
3879, 3891, 3903, 3915, 3927, 3935, 3947, 3955,
3963, 3967, 3975, 3983, 3991, 3995, 4003, 4007,
4011, 4019, 4023, 4027, 4031, 4034, 4034, 4039,
4043, 4047, 4051, 4055, 4054, 4055, 4059, 4063,
4067, 4067, 4071, 4075, 4075, 4079, 4078, 4079,
4083, 4082, 4083, 4087, 4086, 4086, 4087, 4091,
4090, 4090, 4090, 4091, 4095, 4094, 4095, 4094,
4095}};

IMPISPGamma g_gam_low = {{
   0, 332, 611, 837,1014,1146,1249,1341,
1423,1495,1558,1617,1673,1726,1774,1820,
1861,1899,1936,1973,2008,2043,2077,2110,
2142,2173,2204,2234,2263,2291,2319,2346,
2372,2398,2423,2447,2470,2493,2515,2536,
2557,2577,2596,2615,2633,2651,2668,2686,
2704,2721,2739,2756,2774,2791,2808,2826,
2843,2860,2877,2894,2911,2927,2944,2961,
2977,2993,3009,3026,3042,3058,3074,3090,
3106,3122,3138,3153,3169,3185,3200,3215,
3231,3246,3261,3276,3291,3306,3321,3336,
3350,3365,3380,3395,3411,3427,3442,3458,
3475,3491,3508,3524,3541,3559,3576,3593,
3611,3629,3647,3666,3684,3703,3722,3741,
3760,3780,3799,3819,3839,3859,3880,3900,
3921,3942,3963,3985,4006,4028,4050,4072,
4095}};

IMPISPGamma g_gam_ref = {{
   0,  32,  64,  96, 128, 160, 192, 224,
 256, 288, 320, 352, 384, 416, 448, 480,
 512, 544, 576, 608, 640, 672, 704, 736,
 768, 800, 832, 864, 896, 928, 960, 992,
1024,1056,1088,1120,1152,1184,1216,1248,
1280,1312,1344,1376,1408,1440,1472,1504,
1536,1568,1600,1632,1664,1696,1728,1760,
1792,1824,1856,1888,1920,1952,1984,2016,
2048,2079,2111,2143,2175,2207,2239,2271,
2303,2335,2367,2399,2431,2463,2495,2527,
2559,2591,2623,2655,2687,2719,2751,2783,
2815,2847,2879,2911,2943,2975,3007,3039,
3071,3103,3135,3167,3199,3231,3263,3295,
3327,3359,3391,3423,3455,3487,3519,3551,
3583,3615,3647,3679,3711,3743,3775,3807,
3839,3871,3903,3935,3967,3999,4031,4063,
4095}};

IMPISPGamma g_gam_default = {{
   0, 208, 404, 580, 728, 864, 988,1112,
1216,1320,1448,1536,1620,1700,1780,1860,
1940,2020,2099,2171,2243,2315,2387,2455,
2515,2571,2627,2675,2723,2767,2807,2847,
2887,2927,2967,3007,3047,3079,3119,3159,
3199,3219,3239,3259,3279,3299,3319,3339,
3359,3379,3397,3413,3429,3443,3455,3467,
3479,3491,3503,3515,3527,3539,3551,3563,
3575,3587,3599,3609,3617,3625,3633,3641,
3649,3657,3665,3673,3681,3689,3697,3705,
3713,3721,3729,3737,3745,3753,3761,3769,
3777,3779,3791,3799,3811,3819,3831,3839,
3851,3859,3867,3875,3887,3895,3903,3911,
3919,3927,3935,3943,3951,3959,3967,3971,
3979,3987,3995,4003,4011,4019,4027,4031,
4039,4047,4051,4059,4067,4071,4079,4083,
4095}};
/******************************************2020.9.1 - LHY ADD*****************************************/

HRESULT ISP_GetMemSize(ISP_CREATE_PARAM *param)
{
	S32 ret = ISP_LIB_S_OK;

	CHECK_RET(NULL == param, ISP_LIB_E_PARA_NULL);

	param->buf_size = sizeof(ISP_INNER_PARAM);
	param->buf_size += sizeof(ISP_CFG_TABLES);
	param->buf_size += sizeof(ISP_INNER_CTRL);
	
	/** 内存保护区段1KB */
	param->buf_size += 1024;
	return ret;

}

HRESULT ISP_Create(ISP_CREATE_PARAM *param,VOID ** handle)
{
	ISP_INNER_PARAM *inner_param;
	ISP_BUF inner_buf;
	S32 ret = 0;
	CHECK_RET(NULL == param || NULL == handle, ISP_LIB_E_PARA_NULL);
	CHECK_RET(NULL == param->buf, ISP_LIB_E_PARA_NULL);
	//*handle = param->buf;

#if(HIK_USB_PRODUCT == HIK_USB_OS04C10)
	g_isp_object.create_param.max_width = 2560;
	g_isp_object.create_param.max_height = 1440;
	g_isp_object.create_param.sensor_id = OMNI_OS04C10;
	g_vmax = 1000;//TBD...
#endif

#if(HIK_USB_PRODUCT == HIK_USB_OS02D20)
	g_isp_object.create_param.max_width = 1920;
	g_isp_object.create_param.max_height = 1080;
	g_isp_object.create_param.sensor_id = OMNI_OS02D20;
	g_vmax = 1318;//TBD...
#endif

#if(HIK_USB_PRODUCT == HIK_USB_JXF37)
	g_isp_object.create_param.max_width = 1920;
	g_isp_object.create_param.max_height = 1080;
	g_isp_object.create_param.sensor_id = SOI_JXF37;
	g_vmax = 1512;//25fps
#endif

	g_isp_object.create_param.print_en = 0;
	g_isp_object.create_param.af_params.af_type = 0;
	g_isp_object.create_param.af_params.abf_type = 0;

	/**1.分配isp memory*/
	inner_buf.start   = param->buf;
	inner_buf.cur_pos = param->buf;
	inner_buf.end     = (VOID*)((U32)inner_buf.cur_pos + param->buf_size - 1);
	
	/**分配内部参数存储空间 */
	inner_param = (ISP_INNER_PARAM *) isp_alloc_buffer(&inner_buf, sizeof(ISP_INNER_PARAM));
	CHECK_RET(NULL == inner_param, ISP_LIB_E_MEM_OVER);
	memset(inner_param,0,sizeof(ISP_INNER_PARAM));
	inner_param->fd_iav         	= param->fd_iav;
	inner_param->res_type			= param->res_type;
	inner_param->lens_type			= param->lens_type;

	//保存镜头类型
	memcpy(&af_statis, &(param->af_params), sizeof(AF_PARAM));
	PRINT_INFO(DBG_KEY, g_dbg_level, "af_type=%d,lens_type=%d\n",af_statis.af_type,inner_param->lens_type);

	/**分配图像参数表存储空间*/
	gp_isp_cfg_tables = (ISP_CFG_TABLES *) isp_alloc_buffer(&inner_buf, sizeof(ISP_CFG_TABLES));
	CHECK_RET(NULL == gp_isp_cfg_tables, ISP_LIB_E_MEM_OVER);
	inner_param->cfg_tables = gp_isp_cfg_tables;
	memset(gp_isp_cfg_tables, 0, sizeof(ISP_CFG_TABLES));

	
	/**分配图像控制参数存储空间*/
	gp_isp_inner_ctrl = (ISP_INNER_CTRL *) isp_alloc_buffer(&inner_buf, sizeof(ISP_INNER_CTRL));
	
	inner_param->isp_inner_ctrl = gp_isp_inner_ctrl;
	CHECK_RET(NULL == gp_isp_inner_ctrl, ISP_LIB_E_MEM_OVER);
	memset(gp_isp_inner_ctrl, 0, sizeof(ISP_INNER_CTRL));
	//gp_isp_inner_ctrl->mem_start = param->buf;//干啥用的???????????
	gp_isp_inner_ctrl->sensor_id = g_isp_object.create_param.sensor_id;
	gp_isp_inner_ctrl->mirror_en = 0;

	memset(&gp_isp_inner_ctrl->aec_ctrl, 0, sizeof(ISP_AEC_CTRL));
	memset(&gp_isp_inner_ctrl->awb_ctrl, 0, sizeof(AWB_CTRL));
	memset(&gp_isp_inner_ctrl->ccm_ctrl, 0, sizeof(CCM_CTRL));
	/**加载配置文件*/ 
	//isp_sensor_init_cfg(inner_param);
	/**sensor回调函数注册*/
	//ret = isp_sensor_register_cb(inner_param);
	/**3.isp start*/
	inner_param->aaa_param.awb_param.mode = HIK_WB_AWB2_MODE;
	ret = isp_start_isp(inner_param);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	return ISP_LIB_S_OK;
}





#if 000000000000000000000
/* @fn      ISP_Set_Vmax
  * @brief ISP_Set_Vmax函数实现,用于实现曝光库组件化
  * @param   [in] void
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
S32 isp_set_vmax(void)
{
	S32 cur_vmax = 0;
	switch(gp_isp_inner_ctrl->sensor_id)
	{
		//case PANASONIC_MN34425:
		//	cur_vmax = get_sensor_mn34425_vmax();
		//	break;
		case OMNI_OV2735:
			cur_vmax = get_sensor_ov2735_vmax();
			break;
		//case SOI_K02:
		//	cur_vmax = get_sensor_k02_vmax();
		//	break;
		//case OMNI_SP2309:
		//	cur_vmax = get_sensor_sp2309_vmax();
		//	break;
		default:
			cur_vmax = get_sensor_ov2735_vmax();
			printf("<ISP> not support this senor(0x%x) get vmax!\n", gp_isp_inner_ctrl->sensor_id);
			break;
	}
	printf("cur_vmax is %d\n",cur_vmax);
	AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_SET_VMAX_KEY, cur_vmax);

	return ISP_LIB_S_OK;
}

/* @fn      hik_ae_init
  * @brief hik ae库初始化
  * @param   [in] void
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT hik_ae_init(ISP_INNER_PARAM *inner_param)
{
	U32 ret = 0;
	
	/**关闭内置的AE算法*/
	API_ISP_AEAlgEn(FH_FALSE);
	
	/**HIK AE内存分配与AE句柄创建*/
	gp_isp_inner_ctrl->aec_ctrl.total_gain = 100;/**0db*/
	#if 0
	if(PANASONIC_MN34425 == gp_isp_inner_ctrl->sensor_id)
	{
		if(WDR_MODE_2To1_LINE == gp_isp_inner_ctrl->wdr_mode)
		{
			gp_isp_inner_ctrl->aec_ctrl.createParam.ae_sensor_type = CMOS_MN34425_2to1_LINE;
		}
		else
		{
			gp_isp_inner_ctrl->aec_ctrl.createParam.ae_sensor_type = CMOS_MN34425_LINEAR;
		}
		gp_isp_inner_ctrl->aec_ctrl.createParam.ae_resolution_framerate = AEC_1080P_25FPS;
		gp_isp_inner_ctrl->aec_ctrl.createParam.light_freq = LIGHT_FREQ_50HZ;
	}
	#endif 
	if(OMNI_OV2735 == gp_isp_inner_ctrl->sensor_id)
	{
		gp_isp_inner_ctrl->aec_ctrl.createParam.ae_sensor_type = CMOS_OS02E10_LINEAR;
		gp_isp_inner_ctrl->aec_ctrl.createParam.ae_resolution_framerate = AEC_1080P_25FPS;
		gp_isp_inner_ctrl->aec_ctrl.createParam.light_freq = LIGHT_FREQ_50HZ;
	}
	#if 0
	else if(SOI_K02== gp_isp_inner_ctrl->sensor_id)
	{
		gp_isp_inner_ctrl->aec_ctrl.createParam.ae_sensor_type = CMOS_K02_LINEAR;
		gp_isp_inner_ctrl->aec_ctrl.createParam.ae_resolution_framerate = AEC_2048x1536_25FPS;
		gp_isp_inner_ctrl->aec_ctrl.createParam.light_freq = LIGHT_FREQ_50HZ;
	}
	else if(OMNI_SP2309 == gp_isp_inner_ctrl->sensor_id)
	{
		gp_isp_inner_ctrl->aec_ctrl.createParam.ae_sensor_type = CMOS_SP2309_LINEAR;
		gp_isp_inner_ctrl->aec_ctrl.createParam.ae_resolution_framerate = AEC_1080P_25FPS;
		gp_isp_inner_ctrl->aec_ctrl.createParam.light_freq = LIGHT_FREQ_50HZ;
	}
	#endif
	else
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, "ae_init: not support this sensor !!\n");
	}
	
	ret = AEC_GetMemSize(&gp_isp_inner_ctrl->aec_ctrl.createParam, gp_isp_inner_ctrl->aec_ctrl.memTab);
	if (HIK_AE_LIB_S_OK != ret)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, "hik_ae_init: AEC_GetMemSize FAILED, ret=%d\n", ret);
		return ISP_LIB_S_FAIL;
	}
	PRINT_INFO(DBG_FUNCTION, g_dbg_level, "alloc ae mem = %d Byte\n",gp_isp_inner_ctrl->aec_ctrl.memTab[0].size);
	
	if (ISP_LIB_E_MEM_NULL == isp_alloc_mem_tab(gp_isp_inner_ctrl->aec_ctrl.memTab, HIK_AE_MTAB_NUM))
	{
		isp_free_mem_tab(gp_isp_inner_ctrl->aec_ctrl.memTab, HIK_AE_MTAB_NUM);
		PRINT_INFO(DBG_KEY,g_dbg_level, "hik_ae_init: alloc memTab FAILED, ret=%d\n", ret);
		return ISP_LIB_E_MEM_NULL;
	}
	
	ret = AEC_Create(&gp_isp_inner_ctrl->aec_ctrl.createParam, gp_isp_inner_ctrl->aec_ctrl.memTab, &gp_isp_inner_ctrl->aec_ctrl.handle);
	if ((ret != HIK_AE_LIB_S_OK) || (NULL == gp_isp_inner_ctrl->aec_ctrl.handle))
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, "hik_ae_init: AEC_Create FAILED, ret=%d\n", ret);
		return ISP_LIB_S_FAIL;
	}

	/**HIK AE键值初始化*/
	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_SET_APER_TYPE, APERTURE_FAST);
	//ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_VREF_COR_EN, 0);//关闭电压矫正模块
	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_IRIS_DTC_EN, 1);//开启光圈灵敏度自动调整模块
	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_IRIS_DTC_SCHEME_SELECT, IRIS_DETECT_SCHEME_A); //选择方案A

	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_SENSOR_TYPE_KEY, gp_isp_inner_ctrl->aec_ctrl.createParam.ae_sensor_type); //设置sensor类型
	
	if(PANASONIC_MN34425 == gp_isp_inner_ctrl->sensor_id)
	{
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_LIGHT_FREQ_KEY, LIGHT_FREQ_50HZ);
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_RESOLUTION_FRAMERATE_KEY, AEC_1080P_25FPS);
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_AE_MODE_KEY, AEC_SHUT_GAIN);
	}
	else if(OMNI_OV2735 == gp_isp_inner_ctrl->sensor_id)
	{
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_LIGHT_FREQ_KEY, LIGHT_FREQ_50HZ);
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_RESOLUTION_FRAMERATE_KEY, AEC_1080P_25FPS);
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_AE_MODE_KEY, AEC_SHUT_GAIN);
	}
	else if(SOI_K02 == gp_isp_inner_ctrl->sensor_id)
	{
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_LIGHT_FREQ_KEY, LIGHT_FREQ_50HZ);
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_RESOLUTION_FRAMERATE_KEY, AEC_2048x1536_25FPS);
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_AE_MODE_KEY, AEC_SHUT_GAIN);
	}
	else if(OMNI_SP2309 == gp_isp_inner_ctrl->sensor_id)
	{
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_LIGHT_FREQ_KEY, LIGHT_FREQ_50HZ);
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_RESOLUTION_FRAMERATE_KEY, AEC_1080P_25FPS);
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_AE_MODE_KEY, AEC_SHUT_GAIN);
	}
	else
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, "ae_init: not support this sensor !!\n");
	}
	
	//关闭曝光灵敏度开关，不需要等待曝光稳定3秒再调整
	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_SET_CTL_SPEED_EN, 0);
	ret |= isp_set_luma_ref(inner_param);
	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_Y_REF_KEY, 50);
	//按照30ms配置最大曝光时间，后面set_ae_param再刷新
	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_SHUT_MAX_KEY, 33333);
	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_SHUT_MIN_KEY, 10);
	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_GAIN_MAX_KEY, 100);

	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_EXPO_FPS_P_N, (S32)inner_param->aaa_param.ae_param.anti_flicker_mode);
	isp_set_vmax();

	//g_ae_blc_mode = 0;
	//g_smart_ir_status = HIK_FALSE;	//该值初始化为0；若开启了smartIR再赋值。
	return ISP_LIB_S_OK;
}

/* @fn      isp_cal_cur_luma_dol
  * @brief 宽动态获取平均亮度
  * @param   [in] void
  * @return  luma_avg 平均亮度
 */
U32 isp_cal_cur_luma_dol(void)
{
	U64 luma_sum = 0,luma_cnt = 0;
	U32 luma_avg = 0;
 	ISP_HIST_STAT hist_stat;
	U32 i;
	
	/**获取统计信息*/
	memset(&hist_stat, 0, sizeof(ISP_HIST_STAT));
	API_ISP_GetHist(&hist_stat);

	//长帧信息在前16bin上，且处于12bit统计信息的低8bit，亮度在0~255之间
	for(i = 0; i < 16; i++)
	{		
		luma_sum += hist_stat.u32histBin[i][1]; //sum
		luma_cnt += hist_stat.u32histBin[i][0]; //cnt
	}

	//后17个bin为短帧信息，长帧当作过曝处理，全为255
	for(i = 16; i < 33; i++)
	{
		luma_sum += hist_stat.u32histBin[i][0] * 255; //sum
		luma_cnt += hist_stat.u32histBin[i][0]; //cnt
	}
	luma_avg = (luma_sum / MAX(luma_cnt, 1)) << 4;//增加除零保护
		
	//返回亮度，12bit
	return (luma_avg);
}


/* @fn      isp_cal_cur_luma
  * @brief 获取平均亮度
  * @param   [in] void
  * @return  luma_avg 平均亮度
 */
U32 isp_cal_cur_luma(void)
{
	U64 luma_sum = 0,total_sum = AE_ZONE_ALL;
	U32 luma_avg = 0,luma_tmp = 0,luma_cnt = 0, weight = 0;
	GLOBE_STAT isp_ae_stats[AE_ZONE_ALL];
	U32 i;
	U32 temp_y = 0;
	U32 weight_sum = 0;
	U32 index = 0;
	U16 hist_weight_manual[256]= {0};
	
	/**获取统计信息*/
	//printf("GLOBE_STAT is %d\n",sizeof(GLOBE_STAT));
	memset(isp_ae_stats, 0, AE_ZONE_ALL*sizeof(GLOBE_STAT));
	API_ISP_GetGlobeStat(isp_ae_stats);
//	return OK;
	//默认模式下
	if(HIK_FALSE == g_smart_ir_status)
	{
		for(i = 0; i < AE_ZONE_ALL; i++)
		{
			weight = g_isp_ae_weight_table[g_ae_blc_mode].u8Weight[i];
			
			//支持背光补偿，若该块权重不为0，统计亮度+(分块权重*平均亮度)
			if( weight != 0 )
			{
				luma_tmp = isp_ae_stats[i].r.sum + isp_ae_stats[i].gb.sum + \
					isp_ae_stats[i].gr.sum + isp_ae_stats[i].b.sum;
				
				luma_cnt = isp_ae_stats[i].r.cnt + isp_ae_stats[i].gb.cnt + \
					isp_ae_stats[i].gr.cnt + isp_ae_stats[i].b.cnt;
					
				luma_sum += weight * (luma_tmp / MAX(luma_cnt, 1));//增加除零保护
				
				total_sum += weight - 1;//增加分块权重统计块数的影响
			}
			//若该块权重为0，统计块数-1
			else
			{
				total_sum--;
			}
		}
		
		luma_avg = (luma_sum / MAX(total_sum, 1));//增加除零保护
	}
	else
	{
		for(i = 0; i < AE_ZONE_ALL; i++)
		{
			weight = g_isp_ae_weight_table[g_ae_blc_mode].u8Weight[i];
			
			//支持背光补偿，若该块权重不为0，统计亮度+(分块权重*平均亮度)
			if( weight != 0 )
			{
				luma_tmp = isp_ae_stats[i].r.sum + isp_ae_stats[i].gb.sum + \
					isp_ae_stats[i].gr.sum + isp_ae_stats[i].b.sum;
				
				luma_cnt = isp_ae_stats[i].r.cnt + isp_ae_stats[i].gb.cnt + \
					isp_ae_stats[i].gr.cnt + isp_ae_stats[i].b.cnt;
				temp_y = (luma_tmp / MAX(luma_cnt, 1));

				//每个块的亮度范围[0-4095]，右移四位，变换到[0-255]
				index = MIN(255,(temp_y>>4));
				if(set_hist_weight_flag)
				{
					if(index <= 60)
					{
						hist_weight_manual[index] = 2;
					}
					else
					{
						hist_weight_manual[index] = ((hist_weight_max - 2) * index - 60 * hist_weight_max + 510) / 195;
					}
					luma_sum += weight*temp_y*hist_weight_manual[index];
					weight_sum += weight*hist_weight_manual[index];
				}
				else
				{
					luma_sum += temp_y*hist_weight_auto[index];
					weight_sum += hist_weight_auto[index];
				}
			}
		}
		luma_avg = luma_sum/MAX(weight_sum, 1);
		luma_avg = MIN(luma_avg, 4095);
	}
	//返回亮度，12bit
	return (luma_avg);
}

#if 0
HRESULT hik_getAE_statistic()
{
	#if 01
	//printf("11111111111\n");
	if(IS_WDR_NONE(gp_isp_inner_ctrl->wdr_mode))
	{
		rt_kprintf("NONE WDR\n");
		g_average = isp_cal_cur_luma();
	}
	else
	{
		rt_kprintf("WDR\n");
		g_average = isp_cal_cur_luma_dol();
	}
	#endif
}
#endif

/* @fn      hik_ae_run
  * @brief hik ae 库执行函数
  * @param   [in] void
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT hik_ae_run(void)
{
	static int count = 0;
	u32 iso = 100;
	u32 exposure_time = 40000, long_exp = 1;//慢快门倍数
	u32 average_y = 0;
	//rt_kprintf("AEC DOING\n");
	#if 1
	isp_get_current_long_exp(&long_exp);
	//rt_kprintf("g_average is %d\n",g_average);
	#if 01
	//printf("11111111111\n");
	if(IS_WDR_NONE(gp_isp_inner_ctrl->wdr_mode))
	{
		//rt_kprintf("NONE WDR\n");
		gp_isp_inner_ctrl->aec_ctrl.procParam.average_y = isp_cal_cur_luma();
	}
	else
	{
		//rt_kprintf("WDR\n");
		gp_isp_inner_ctrl->aec_ctrl.procParam.average_y = isp_cal_cur_luma_dol();
	}
	#endif
	//gp_isp_inner_ctrl->aec_ctrl.procParam.average_y = g_average;
	//#else
	//usleep(10);
	//rt_kprintf("1--------------------------isp_cal_cur_luma is %u\n",gp_isp_inner_ctrl->aec_ctrl.procParam.average_y);
	gp_isp_inner_ctrl->aec_ctrl.procParam.average_y = gp_isp_inner_ctrl->aec_ctrl.procParam.average_y >> 4;
	
	/**执行hik ae程序*/
	AEC_Process(gp_isp_inner_ctrl->aec_ctrl.handle, &gp_isp_inner_ctrl->aec_ctrl.procParam);
	/**传给awb库环境评估亮度*/
	iso = (gp_isp_inner_ctrl->aec_ctrl.total_gain>>6)*100;
	exposure_time = gp_isp_inner_ctrl->aec_ctrl.procParam.aec_ret_params.aec_expose_time[0];
	average_y = gp_isp_inner_ctrl->aec_ctrl.procParam.average_y<<10;
	gp_isp_inner_ctrl->awb_ctrl.awb_lux = pow(((float)average_y / ((float)exposure_time * (float)iso / 100000000)), 0.5);
	if(count++ % 20 == 0)
	{	
		//rt_kprintf("exposure_time is %d\n",exposure_time);
		//PRINT_INFO(DBG_KEY, g_dbg_level,"average_y : %d\tiso:%d\texptime:%d\n",gp_isp_inner_ctrl->aec_ctrl.procParam.average_y,
		//iso,exposure_time);
	}
	#endif
	return ISP_LIB_S_OK;
}


/* @fn      hik_ae_exit
  * @brief hik ae 库退出函数
  * @param   [in] void
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
s32 hik_ae_exit(void)
{
	return ISP_LIB_S_OK;
}


/* @fn      isp_stop_ae
  * @brief hik ae 库停止函数
  * @param   [in] inner_param
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_stop_ae(ISP_INNER_PARAM *inner_param)
{		
	S32 ret_api;
	ret_api = AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle,AEC_AE_SWITCH_KEY,0);
	CHECK_RET(ret_api < 0, ISP_LIB_S_FAIL);
	return ISP_LIB_S_OK;
}


/* @fn      isp_start_ae
  * @brief hik ae 库启动函数
  * @param   [in] inner_param
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_start_ae(ISP_INNER_PARAM *inner_param)
{		
	S32 ret_api;
	ret_api = AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle,AEC_AE_SWITCH_KEY,1);
	CHECK_RET(ret_api < 0, ISP_LIB_S_FAIL);
	return ISP_LIB_S_OK;
}


/* @fn      isp_set_brightness
  * @brief 平台设置亮度
  * @param   [in] inner_param,isp 参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
//HRESULT isp_set_brightness(ISP_INNER_PARAM *inner_param, S32 param_val)
HRESULT isp_set_brightness(S32 param_val)
{
	S32 ret_api;
	g_img_quality.brightness = param_val;
	param_val = LINEAR_INTERPOLATION(1,255,1,100,param_val);

	//rt_kprintf("target brightness is %d\n",param_val);
	#if 0
	//真宽动态下强光抑制开启，降低目标亮度到80%
	if((1 == inner_param->img_enhance.high_light_control_enable)&&(IS_WDR_TRUE(inner_param->img_enhance.auto_local_exp_mode)))
	{
		param_val = MAP_FRONT_DEFAULT_PARAM(param_val,40);
	}
	#endif
	ret_api = AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle,AEC_Y_REF_KEY, param_val);
	CHECK_RET(ret_api < 0, ISP_LIB_S_FAIL);

	return ISP_LIB_S_OK;
}


/* @fn      isp_set_shutter_fastest
  * @brief 设置ae 快门最快值
  * @param   [in] inner_param,isp 参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_set_shutter_fastest(ISP_INNER_PARAM *inner_param, S32 param_val)
{
	S32 ret_api;
	ret_api = AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle,AEC_SHUT_MIN_KEY,param_val);
	CHECK_RET(ret_api < 0, ISP_LIB_S_FAIL);
	return ISP_LIB_S_OK;

}


/* @fn      isp_set_shutter_slowest
  * @brief 设置ae 快门最慢值
  * @param   [in] inner_param,isp 参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
//HRESULT isp_set_shutter_slowest(ISP_INNER_PARAM *inner_param, S32 param_val)
//HRESULT isp_set_shutter_slowest(S32 param_val)
HRESULT isp_set_shutter_slowest( )
{
	S32 ret_api;
	int exp_time;
	//g_aaa_param.ae_param.shutter_time_max = param_val;
	if(OMNI_OV2735 ==gp_isp_inner_ctrl->sensor_id)
	{
		if(g_fps == 25)
		{
			exp_time = 40000;
		}
		else
		{
			exp_time = 40000 * 833 / 1000;
		}
	}
	//rt_kprintf("isp_set_shutter_slowest is %d\n",exp_time);
	ret_api = AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle,AEC_SHUT_MAX_KEY,exp_time);
	CHECK_RET(ret_api < 0, ISP_LIB_S_FAIL);
	return ISP_LIB_S_OK;
}


/* @fn      isp_set_shutter_manual
  * @brief 设置ae的手动快门时间	-af使用，写快门时间，应用层不调用该接口，高帧率不除2
  * @param   [in] inner_param,isp 参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_set_shutter_manual(ISP_INNER_PARAM *inner_param, S32 param_val)
{
	return ISP_LIB_S_OK;
}


/* @fn      isp_set_gain_max
  * @brief 设置ae的最大增益值, 和sensor相关
  * @param   [in] inner_param,isp 参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_set_gain_max(ISP_INNER_PARAM *inner_param, S32 param_val)
{	
	S32 ret_api;
	ret_api = AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle,AEC_GAIN_MAX_KEY,param_val);
	CHECK_RET(ret_api < 0, ISP_LIB_S_FAIL);
	return ISP_LIB_S_OK;
}


/* @fn      isp_set_gain_manual
  * @brief 设置ae的手动增益值
  * @param   [in] inner_param,isp 参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_set_gain_manual(ISP_INNER_PARAM *inner_param, S32 param_val)
{	
	S32 ret_api;
	ret_api = AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle,AEC_SET_GAIN_REG,param_val);//0-100
	CHECK_RET(ret_api < 0, ISP_LIB_S_FAIL);
	return ISP_LIB_S_OK;

}


/* @fn      isp_get_bayer_luma
  * @brief 获取图像亮度(bayer域)
  * @param   [in] param_val,亮度
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_get_bayer_luma(S32 *param_val)
{
	*param_val = gp_isp_inner_ctrl->aec_ctrl.procParam.average_y;
	return ISP_LIB_S_OK;
}


/* @fn      isp_get_gain_level
  * @brief 获取增益等级 0-100,(提供AF 曲线测试)
  * @param   [in] param_val,增益
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_get_gain_level(S32 *param_val)
{	
	S32 gain_level = 0;
	
	gain_level = gp_isp_inner_ctrl->aec_ctrl.procParam.aec_ret_params.aec_gain_level; // gain level 0-100 
	*param_val = gain_level;
	return ISP_LIB_S_OK;
}


/* @fn      isp_get_exp_status
  * @brief 获取曝光状态
  * @param   [in] param_val,曝光状态
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_get_exp_status( S32* exposure_status )
{
	*exposure_status = gp_isp_inner_ctrl->aec_ctrl.procParam.aec_ret_params.aec_status_result;
	return ISP_LIB_S_OK;
}

/************************************************************************************************
 * 功 能：HIK自定义设置ae参考亮度的函数
 * 参 数：inner_param
 *
 *
 * 返回值：状态值
 *************************************************************************************************/
HRESULT isp_set_luma_ref(ISP_INNER_PARAM * inner_param)
{
	S32 ret = 0;
	AEC_Y_REF_LIST luma_ref;
	
	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_SET_GA_YRA_EN, 1); //参考亮度随增益衰减使能

	if(0 == inner_param->img_enhance.high_light_control_enable && IS_WDR_NONE(inner_param->img_enhance.auto_local_exp_mode))//linear
	{
		memcpy(&luma_ref, gp_isp_inner_ctrl->aec_ctrl.y_ref_map_linear, sizeof(AEC_Y_REF_LIST));
	}
	else if(1 == inner_param->img_enhance.high_light_control_enable && IS_WDR_TRUE(inner_param->img_enhance.auto_local_exp_mode))//真宽动态强光抑制
	{
		memcpy(&luma_ref, gp_isp_inner_ctrl->aec_ctrl.y_ref_map_hlc, sizeof(AEC_Y_REF_LIST));
	}
	else if(1 == inner_param->img_enhance.high_light_control_enable && IS_WDR_NONE(inner_param->img_enhance.auto_local_exp_mode))//数字强光抑制
	{
		memcpy(&luma_ref, gp_isp_inner_ctrl->aec_ctrl.y_ref_map_linear_hlc, sizeof(AEC_Y_REF_LIST));
	}
	else // 真宽动态
	{
		memcpy(&luma_ref, gp_isp_inner_ctrl->aec_ctrl.y_ref_map_wdr, sizeof(AEC_Y_REF_LIST));
	}

	ret |= AEC_SetYRef(gp_isp_inner_ctrl->aec_ctrl.handle, &luma_ref);//wdr
	
	return ret;
}

/* @fn      isp_set_ae_param
  * @brief 根据帧率，制式配置ae参数
  * @param   [in] inner_param,isp 参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_set_ae_param(ISP_INNER_PARAM * inner_param)
{
	rt_kprintf("=================isp_set_ae_param===================\n");
	S32 ret = 0;
	U32 resolution_framerate = AEC_1080P_25FPS;
	AEC_SENSOR_TYPE sensor_work_mode = CMOS_OS02E10_LINEAR;

	/****************** 配置 AE sensor work type ********************/
	switch (gp_isp_inner_ctrl->sensor_id)
	{ 
		case OMNI_OV2735:
			sensor_work_mode = CMOS_OS02E10_LINEAR;
			gp_isp_inner_ctrl->aec_ctrl.createParam.ae_sensor_type = CMOS_OS02E10_LINEAR;
			break;
 		default:
			PRINT_INFO(DBG_KEY,g_dbg_level, "The sensor 0x%x not support !!\n",gp_isp_inner_ctrl->sensor_id);
			break;
	}
 	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_SENSOR_TYPE_KEY, sensor_work_mode);
 	/**********************配置 AE resolution ***************************/
	switch (gp_isp_inner_ctrl->sensor_id)
	{
		case PANASONIC_MN34425:
			if (VI_CAPTURE_MODE_1080P30 == gp_isp_inner_ctrl->isp_capture_mode)
			{
				resolution_framerate = AEC_1080P_30FPS;
			}
			else if(VI_CAPTURE_MODE_1080P25  == gp_isp_inner_ctrl->isp_capture_mode)
			{
				resolution_framerate = AEC_1080P_25FPS;
			}
			break;
		case OMNI_OV2735:
			if (VI_CAPTURE_MODE_1080P30 == gp_isp_inner_ctrl->isp_capture_mode ||VI_CAPTURE_MODE_XVGAP30 == gp_isp_inner_ctrl->isp_capture_mode)
			{
				//sensor 只有一种分辨率，1080P
				if(25 == g_fps)
				{
					resolution_framerate = AEC_1080P_25FPS;
				}
				else
				{
					resolution_framerate = AEC_1080P_30FPS;
				}
			}
			break;
		case SOI_K02:
			if (VI_CAPTURE_MODE_2048X1536P20 == gp_isp_inner_ctrl->isp_capture_mode)
			{
				resolution_framerate = AEC_2048x1536_20FPS;
			}
			else if(VI_CAPTURE_MODE_2048X1536P25 == gp_isp_inner_ctrl->isp_capture_mode)
			{
				resolution_framerate = AEC_2048x1536_25FPS;
			}
			else if(VI_CAPTURE_MODE_2048X1536P30 == gp_isp_inner_ctrl->isp_capture_mode)
			{
				resolution_framerate = AEC_2048x1536_30FPS;
			}
			break;
		case OMNI_SP2309:
			if (VI_CAPTURE_MODE_1080P30 == gp_isp_inner_ctrl->isp_capture_mode)
			{
				resolution_framerate = AEC_1080P_30FPS;
			}
			else if(VI_CAPTURE_MODE_1080P25  == gp_isp_inner_ctrl->isp_capture_mode)
			{
				resolution_framerate = AEC_1080P_25FPS;
			}
			break;
		default:
			PRINT_INFO(DBG_KEY,g_dbg_level, "The sensor 0x%x not support !!\n",gp_isp_inner_ctrl->sensor_id);
			break;
	}
	#if 0
	/**********************配置 AE 制式 ***************************/
	switch( gp_isp_inner_ctrl->isp_capture_mode )
	{
		case VI_CAPTURE_MODE_XVGAP25:
		case VI_CAPTURE_MODE_1080P25:
		case VI_CAPTURE_MODE_2048X1536P25:
			ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_LIGHT_FREQ_KEY, LIGHT_FREQ_50HZ);
			break;

		case VI_CAPTURE_MODE_XVGAP30:
		case VI_CAPTURE_MODE_1080P30:
		case VI_CAPTURE_MODE_2048X1536P30:
		case VI_CAPTURE_MODE_2048X1536P20:
			ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_LIGHT_FREQ_KEY, LIGHT_FREQ_60HZ);
			break;
		default:
			PRINT_INFO(DBG_KEY,g_dbg_level, "The Capture_Mode %d not support !!\n",gp_isp_inner_ctrl->isp_capture_mode);
			break;		
	}
	#else
	if(g_fps == 25)
	{
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_LIGHT_FREQ_KEY, LIGHT_FREQ_50HZ);
	}
	else
	{
		ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_LIGHT_FREQ_KEY, LIGHT_FREQ_60HZ);
	}
	#endif
	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_RESOLUTION_FRAMERATE_KEY, (S32)resolution_framerate);
	PRINT_INFO(DBG_FUNCTION,g_dbg_level, "set_ae_param: sensor_work_mode = %d\n",sensor_work_mode);
	ret |= isp_set_luma_ref(inner_param); // 配置参考亮度衰减曲线
	inner_param->aaa_param.ae_param.gain_max = 40;//最大增益限制 40%*48dB = 19dB
	rt_kprintf("inner_param->aaa_param.ae_param.gain_max is %d\n",inner_param->aaa_param.ae_param.gain_max);
	ret |= isp_set_gain_max(inner_param,(S32)inner_param->aaa_param.ae_param.gain_max);
	//inner_param->img_quality.brightness = 50;
	rt_kprintf("inner_param->aaa_param.img_quality.brightness is %d\n",g_img_quality.brightness);
	//ret |= isp_set_brightness(inner_param,(S32)inner_param->img_quality.brightness);
	ret |= isp_set_brightness(g_img_quality.brightness);
	#if 0
	if((VI_CAPTURE_MODE_XVGAP30 == gp_isp_inner_ctrl->isp_capture_mode) ||
		(VI_CAPTURE_MODE_1080P30 == gp_isp_inner_ctrl->isp_capture_mode))
	{
		inner_param->aaa_param.ae_param.anti_flicker_mode = HIK_ANTI_FLICKER_60HZ;
	}
	else
	{
		inner_param->aaa_param.ae_param.anti_flicker_mode = HIK_ANTI_FLICKER_50HZ;
	}
	rt_kprintf("inner_param->aaa_param.ae_param.anti_flicker_mode is %d\n",inner_param->aaa_param.ae_param.anti_flicker_mode);
	//限制最大快门时间
	ret |= AEC_SetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle,AEC_EXPO_FPS_P_N,(S32)inner_param->aaa_param.ae_param.anti_flicker_mode);
	#endif
	ret |= isp_set_vmax();//必须先配置AEC_EXPO_FPS_P_N,再配置VMAX.触发自动生成曝光表.

	//inner_param->aaa_param.ae_param.shutter_time_max = 40000;//us单位
	//rt_kprintf("inner_param->aaa_param.ae_param.shutter_time_max is %d\n",inner_param->aaa_param.ae_param.shutter_time_max);
	//ret |= isp_set_shutter_slowest((S32)inner_param->aaa_param.ae_param.shutter_time_max);
	ret |= isp_set_shutter_slowest();
	inner_param->aaa_param.ae_param.shutter_time_min = 10;
	rt_kprintf("inner_param->aaa_param.ae_param.shutter_time_min is %d\n",inner_param->aaa_param.ae_param.shutter_time_min);
	ret |= isp_set_shutter_fastest(inner_param,(S32)inner_param->aaa_param.ae_param.shutter_time_min);
	rt_kprintf("%s[%d] set OK\n",__FUNCTION__,__LINE__);
	CHECK_RET( ret < 0, ISP_LIB_S_FAIL)
	return ISP_LIB_S_OK;
}


/**
  * @brief 	isp_set_capture_mode
  * @details 
  * @param[in]   inner_param:isp inner global struct. 
  * @param[in] sensor_cfg:sensor attribute.	
  * @retval  ISP_LIB_S_OK  success. 
  * @retval  ISP_LIB_S_FAIL  failed.
  * @note 
  */
HRESULT isp_set_capture_mode(ISP_INNER_PARAM *inner_param, SENSOR_CFG_S sensor_cfg,FH_UINT32 fps)
{
	FH_VPU_SIZE vpu_size;
	rt_kprintf("=======%s %d fps is %d \n", __FUNCTION__, __LINE__,fps);
	ISP_VI_ATTR_S stViAttr;
	//MIRROR_CFG_S mirror_cfg;
	HRESULT ret;

	/**get isp public attribute */
	ret = isp_get_viu_attr(sensor_cfg, &stViAttr); 
	ret = API_ISP_SetViAttr(&stViAttr);//配置VI属性
	CHECK_RET(ret < 0, ISP_LIB_S_FAIL);
	
	vpu_size.vi_size.u32Width = stViAttr.u16PicWidth;
	vpu_size.vi_size.u32Height = stViAttr.u16PicHeight;
	ret = FH_VPSS_SetViAttr(&vpu_size);//配置VI属性
	CHECK_RET(ret < 0, ISP_LIB_S_FAIL);
	
	//暂停isp->关闭vpu->切换（配置VPU属性，复位isp）->打开vpu->恢复isp只能在dsp里面调用了。
	//1.stop isp data stream
	//ret = API_ISP_Pause();
	//CHECK_RET(ret < 0, ISP_LIB_S_FAIL);

	PRINT_INFO(DBG_KEY, g_dbg_level, "stViAttr cap mode:%d,w:%d,h:%d,fps:%d\n",\
			sensor_cfg.capture_mode, stViAttr.u16PicWidth,stViAttr.u16PicHeight, fps);
	
	gp_isp_inner_ctrl->isp_capture_mode = sensor_cfg.capture_mode;
	/**save current isp output image's width and height*/ 
	gp_isp_inner_ctrl->max_image_size.width = stViAttr.u16PicWidth;
	gp_isp_inner_ctrl->max_image_size.height = stViAttr.u16PicHeight;
	gp_isp_inner_ctrl->frame_rate = fps;
	g_fps = fps;
	// 2. set sensor driver
	/**set isp capture mode */
	gp_isp_inner_ctrl->funcs.pfn_capture_mode_cfg(fps);

	//3.set vi attr 
	/**set isp public attribute */ 
	//ret = API_ISP_SetViAttr(&stViAttr);//配置VI属性
	//CHECK_RET(ret < 0, ISP_LIB_S_FAIL);
	//sdk中修复流程，无需刷新镜像
	//mirror_cfg.bEN = gp_isp_inner_ctrl->mirror_en;
	//mirror_cfg.mirror_bayer = g_mirror_offset2fmt[stViAttr.enBayerType][OFFSET_X];
	//mirror_cfg.normal_bayer = stViAttr.enBayerType;
	//ret = API_ISP_MirrorEnable(&mirror_cfg);
	//CHECK_RET(ret < 0, ISP_LIB_S_FAIL);	
	
	ret = isp_refresh_fh_param(inner_param);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	// 4. set ae param
	/**config ae param*/ 
	ret = isp_set_ae_param(inner_param);
	CHECK_RET(ret < 0, ISP_LIB_S_FAIL);

	//刷新最大快门时间
	//ret = isp_set_shutter_slowest();
	
	//isp_refresh_awb_param(inner_param, param_val);
	//初始化时候已经初始化过AWB的KEY值了，此处只刷新awb_mode
	ret = isp_set_awb_mode(inner_param, inner_param->aaa_param.awb_param.mode);	
	CHECK_RET(ret < 0, ISP_LIB_S_FAIL);

	//
	ret = isp_refresh_isp_config(inner_param);
	CHECK_RET(ret < 0, ISP_LIB_S_FAIL);
	//5. resume isp data stream
	//ret = API_ISP_Resume();
	//CHECK_RET(ret < 0, ISP_LIB_S_FAIL);
	
	return ISP_LIB_S_OK;
}


/******************************************************************************
 * 功  能: 设置宽动态功能开关状态
 * 参  数:
 *		   inner_param	   -	 内部参数
 *
 * 返回值: 状态码
 * 备  注:
 *******************************************************************************/
HRESULT isp_set_wdr_status(ISP_INNER_PARAM *inner_param, S32 param_val)
{
	ISP_VI_ATTR_S stViAttr;
	//MIRROR_CFG_S mirror_cfg;
	SENSOR_CFG_S sensor_cfg;
	static S32 last_wdr_mode = WDR_MODE_NONE;
	HRESULT ret;
	
	if( param_val == last_wdr_mode )
	{
		PRINT_INFO(DBG_KEY, g_dbg_level, "same wdr mode, return.\n");
		return ISP_LIB_S_OK;
	}
	last_wdr_mode = param_val;

	// 注意，inner_param->img_enhance.auto_local_exp_mode存所有的宽动态模式，wd_mode不存数字宽动态
	gp_isp_inner_ctrl->wdr_mode = param_val;

	sensor_cfg.sensor_id = gp_isp_inner_ctrl->sensor_id;
	sensor_cfg.capture_mode = gp_isp_inner_ctrl->isp_capture_mode;
	sensor_cfg.wdr_mode = gp_isp_inner_ctrl->wdr_mode;

	//暂停isp->关闭vpu->切换（配置VPU属性，复位isp）->打开vpu->恢复isp只能在dsp里面调用了。
	//1.stop isp data stream
	//ret = API_ISP_Pause();
	//CHECK_RET(ret < 0, ISP_LIB_S_FAIL);

	if(PANASONIC_MN34425 == gp_isp_inner_ctrl->sensor_id)
	{
		//2.reset sensor
		/**sensor reset low*/
		ret = isp_sensor_reset();
		CHECK_RET(ret < 0, ISP_LIB_S_FAIL);
		/**delay time:10ms*/
		DELAY_MS(10);
		/**sensor reset high*/
		ret |= isp_sensor_unreset();
		CHECK_RET(ret < 0, ISP_LIB_S_FAIL);

		//3.set vi attr 
		/**set isp public attribute */ 
		gp_isp_inner_ctrl->funcs.pfn_init();
		ret = isp_get_viu_attr(sensor_cfg, &stViAttr); 
		CHECK_RET(ret != ISP_LIB_S_OK, ret);
		ret = API_ISP_SetViAttr(&stViAttr);//配置VI属性
		CHECK_RET(ret < 0, ISP_LIB_S_FAIL);

		//4.soft reset isp，软复位isp会在打开vpu之前启动isp，导致概率性崩溃，调整为配置isp属性
		//sdk中修复流程，无需在API_ISP_Resume之前获取isp属性
		//ret = API_ISP_SoftReset();
		//ret = API_ISP_Set_Attribute();
		//CHECK_RET(ret != ISP_LIB_S_OK, ret);

		//5.load isp param
		ret = isp_refresh_fh_param(inner_param);
		CHECK_RET(ret != ISP_LIB_S_OK, ret);

		//sdk中修复流程，无需刷新镜像
		//mirror_cfg.bEN = gp_isp_inner_ctrl->mirror_en;
		//mirror_cfg.mirror_bayer = g_mirror_offset2fmt[stViAttr.enBayerType][OFFSET_X];
		//mirror_cfg.normal_bayer = stViAttr.enBayerType;
		//ret = API_ISP_MirrorEnable(&mirror_cfg);
		//CHECK_RET(ret != ISP_LIB_S_OK, ret);
		
		PRINT_INFO(DBG_KEY, DBG_KEY, "wdr_mode is %d\n", param_val);
	}
	else
	{
		PRINT_INFO(DBG_KEY, g_dbg_level, "[isp_set_wdr_status] error,sensor 0x%x not support this wdr mode(val:%d).\n",gp_isp_inner_ctrl->sensor_id,param_val);
	}

	memset(pre_gamma_lut, 0, 160 * sizeof(U16));// 解决宽动态切线性后gbce不设置gama的问题

	isp_set_ae_param(inner_param);
	isp_refresh_awb_param(inner_param, param_val);
	
	isp_refresh_isp_config(inner_param);

	//6. resume isp data stream
	//ret = API_ISP_Resume();
	//CHECK_RET(ret < 0, ISP_LIB_S_FAIL);

	return ISP_LIB_S_OK;
}

HRESULT isp_set_ae_dgain(unsigned int dgain,unsigned int total_gain)
{
	ISP_AE_INFO ae_dgain_cfg;
	HRESULT ret;
	
	//PRINT_INFO(DBG_AE_REG, g_dbg_level, "isp_set_ae_dgain, dgain %d total_gain %d.\n",dgain,total_gain);
	//PRINT_INFO(DBG_KEY, g_dbg_level, "isp_set_ae_dgain, dgain %d total_gain %d.\n",dgain,total_gain);
	
	//PRINT_INFO(DBG_KEY, g_dbg_level, "exp is  %d.\n",gp_isp_inner_ctrl->aec_ctrl.procParam.aec_ret_params.aec_expose_time[0]);
	ae_dgain_cfg.stAeStatCfg.bChange = HIK_FALSE;
	ae_dgain_cfg.u32IspGain = dgain;
	ae_dgain_cfg.u32IspGainShift = 6;
	ae_dgain_cfg.u32TotalGain = total_gain;
	
	ret = API_ISP_SetAeInfo(&ae_dgain_cfg);
	CHECK_RET(ret < 0, ISP_LIB_S_FAIL);

	gp_isp_inner_ctrl->aec_ctrl.total_gain = total_gain;

	return ISP_LIB_S_OK;
}


HRESULT isp_get_ae_info(int *vmax, int *short_shutter_max, int *short_shutter_min,int *long_shutter_max,int *long_shutter_min,int *fps)
{
	 return 0;
}

HRESULT aec_set_reg_i2c(int addr ,int val)
{
	 return 0;
}

HRESULT aec_set_adg_index(S32 index)
{
	 return 0;
}

HRESULT set_dc_iris_pwm(S32 duty_cycle)
{
	return ISP_LIB_S_OK;
}

HRESULT S2_AECSetDCIris(u8 pin , unsigned short val)
{
	return 0;
}

HRESULT S2_AECSetDCIris_an41908a(unsigned short  val)
{
	return 0;
}

/* @fn      isp_get_current_db
  * @brief 获取当前的db数，用于isp相关处理
  * @param   [in] pval -当前db数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_get_current_db(U32 *pval)
{
	S32 ret_api, gain_db = 0;
	ret_api = AEC_GetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_GAIN_DB_KEY, &gain_db);
	CHECK_RET(ret_api < 0, ISP_LIB_S_FAIL);

	//如果读出来的增益异常，限制到参数范围内
	gain_db = CLIP(gain_db, 0, 66);
	*pval = (U32)gain_db;
	return ISP_LIB_S_OK;
}

/* @fn      isp_get_current_gain
  * @brief 获取当前归一到0-100的白平衡增益，用于awb/ccm的联动
  * @param   [in] pval -当前增益
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_get_current_gain(U32 *pval)
{
	S32 ret_api, gain_time = 0; //倍率
	ret_api = AEC_GetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_GAIN_KEY, &gain_time);// gain total
	CHECK_RET(ret_api < 0, ISP_LIB_S_FAIL);
	
	*pval = (U32)gain_time;
	return ISP_LIB_S_OK;
}


/* @fn      isp_get_current_long_exp
  * @brief 获取当前慢快门倍数，用于isp相关处理
  * @param   [in] pval -当前慢快门倍数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_get_current_long_exp(U32 *pval)
{
	S32 ret_api, long_exp = 1;
	ret_api = AEC_GetKeyParam(gp_isp_inner_ctrl->aec_ctrl.handle, AEC_CUR_LONG_EXPOSURE, &long_exp);
	CHECK_RET(ret_api < 0, ISP_LIB_S_FAIL);
	
	//如果读出来的倍率异常，限制到倍率范围内
	long_exp = CLIP(long_exp, 1, 16);
	*pval = (U32)long_exp;
	return ISP_LIB_S_OK;
}

/*******************************************************************************
* 功 能：获取自动宽动态的切换db阈值
* 参 数：
*        
*
* 返回值：返回状态码
*********************************************************************************/
HRESULT isp_get_auto_wdr_db_thres(U32* to_linear_db, U32* to_wdr_db )
{
	/**************************************************************************
	以185为例，线性最大66db，宽动态最大36db，假设同一照度下宽动态比线性多起12db，
	则to宽动态的阈值若定在线性状态下的12db，到宽动态时其增益大约为24db，场景再暗，
	到宽动态的30db时切换回线性，此时线性状态下的增益为18db，则可以避免震荡。

	尽量测试窗口等场景，宽动态类型不一样需要重新拟定阈值。

	**************************************************************************/
	switch(gp_isp_inner_ctrl->sensor_id)
	{
		case PANASONIC_MN34425:
			*to_wdr_db =  15;
			*to_linear_db =  30;	
			break;
		default:
			*to_wdr_db =  15;
			*to_linear_db =  30;	
			break;
	} 

	if(*to_wdr_db +9  > *to_linear_db)//最低做9db的保护
	{
		*to_wdr_db  = *to_linear_db -9;
		PRINT_INFO(DBG_KEY,g_dbg_level,"the auto wdr thres is error, please check.\n");
	}
	return ISP_LIB_S_OK;
}

/*******************************************************************************
* 功 能：获取曝光状态
* 参 数：
*         exposure_status      -曝光状态
*
* 返回值：返回状态码
*********************************************************************************/
HRESULT isp_get_auto_wdr_flag(ISP_INNER_PARAM * inner_param, S32* auto_wdr_flag)
{
	S32 ret;
	S32 exp_status;
	U32 total_db, to_linear_db, to_wdr_db;

	ret = isp_get_exp_status(&exp_status);
	ret |= isp_get_current_db(&total_db);
	ret |= isp_get_auto_wdr_db_thres(&to_linear_db, &to_wdr_db);

	PRINT_INFO(DBG_ADJUST,g_dbg_level, "exp_status:%d, total:%d, to_linear:>=%d, to_wdr:<%d\n",exp_status,total_db,to_linear_db,to_wdr_db);

	// 曝光处于稳定状态或者极限状态
	if(1 == exp_status || 2 == exp_status)
	{
		if(total_db < to_wdr_db)
		{
			*auto_wdr_flag  = ISP_AUTO_WDR;
		}
		else if(total_db >=  to_linear_db)
		{
			*auto_wdr_flag  = ISP_AUTO_LINEAR;
		}
		else
		{
			*auto_wdr_flag  = ISP_AUTO_OTHER;
		}
	}
	else
	{
		*auto_wdr_flag  = ISP_AUTO_OTHER;
	}
	
	return ISP_LIB_S_OK;
}
#endif
/* @fn      hik_awb_init
  * @brief hik awb算法库初始化
  * @param   [in] void
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT hik_awb_init(void)
{
	U32 ret = 0;
	//ISP_HLR_CFG isp_hlr_cfg = {0};
	//U32 hlrValue = 0;
	/**关闭内置的AWB、CCM算法*/
	//API_ISP_AWBAlgEn(FH_FALSE);
		
	/**设置高光恢复阈值为最大，修复回补黑电平亮度时画面成黑白*/
	//isp_hlr_cfg.bHlrEn = FH_FALSE;
	//isp_hlr_cfg.s08Level = 0x3F;
	//API_ISP_SetHlrCfg(&isp_hlr_cfg);
	/*
	FH_SYS_GetReg(0xa2ff0c68,&hlrValue);
	rt_kprintf("1111hlrValue is %d\n",hlrValue);
	FH_SYS_SetReg(0xa2ff0c68, 0xffff);
	FH_SYS_GetReg(0xa2ff0c68,&hlrValue);
	rt_kprintf("2222hlrValue is %d\n",hlrValue);
	*/
	//AWB
	/**HIK AWB内存分配与句柄创建*/
	gp_isp_inner_ctrl->awb_ctrl.createParam.cfg_data_addr = (void*)gp_isp_cfg_tables->awb_config_data_linear;
	gp_isp_inner_ctrl->awb_ctrl.createParam.awb_type      = AWB_TYPE_IPC_PCC;
	gp_isp_inner_ctrl->awb_ctrl.awb_lux = 600;//正常室内环境大概600左右
	ret = AWB_GetMemSize(&gp_isp_inner_ctrl->awb_ctrl.createParam, gp_isp_inner_ctrl->awb_ctrl.memTab);

	if (HIK_AWB_LIB_S_OK != ret)
	{
		PRINT_INFO(DBG_KEY, g_dbg_level, "hik_awb_init: AWB_GetMemSize FAILED, ret=%x\n", ret);
		return ISP_LIB_S_FAIL;
	}

	if (ISP_LIB_E_MEM_NULL == isp_alloc_mem_tab(gp_isp_inner_ctrl->awb_ctrl.memTab, HIK_AWB_MTAB_NUM))
	{
		isp_free_mem_tab(gp_isp_inner_ctrl->awb_ctrl.memTab, HIK_AWB_MTAB_NUM);
		PRINT_INFO(DBG_KEY, g_dbg_level, "hik_awb_init: AWB alloc memTab FAILED!!!\n");
		return ISP_LIB_S_FAIL;
	}
	PRINT_INFO(DBG_FUNCTION, g_dbg_level, "alloc ae mem = %d Byte\n",gp_isp_inner_ctrl->awb_ctrl.memTab[0].size);
	
	ret = AWB_Create(&gp_isp_inner_ctrl->awb_ctrl.createParam, gp_isp_inner_ctrl->awb_ctrl.memTab, &gp_isp_inner_ctrl->awb_ctrl.handle);
	if ((ret != HIK_AWB_LIB_S_OK) || (NULL == gp_isp_inner_ctrl->awb_ctrl.handle))
	{
		PRINT_INFO(DBG_KEY, g_dbg_level, "hik_awb_init: AWB_Create FAILED, ret=%x\n", ret);
		return ISP_LIB_S_FAIL;
	}

	AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_CFG_DATA_ADDR, (S32)gp_isp_cfg_tables->awb_config_data_linear);
	AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_OVER_EXPOSE_LIMIT, 128);  /*过曝阈值，默认128，R2采用210*/

	//CCM
	/**HIK CCM内存分配与句柄创建*/
	gp_isp_inner_ctrl->ccm_ctrl.procParam.cc_mat[0] = 256;
	gp_isp_inner_ctrl->ccm_ctrl.procParam.cc_mat[4] = 256;
	gp_isp_inner_ctrl->ccm_ctrl.procParam.cc_mat[8] = 256;
	/* 获取算法库内存大小&分配内存 */
	ret = CCM_GetMemSize(&gp_isp_inner_ctrl->ccm_ctrl.ccm_param, gp_isp_inner_ctrl->ccm_ctrl.mem_tab);
	if (HIK_CCM_LIB_S_OK != ret)
	{
		PRINT_INFO(DBG_KEY, g_dbg_level, "hik_awb_init: CCM_GetMemSize FAILED, ret=%x\n", ret);
		return ISP_LIB_S_FAIL;
	}
	PRINT_INFO(DBG_FUNCTION, g_dbg_level, "alloc ccm mem = %d Byte\n",gp_isp_inner_ctrl->ccm_ctrl.mem_tab[0].size);

	if (ISP_LIB_E_MEM_NULL == isp_alloc_mem_tab(gp_isp_inner_ctrl->ccm_ctrl.mem_tab, HIK_CCM_MTAB_NUM))
	{
		isp_free_mem_tab(gp_isp_inner_ctrl->ccm_ctrl.mem_tab, HIK_CCM_MTAB_NUM);
		PRINT_INFO(DBG_KEY, g_dbg_level, "hik_awb_init: CCM alloc memTab FAILED!!!\n");
		return ISP_LIB_E_MEM_NULL;
	}

	/* 创建ccm */
	gp_isp_inner_ctrl->ccm_ctrl.ccm_param.cfg_data_addr = (void*)gp_isp_cfg_tables->ccm_config_data_linear;	
	ret = CCM_Create(&gp_isp_inner_ctrl->ccm_ctrl.ccm_param, gp_isp_inner_ctrl->ccm_ctrl.mem_tab, &gp_isp_inner_ctrl->ccm_ctrl.handle);
	if (HIK_CCM_LIB_S_OK != ret)
	{
		isp_free_mem_tab(gp_isp_inner_ctrl->ccm_ctrl.mem_tab, HIK_CCM_MTAB_NUM);
		PRINT_INFO(DBG_KEY, g_dbg_level, "hik_awb_init: CCM_Create FAILED, ret=%x\n", ret);
		return ISP_LIB_S_FAIL;
	}
	
	CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_HUE_LEVEL, 50); 	/*色度，范围0~100，默认50*/

	return ISP_LIB_S_OK;
}


/* @fn      hik_awb_run
  * @brief hik awb算法库和ccm算法库执行函数
  * @param   [in] void
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT hik_awb_run(void)
{
	static int cnt = 1;
	
	IMPISPWB isp_awb_result,jz_isp_awb_ret;
	IMPISPCCMAttr isp_ccm_result;
	IMPISPAWBZone isp_awb_stats;
	unsigned int ae_gain_db = 0;
	int ret;
	unsigned char i = 0, r_avg = 0, b_avg = 0, g_avg = 0;//每一块的RGB平均统计信息
	unsigned short start_pos = 0;
	unsigned int agc = 0;
	int iValTmp1 = 0,iValTmp2 = 0,iValTmp3 = 0;
	int iVal32= 0;
	//static int j = 0;
	unsigned int awbCntR = 0, awbCntG = 0, awbCntB = 0, awbSumR = 0, awbSumG = 0, awbSumB = 0;//总的RGB统计信息，用于日夜切换判断依据
	IMPISPWB wb = {0};
	IMP_ISP_Tuning_GetWB(&wb);

	//isp_get_current_gain(&agc);
	//isp_get_current_db(&ae_gain_db);
	IMP_ISP_Tuning_GetTotalGain(&agc);
	/***********************************************TBD***********************************************************/
	//白平衡待JZ提供接口,配置手动模式、配置统计方式等
	//isp_awb_result.stAwbStatCtrl.bChange = HIK_FALSE;
	//isp_awb_result.stAwbLongStatCtrl.bChange = HIK_FALSE;
	//isp_awb_result.stAwbShortStatCtrl.bChange = HIK_FALSE;
	
	/**获取统计信息,统计信息窗口配置成16x16个*/
	memset(&isp_awb_stats, 0, sizeof(IMPISPAWBZone));

	//出来的统计信息数据基于12bit，宽动态长帧在8bit上
	IMP_ISP_Tuning_GetAwbZone(&isp_awb_stats);
	#if 0
	//打印统计信息
	printf("====================block start==================\n");
	printf("r\tg\tb\t\n");
	for(i = 0; i < AWB_ZONE_ALL; i++)
	{	
		printf("%3d\t%3d\t%3d\n",isp_awb_stats.zone_r[i],isp_awb_stats.zone_g[i],isp_awb_stats.zone_b[i]);
	}	
	//printf(" %d\n",gp_isp_inner_ctrl->aaa_param.awb_param.mode);
	printf("====================block end==================\n");
	#endif
	
	for(i = 0; i < AWB_ZONE_ALL; i++)
	{		 
		r_avg = isp_awb_stats.zone_r[i];
		g_avg = isp_awb_stats.zone_g[i];
		b_avg = isp_awb_stats.zone_b[i];

		#if 0
		if(IS_WDR_TRUE(gp_isp_inner_ctrl->wdr_mode)) //宽动态WB在globestat之前，需反除
		{
			r_avg = (r_avg * 512 / gp_isp_inner_ctrl->awb_ctrl.awb_status.rgain) << 4;
			g_avg = (g_avg * 512 / gp_isp_inner_ctrl->awb_ctrl.awb_status.ggain) << 4;
			b_avg = (b_avg * 512 / gp_isp_inner_ctrl->awb_ctrl.awb_status.bgain) << 4;
		}
		#endif
		
		gp_isp_inner_ctrl->awb_ctrl.g_stat_buf_out_arr[start_pos]= MIN(r_avg<<4, 4095);
		gp_isp_inner_ctrl->awb_ctrl.g_stat_buf_out_arr[start_pos + 1] = MIN(g_avg<<4, 4095);
		gp_isp_inner_ctrl->awb_ctrl.g_stat_buf_out_arr[start_pos + 2] = MIN(b_avg<<4, 4095);

		start_pos += 3;
		//计算256块总的RGB统计信息，用于日夜切换判断依据
		awbSumR += isp_awb_stats.zone_r[i];
		awbCntR += 1;
		
		awbSumG += isp_awb_stats.zone_g[i];
		awbCntG += 1;
		
		awbSumB += isp_awb_stats.zone_b[i];
		awbCntB += 1;
	}
	awbSumR = awbSumR<<4;
	awbSumG = awbSumG<<4;
	awbSumB = awbSumB<<4;
	gp_isp_inner_ctrl->aec_ctrl.procParam.average_y = (awbSumR+awbSumG<<1+awbSumB)/(awbCntG<<2);
	gp_isp_inner_ctrl->aec_ctrl.procParam.average_y = MIN(gp_isp_inner_ctrl->aec_ctrl.procParam.average_y,4095);
	//printf("awbCntR is %d\n",awbCntR);
	//通知算法库平均值
	gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_gstat.r_gstat = (awbSumR / MAX(awbCntR, 1));//增加除零保护，以下类似
	gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_gstat.g_gstat = (awbSumG / MAX(awbCntG, 1));
	gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_gstat.b_gstat = (awbSumB / MAX(awbCntB, 1));
	//printf("global is %d\t%d\t%d\n",gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_gstat.r_gstat,gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_gstat.g_gstat,
	//	gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_gstat.b_gstat);

	//通知算法库分块统计值
	gp_isp_inner_ctrl->awb_ctrl.procParam.block_num = AWB_ZONE_ALL;
	gp_isp_inner_ctrl->awb_ctrl.procParam.stat_data = gp_isp_inner_ctrl->awb_ctrl.g_stat_buf_out_arr;

	//gain 2 DB 
	gp_isp_inner_ctrl->awb_ctrl.procParam.ae_gain_db = ae_gain_db;
	gp_isp_inner_ctrl->awb_ctrl.procParam.env_luv = gp_isp_inner_ctrl->awb_ctrl.awb_lux;//暂时写死在hik_awb_init中为600lux
	ret = AWB_Process(gp_isp_inner_ctrl->awb_ctrl.handle, &gp_isp_inner_ctrl->awb_ctrl.procParam);  
	if(ret != HIK_AWB_LIB_S_OK)
	{
		//PRINT_INFO(DBG_KEY,g_dbg_level,"AWB_Process FAIL ret = 0x%x \n",ret);
		printf("AWB_Process FAIL \n");
		return HIK_AWB_LIB_S_FAIL;
	}

	gp_isp_inner_ctrl->ccm_ctrl.procParam.avg_lum = gp_isp_inner_ctrl->aec_ctrl.procParam.average_y;
	gp_isp_inner_ctrl->ccm_ctrl.procParam.gain_level = agc>>8;
	//printf("agc is %d\n",agc);
	gp_isp_inner_ctrl->ccm_ctrl.procParam.color_temp = gp_isp_inner_ctrl->awb_ctrl.procParam.color_temp;
	//printf("temp is %d\n",gp_isp_inner_ctrl->awb_ctrl.procParam.color_temp);

	ret = CCM_Process(gp_isp_inner_ctrl->ccm_ctrl.handle, &gp_isp_inner_ctrl->ccm_ctrl.procParam);
	if(ret != HIK_CCM_LIB_S_OK)
	{
		//PRINT_INFO(DBG_COULOR, g_dbg_level,"CCM_Process FAIL ret = 0x%x \n",ret);
		printf("CCM_Process FAIL \n");
		return HIK_CCM_LIB_S_FAIL;
	}

	for(i=0;i<9;i++)//饱和度计算
	{
		if(i<3)
		{
			iValTmp1 = ccm_cur[0] * gp_isp_inner_ctrl->ccm_ctrl.procParam.cc_mat[i%3 + 0];
			iValTmp2 = ccm_cur[1] * gp_isp_inner_ctrl->ccm_ctrl.procParam.cc_mat[i%3 + 3];
			iValTmp3 = ccm_cur[2] * gp_isp_inner_ctrl->ccm_ctrl.procParam.cc_mat[i%3 + 6];
		}
		else if(i<6)
		{
			iValTmp1 = ccm_cur[3] * gp_isp_inner_ctrl->ccm_ctrl.procParam.cc_mat[i%3 + 0];
			iValTmp2 = ccm_cur[4] * gp_isp_inner_ctrl->ccm_ctrl.procParam.cc_mat[i%3 + 3];
			iValTmp3 = ccm_cur[5] * gp_isp_inner_ctrl->ccm_ctrl.procParam.cc_mat[i%3 + 6];
		}
		else
		{
			iValTmp1 = ccm_cur[6] * gp_isp_inner_ctrl->ccm_ctrl.procParam.cc_mat[i%3 + 0];
			iValTmp2 = ccm_cur[7] * gp_isp_inner_ctrl->ccm_ctrl.procParam.cc_mat[i%3 + 3];
			iValTmp3 = ccm_cur[8] * gp_isp_inner_ctrl->ccm_ctrl.procParam.cc_mat[i%3 + 6];
		}
		iVal32 = (iValTmp1 + iValTmp2 + iValTmp3);
		iVal32 >>= 8;
		/***********************************************TBD***********************************************************/
		//CCM待JZ提供接口,配置手动模式、配置统计方式等
		isp_ccm_result.ColorMatrix[i] = (float)(iVal32) / 256.0;
		
		//CCM库的结果是用补码表示的，精度为S8.8，平台精度为S4.9
		//isp_awb_result.u16CcmCfg[i] = U16toU13(isp_awb_result.u16CcmCfg[i]);
	}
	
	/*白平衡增益channel与bayer_type映射更改至sdk，此处按照R/G/B/G顺序配置*/
	//临时添加 供系统测试,此处与UVC接口有一定的冲突
	//gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_mode = AWB_AUTO;
	if(gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_mode == AWB_AUTO)
	{
		isp_awb_result.rgain = gp_isp_inner_ctrl->awb_ctrl.procParam.r_gain ;
		isp_awb_result.bgain = gp_isp_inner_ctrl->awb_ctrl.procParam.b_gain ;
		gp_isp_inner_ctrl->awb_ctrl.awb_status.rgain = isp_awb_result.rgain;
		gp_isp_inner_ctrl->awb_ctrl.awb_status.ggain = 512;
		gp_isp_inner_ctrl->awb_ctrl.awb_status.bgain = isp_awb_result.bgain;
	}
	else if(gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_mode == AWB_MANUAL)
	{
		isp_awb_result.rgain = (gp_isp_inner_ctrl->awb_ctrl.procParam.r_gain)*MWB_LEVAL_R/512;
		isp_awb_result.bgain = (gp_isp_inner_ctrl->awb_ctrl.procParam.b_gain)*MWB_LEVAL_B/512;
		gp_isp_inner_ctrl->awb_ctrl.awb_status.rgain = (gp_isp_inner_ctrl->awb_ctrl.procParam.r_gain)*MWB_LEVAL_R/512;
		gp_isp_inner_ctrl->awb_ctrl.awb_status.ggain = 512;
		gp_isp_inner_ctrl->awb_ctrl.awb_status.bgain = (gp_isp_inner_ctrl->awb_ctrl.procParam.b_gain)*MWB_LEVAL_B/512;
	}
	else//锁定白平衡
	{
		isp_awb_result.rgain = gp_isp_inner_ctrl->awb_ctrl.awb_status.rgain;
		gp_isp_inner_ctrl->awb_ctrl.awb_status.ggain = 512;
		isp_awb_result.bgain = gp_isp_inner_ctrl->awb_ctrl.awb_status.bgain;
	}

	//TBD
	/*回补黑电平导致的亮度损失*/
	/*计算方式为:4096/(4096-256) = 1.07,其中256为目前所有黑电平最大设置的值*/
	//isp_awb_result.rgain = isp_awb_result.rgain * 107/100;
	//isp_awb_result.bgain = isp_awb_result.bgain * 107/100;

	isp_awb_result.rgain = isp_awb_result.rgain >> 1;
	isp_awb_result.bgain = isp_awb_result.bgain >> 1;

	isp_ccm_result.ManualEn = IMPISP_TUNING_OPS_MODE_ENABLE;
	isp_ccm_result.SatEn = IMPISP_TUNING_OPS_MODE_ENABLE;
	//isp_awb_result.mode = ISP_CORE_WB_MODE_AUTO;
	/*保存白平衡配置，由sensor实现镜像，更改bayer_type时使用*/
	memcpy(&gp_isp_inner_ctrl->awb_cfg, &isp_awb_result, sizeof(IMPISPWB));
	if(0 == g_aaa_param.awb_param.auto_enable)//auto
	{
		isp_awb_result.mode = ISP_CORE_WB_MODE_MANUAL;
		ret = IMP_ISP_Tuning_SetWB(&isp_awb_result);		
		ret |= IMP_ISP_Tuning_SetCCMAttr(&isp_ccm_result);
	}
	else//manual
	{
		//printf("-------------------manual awb run---------------------\n");
		//考虑到下发参数的时候ccm未生效，导致手动awb并重启后饱和度淡的问题，所以在此处获取手动的值并配合ccm一起刷新
		//isp_awb_result.rgain = (float)g_aaa_param.awb_param.manual_wb_gain.r_gain / g_aaa_param.awb_param.manual_wb_gain.g_gain;
		//isp_awb_result.bgain = (float)g_aaa_param.awb_param.manual_wb_gain.b_gain / g_aaa_param.awb_param.manual_wb_gain.g_gain;
		isp_awb_result.rgain = wb.rgain;
		isp_awb_result.bgain = wb.bgain;
		ret = IMP_ISP_Tuning_SetWB(&isp_awb_result);
		//ret = IMP_ISP_Tuning_SetWB(&isp_awb_result);
		ret |= IMP_ISP_Tuning_SetCCMAttr(&isp_ccm_result);
	}
	cnt ++;
	return ISP_LIB_S_OK;
}


//awb config
/* @fn      isp_stop_awb
  * @brief awb停止
  * @param   [in] inner_param -内部参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_stop_awb(ISP_INNER_PARAM *inner_param)
{
	gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_mode = AWB_LOCK;
	return ISP_LIB_S_OK;
}


/* @fn      isp_start_awb
  * @brief awb开启
  * @param   [in] inner_param -内部参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_start_awb(ISP_INNER_PARAM *inner_param)
{
	//awb_mode保存的是白平衡增益生效的时候的具体模式
	if(HIK_WB_MANUAL_MODE == inner_param->aaa_param.awb_param.mode)//手动白平衡
	{
		gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_mode = AWB_MANUAL;
	}
	else
	{
		gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_mode = AWB_AUTO;
	}
	return ISP_LIB_S_OK;
}

/******************************************************************************
* 功  能: awb开启
* 参  数:
*         inner_param     -     内部参数
*
* 返回值: 状态码
* 备  注:
*******************************************************************************/
HRESULT isp_save_awb_mode(ISP_INNER_PARAM *inner_param, S32 param_val)
{
	//awb_mode保存的是白平衡增益生效的时候的具体模式
	if(HIK_WB_MANUAL_MODE == param_val)//手动白平衡
	{
		gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_mode = AWB_MANUAL;
	}
	else
	{
		gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_mode = AWB_AUTO;
	}
	return ISP_LIB_S_OK;
}

/* @fn      isp_set_awb_speed
  * @brief 平台awb速度控制
  * @param   [in] inner_param -内部参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_set_awb_speed(ISP_INNER_PARAM *inner_param, S32 param_val)
{
	AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle,AWB_SMOOTH_RATIO, param_val);
	return ISP_LIB_S_OK;
}


/******************************************************************************
 * 功  能: 配置awb参数(色温适应范围比率)
 * 参  数:
 *		   inner_param	   -	 内部参数
 *
 * 返回值: 状态码
 * 备  注:
 *******************************************************************************/
HRESULT isp_cfg_awb_params(ISP_INNER_PARAM *inner_param, S32 param_val)
{
	S32 ret = 0;

	/*AWB参数配置*/
	// 2. 配置高增益下，切换到全局白平衡的上下限增益
	switch(gp_isp_inner_ctrl->sensor_id)
	{
		case PANASONIC_MN34425:
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_LOW_LIGHT_START_DB, 48);
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_LOW_LIGHT_END_DB, 54);
			break;
			#if 0
		case OMNI_OV2735:
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_LOW_LIGHT_START_DB, 42);
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_LOW_LIGHT_END_DB, 48);
			break;
			#endif
		case SOI_K02:
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_LOW_LIGHT_START_DB, 42);
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_LOW_LIGHT_END_DB, 48);
			break;
			#if 0
		case OMNI_SP2309:
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_LOW_LIGHT_START_DB, 42);
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_LOW_LIGHT_END_DB, 48);
			break;
			#endif
		default:
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_LOW_LIGHT_START_DB, 48);
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_LOW_LIGHT_END_DB, 54);
			break;
	}

	CHECK_RET(ret != HIK_AWB_LIB_S_OK, ISP_LIB_S_FAIL);

	return ISP_LIB_S_OK;
}

/* @fn      isp_set_awb_mode
  * @brief awb模式控制
  * @param   [in] inner_param -内部参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */	
HRESULT isp_set_awb_enable(S32 param_val)
{
	//rt_kprintf("isp_set_awb_enable is %d\n",param_val);
	if(0 == param_val)
	{
		g_aaa_param.awb_param.auto_enable = param_val;//auto
	}
	else
	{
		g_aaa_param.awb_param.auto_enable = param_val;//manual
	}
	return 0;
}

/* @fn      isp_set_mwb_param
  * @brief awb模式控制
  * @param   [in] inner_param -内部参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */	
HRESULT isp_set_mwb_param(S32 r_gain,S32 g_gain,S32 b_gain)
{
	//rt_kprintf("isp_set_awb_enable is %d\n",param_val);
	g_aaa_param.awb_param.manual_wb_gain.r_gain = r_gain; 
	g_aaa_param.awb_param.manual_wb_gain.g_gain = g_gain;
	g_aaa_param.awb_param.manual_wb_gain.b_gain = b_gain;
	return 0;
}

/* @fn      isp_set_awb_mode
  * @brief awb模式控制
  * @param   [in] inner_param -内部参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_set_awb_mode(ISP_INNER_PARAM *inner_param, S32 param_val)
{
	U32 ret = ISP_LIB_S_OK;
	U08 *awb_cfg_addr;
	U16 *awb_cen_addr;
	U08 tab_index = 0, awb3_en = 0;
	U08 color_priority = 0,nature_mode = 0,pure_mode = 0;
	U08 sat_ratio = 0, sat_weight = 0;
	
	if ( IS_WDR_NONE(inner_param->img_enhance.auto_local_exp_mode) ) //线性模式
	{
		awb_cfg_addr = gp_isp_cfg_tables->awb_config_data_linear;
	}
	else	// 宽动态
	{
		awb_cfg_addr = gp_isp_cfg_tables->awb_config_data_wdr;
	}

	// awb配置文件内部关系: 0-自动白平衡 1-钠灯/白炽灯 2-自动白平衡1 3-日光灯 4-自然光/室外 5-暖光灯/室内
	switch (param_val)
	{
		case HIK_WB_LOCK_MODE:			//锁定白平衡
			isp_stop_awb(inner_param);
			break;
		case HIK_WB_MANUAL_MODE:		//手动白平衡(全色域+R/Bgain offset)
			tab_index = 1;
			awb3_en = 0;
			color_priority = 0;
			nature_mode = 0;
			pure_mode = 0;
			sat_ratio = 255;
			sat_weight = 1;
			break;
		case HIK_WB_AWB1_MODE:			// 自动白平衡1(通用awb3.0)
		case HIK_WB_AUTO_TRACK_MODE:	// 自动跟踪
			tab_index = 2;
			awb3_en = 1;
			color_priority = 0;
			nature_mode = 0;
			pure_mode = 0;
			sat_ratio = 0;
			sat_weight = 16;
			break;
		case HIK_WB_FLUORESCENT_MODE:	// 日光灯(新自然光模式)
			tab_index = 2;
			awb3_en = 1;
			color_priority = 0;
			nature_mode = 1;
			pure_mode = 0;
			sat_ratio = 0;
			sat_weight = 16;
			break;
		case HIK_WB_NATURAL_MODE:		// 自然光(awb2.0)
		case HIK_WB_OUTDOOR_MODE:		// 室外模式
			tab_index = 2;
			awb3_en = 0;
			color_priority = 0;
			nature_mode = 0;
			pure_mode = 0;
			sat_ratio = 0;
			sat_weight = 16;
			break;
		case HIK_WB_WARM_MODE:			// 暖光灯(黄色优先)
		case HIK_WB_INDOOR_MODE:		// 室内模式
			tab_index = 2;
			awb3_en = 1;
			color_priority = 1;
			nature_mode = 0;
			pure_mode = 0;
			sat_ratio = 0;
			sat_weight = 16;
			break;
		case HIK_WB_INCANDESCENT_MODE:	// 白炽灯(全色域)
		case HIK_WB_SODIUM_MODE:		// 钠灯
			tab_index = 1;
			awb3_en = 0;
			color_priority = 0;
			nature_mode = 0;
			pure_mode = 0;
			sat_ratio = 255;
			sat_weight = 1;
			break;
		case HIK_WB_AWB2_MODE:		// 自动白平衡2(窄色域)
		case HIK_WB_AUTO_MODE:		// 自动白平衡
		default:
			// TODO : YYL
#if(CAMERA_MODEL == TV22_2K809 || CAMERA_MODEL == TV24_2K810 || CAMERA_MODEL == E12A_2K822 || CAMERA_MODEL == E12A_2K823 || CAMERA_MODEL == E12A_2K819 || CAMERA_MODEL == E12A_2K820)
			tab_index = 0;
#else
			tab_index = 3;
#endif
			awb3_en = 1;
			color_priority = 0;
			nature_mode = 0;
			pure_mode = 0;
			sat_ratio = 0;
			sat_weight = 16;
			break;		
	}
	if (HIK_WB_LOCK_MODE != param_val)	// 非锁定模式
	{
		isp_save_awb_mode(inner_param,param_val);

		ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle,AWB_KEY_CFG_DATA_ADDR, (S32)(awb_cfg_addr + tab_index * HIK_AWB_CFG_DATA_LEN));	
		awb_cen_addr = (U16 *)(awb_cfg_addr + 15 * HIK_AWB_CFG_DATA_LEN + 6);
		ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_CEN_DATA_ADDR, (S32)awb_cen_addr);
		
		ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_AREA_INDEX, tab_index);
		
		if (HIK_WB_INDOOR_MODE == param_val)
		{
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_TOO_DARK_LIMIT, 4);
		}
		else
		{
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_TOO_DARK_LIMIT, 3);
		}
		
		if( WDR_MODE_2To1_LINE == inner_param->img_enhance.auto_local_exp_mode )
		{
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_OVER_EXPOSE_LIMIT, 128);//根据田工建议DOL宽动态用128
		}
		else
		{
			ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_OVER_EXPOSE_LIMIT, 128);//其他用200
		}
		
		ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_K_GAIN_RATIO, 128);
		
		//配置awb低照度偏色的起始db
		isp_cfg_awb_params(inner_param, param_val);
		
		ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_COLOR_PRIORITY, color_priority);
		
		ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_NATURE_MODE_ENABLE, nature_mode);
		
		ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_ENABLE_PURE, pure_mode);
		
		ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_ENABLE_AWB3, awb3_en);
		
		ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_PCC_SAT_RATIO, sat_ratio);

		ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_AUTO_SAT_MAX_WEIGHT, sat_weight);
		
		ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_LEAVES_HOLD_DIS, 75); //调整树叶保持直线距离，解决ar0237特定灯源偏绿

		ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_AUTO_SCALE_WEIGHT_ENABLE, 1); //自动权重进行缩放使能

		//ret |= AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_KEY_CLUSTER_CLOSE_ENABLE, 1); //黄/蓝色优先模式关闭聚类，解决正常场景震荡

		CHECK_RET(ret != HIK_AWB_LIB_S_OK, ISP_LIB_S_FAIL);
	}
	gp_isp_inner_ctrl->awb_ctrl.procParam.max_gain_option = AWB_MAX_GAIN_X8;
	
	return ISP_LIB_S_OK;
}


/* @fn      isp_set_rgb_gain
  * @brief 设置awb RGB增益
  * @param   [in] inner_param -内部参数, wb_gain -rgb增益结构体
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_set_rgb_gain(ISP_INNER_PARAM *inner_param, HIK_WB_GAIN wb_gain)
{
	//MWB
	if(HIK_WB_MANUAL_MODE == inner_param->aaa_param.awb_param.mode)
	{
		MWB_LEVAL_R = (U16)(wb_gain.r_gain);
		MWB_LEVAL_B = (U16)(wb_gain.b_gain);
		//gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_mode = AWB_MANUAL;//此处不再赋值，白平衡模式放在start和stop函数中
	}
	//locked wb
	else
	{
		gp_isp_inner_ctrl->awb_ctrl.awb_status.rgain = (U16)(wb_gain.r_gain);
		gp_isp_inner_ctrl->awb_ctrl.awb_status.ggain = (U16)(wb_gain.g_gain);
		gp_isp_inner_ctrl->awb_ctrl.awb_status.bgain = (U16)(wb_gain.b_gain);
		//gp_isp_inner_ctrl->awb_ctrl.awb_status.awb_mode = AWB_LOCK;
	}

	return ISP_LIB_S_OK;
}


/* @fn      isp_get_rgb_gain
  * @brief 获取awb rgb分量增益值
  * @param   [in] wb_gain -rgb增益结果
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_get_rgb_gain(HIK_WB_GAIN *wb_gain)
{
	wb_gain->r_gain = gp_isp_inner_ctrl->awb_ctrl.awb_status.rgain;
	wb_gain->g_gain = gp_isp_inner_ctrl->awb_ctrl.awb_status.ggain;
	wb_gain->b_gain = gp_isp_inner_ctrl->awb_ctrl.awb_status.bgain;
	return ISP_LIB_S_OK;
}

/******************************************************************************
* 功  能: 真宽动态与线性切换刷新awb的内部参数
* 参  数:
*         inner_param - 内部参数
*		   param_val   - 宽动态的等级
*
* 返回值: 无
* 备  注:
******************************************************************************/
HRESULT isp_refresh_awb_param(ISP_INNER_PARAM *inner_param, S32 param_val)
{
	// 刷新ccm参数
	if(IS_WDR_NONE(param_val))//线性
	{	
		PRINT_INFO(DBG_KEY,g_dbg_level,"isp_refresh_awb_param:liner\n");
		CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_CFG_DATA_ADDR, (S32)gp_isp_cfg_tables->ccm_config_data_linear);		
		CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_CT_SWITCH, 1); /*色温联动，0、1，默认1*/
		switch(gp_isp_inner_ctrl->sensor_id)
		{
			case PANASONIC_MN34425:
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_YRA_RATIO_GAIN_MIN, 20);//与E3效果一致，ccm库默认值20
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_YRA_RATIO_GAIN_MAX, 80);
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_CNR_LEVEL, 50);
				break;
			case OMNI_OS02D20:
			case OMNI_OS04C10:
			case SOI_JXF37:
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_YRA_RATIO_GAIN_MIN, 20);//与E3效果一致，ccm库默认值20
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_YRA_RATIO_GAIN_MAX, 80);
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_CNR_LEVEL, 50);
				break;
			case SOI_K02:
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_YRA_RATIO_GAIN_MIN, 30);
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_YRA_RATIO_GAIN_MAX, 80);
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_CNR_LEVEL, 75);
				break;
			default:
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_YRA_RATIO_GAIN_MIN, 50);
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_YRA_RATIO_GAIN_MAX, 80);
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_CNR_LEVEL, 75);// 线性下色彩抑制统一为75
				break;
		}
	}
	else//宽动态
	{
		PRINT_INFO(DBG_KEY,g_dbg_level,"isp_refresh_awb_param:wdr\n");
		CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_CFG_DATA_ADDR, (S32)gp_isp_cfg_tables->ccm_config_data_wdr);
		CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_CT_SWITCH, 1);
		switch(gp_isp_inner_ctrl->sensor_id)
		{
			case PANASONIC_MN34425:
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_YRA_RATIO_GAIN_MIN, 20);//与E3效果一致，ccm库默认值20
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_YRA_RATIO_GAIN_MAX, 80);
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_CNR_LEVEL, 30);
				break;
			default:
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_YRA_RATIO_GAIN_MIN, 50);
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_YRA_RATIO_GAIN_MAX, 80);
				CCM_SetKeyParam(gp_isp_inner_ctrl->ccm_ctrl.handle, CCM_KEY_CNR_LEVEL, 30);
				break;
		}
	}

	// 刷新白平衡参数
	isp_set_awb_mode(inner_param, inner_param->aaa_param.awb_param.mode);

	return ISP_LIB_S_OK;
}

/******************************************************************************
 * 功  能: 设置白平衡库的打印等级
 * 参  数:
 *         inner_param - 内部参数
 *
 * 返回值: 状态码
 * 备  注:
 *******************************************************************************/
HRESULT isp_set_awb_print(ISP_INNER_PARAM *inner_param)
{
	if(inner_param->print_en & DBG_COULOR)//打印awb
	{
		AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_DEBUG_PIN, 1);
	}
	else
	{
		AWB_SetKeyParam(gp_isp_inner_ctrl->awb_ctrl.handle, AWB_DEBUG_PIN, 0);
	}
	return ISP_LIB_S_OK;
}

/*******************************************************************************
 * 功 能：isp_gam_interface,
 * 参 数：
 *	gamma	君正的gamma指针，
 *	alpha	gamma的权重值0-255
 *
 * 返回值：返回状态码
 *********************************************************************************/
HRESULT isp_gam_interface(IMPISPGamma *gamma,U32 alpha)
{
	IMPISPGamma *p_gamma = gamma;
	U32 weight = alpha;
	U32 i = 0;
	CHECK_RET(NULL == p_gamma, ISP_LIB_E_PARA_NULL);
	if(weight>255)
	{
		weight = 255;
	}

	if(weight <= 128)
	{
		for(i = 0;i<129;i++)
		{
			p_gamma->gamma[i] = (g_gam_low.gamma[i]*(128-weight)+g_gam_default.gamma[i]*weight)>>7;
		}
	}
	else if(weight > 192)
	{
		weight = weight - 192;
		for(i = 0;i<129;i++)
		{
			p_gamma->gamma[i] = (g_gam_mid.gamma[i]*(64-weight)+g_gam_high.gamma[i]*weight)>>6;
		}
	}
	else
	{
		weight = weight - 128;
		for(i = 0;i<129;i++)
		{
			p_gamma->gamma[i] = (g_gam_default.gamma[i]*(64-weight)+g_gam_mid.gamma[i]*weight)>>6;
		}
	}
	return ISP_LIB_S_OK;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

