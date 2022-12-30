/** @file isp_struct_type.h
*	@note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*	@brief 主要定义各类用户态结构体、宏、枚举。
*	
*	@author   yaojinbo
*	@date	  2015-05-26
*	@version  1.0
*	@note 只定义图像参数相关结构体、宏，其他请放在isp_struct_type.h中
*	@note History:		  
*	@note	  <author>	 <time>    <version >	<desc>
*	@note	  yaojinbo	  2015-05-28  修改
*	@warning  
*/

#ifndef __ISP_STRUCT_TYPE_H__
#define __ISP_STRUCT_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_platform.h"
#include "isp_interface.h"
#include "isp_3a.h"
#include "sensor.h"


#define PARAM_LEVEL_NUMS				11

#define AWB_CONFIG_DATA_NUM				65346

#define CCM_CONFIG_DATA_NUM				1114

#define HIK_GBCE_CURVE_TBL_SIZE        (2 * 40 * 256)   // 表大小2*20*256

#define CFG_NAME_LEN					64


/** 图像尺寸结构体*/
typedef struct _MAX_IMAGE_SIZE_
{
	S32	 height;
	S32	 width;
}MAX_IMAGE_SIZE;

typedef struct _HIK_ISP_FUNC_
{
	S32	(*pfn_init)(VOID);
	S32	(*pfn_set_gain)(U32 gain);
	S32	(*pfn_get_gain)(U32 *gain);
	S32	(*pfn_set_intt)(U32 intt);
	S32	(*pfn_get_intt)(U32 *intt);
	S32	(*pfn_set_sns_fmt)(int format);
    VOID(*pfn_capture_mode_cfg)(U32 fps);
	S32	(*pfn_set_exposure_ratio)(U32 exposure_ratio);
	S32	(*pfn_get_exposure_ratio)(U32 *exposure_ratio);
	S32	(*pfn_get_sensor_attribute)(char *name, U32 *value);
} HIK_ISP_FUNC;

/**存放3A RUN以及sensor操作所需要的变量*/
typedef struct _ISP_INNER_CTRL_S_
{
	VOID				*mem_start;		/**<缓存起始位置*/
	ISP_VI_MODE_E		sensor_id;
	ISP_E1_CAPTURE_MODE	isp_capture_mode;
	U16					frame_rate;
	ISP_WDR_MODE_E		wdr_mode;
	S32					mirror_en;
	MAX_IMAGE_SIZE		max_image_size;
	ISP_AEC_CTRL		aec_ctrl;
	AWB_CTRL			awb_ctrl;
	CCM_CTRL			ccm_ctrl;
	BUS_ATTR			bus_attr;
	HIK_ISP_FUNC		funcs;
	//ISP_AWB_CFG			awb_cfg;
	IMPISPWB			awb_cfg;
	GBCE_CTRL			gbceCtrl;//GBCE Struct Params
	S32					manual_blc_en;
}ISP_INNER_CTRL;

#ifdef __cplusplus
}
#endif

#endif /* __ISP_STRUCT_TYPE_H__ */

