/** @file isp_img.h
*	@note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*	@brief ispģ����ͼ��Ч�����ͷ�ļ�
*	
*	@author   yaojinbo
*	@date	  2015-05-26
*	@version  1.0
*	@note ֻ����ͼ�������ؽṹ�塢�꣬���������isp_struct_type.h��
*	@note History:		  
*	@note	  <author>	 <time>    <version >	<desc>
*	@note	  yaojinbo	  2015-05-28  �޸�
*	@warning  
*/

#ifndef __ISP_IMG_H__
#define __ISP_IMG_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include "isp_inner.h"
#include "isp_struct_type.h"
//��������ֵ[0,100]ӳ�䵽�ڲ�[0,10]--11���ȼ�
#define MAP_PARAM(val)  				((val)/10)

//map ǰ�˲���Ĭ��ֵ
#define MAP_FRONT_DEFAULT_PARAM(val,default_value)			(((val)<50)?(val)*(default_value)/50:(((val)-50)*(100-(default_value))/50+(default_value))) 

#define SENSOR_PAGE_OV2735		0xfd
#define SENSOR_PAGE_1_OV2735	0x1
#define MIRROR_ADDR_OV2735		0x3F
#define BLC_CTRL_OV2735			0xF8

#define GAIN_NODES  (12)
typedef enum _ISP_MIRROR_MODE_
{
	MIRROR_OFF        = 0, 		/* �����þ��� */
	MIRROR_HORIZONTAL = 1,      /* ���Ҿ��� */
	MIRROR_VERTICAL   = 2, 	    /* ���¾��� */
	MIRROR_CENTER     = 3 		/* ���ľ��� */
}ISP_MIRROR_MODE;

typedef enum _ISP_MIRROR_OFFSET_
{
	OFFSET_OFF	= 0, 		/* ��ƫ�� */
	OFFSET_X 	= 1,      	/* ˮƽ����ƫ�� */
	OFFSET_Y   	= 2, 	    /* ��ֱ����ƫ�� */
	OFFSET_ALL  = 3 		/* ˮƽ����ֱ�����ƫ�� */
}ISP_MIRROR_OFFSET;

/***************************** �����ı��־λ ****************************************/
//one bit represents one param
typedef enum
{
	PARAMS_NO_CHANGED			= 0x00000000,
	PARAM_LTM_CHANGED			= 0x00000001,
	_PARAM_LTM_CHANGED			= 0xFFFFFFFE,
}ISP_PARAMS_CHANGED;

typedef struct ISP_SHARPENESS_PARAM_S
{
	FH_UINT8 u08NegativeStr[GAIN_NODES];//APCNGain
	FH_UINT8 u08PositiveGain[GAIN_NODES];//APCPGain
}ISP_SHARPENESS_PARAM;

typedef struct ISP_CONTRAST_PARAM_S
{
	FH_UINT8 u08CrtMap[GAIN_NODES];
}ISP_CONTRAST_PARAM;

typedef struct ISP_NR2D_PARAM_S
{
	FH_UINT8 u8BlendingVal;
	FH_UINT8 u08StrMap[GAIN_NODES];
}ISP_NR2D_PARAM;

typedef struct ISP_NR3D_PARAM_S
{
	FH_UINT8 u08StrMap[GAIN_NODES];
	FH_UINT8 u08CoeffOffsetMap[GAIN_NODES];
	FH_UINT8 u08MtStrMap[GAIN_NODES];
	FH_UINT8 u08Nr3dSftMap[GAIN_NODES];
}ISP_NR3D_PARAM;

typedef struct ISP_YNR_PARAM_S
{
	FH_SINT8 s08YnrThSlope0[GAIN_NODES];
	FH_UINT8 u08YnrThOffset0[GAIN_NODES];
}ISP_YNR_PARAM;

typedef struct ISP_GAMMA_PARAM_S
{
	FH_UINT16 u16CGamma[160];
}ISP_GAMMA_PARAM;

typedef struct ISP_LTM_PARAM_S
{
	FH_UINT32 u32TonemapCurve[128];
	FH_UINT8 u08NlYScalerMap[GAIN_NODES];
	FH_UINT8 u08LtmWeight[GAIN_NODES];
}ISP_LTM_PARAM;

typedef struct ISP_VPSS_PARAM_S
{
	FH_UINT8 u08VpssScalerMap[GAIN_NODES];
}ISP_VPSS_PARAM;

typedef struct ISP_AF_SET_AFD_S{
         //ISP_AF_FILTER af_filter;
         //ISP_AF_WIN_INFO af_win_info;
}ISP_AF_STAT_PARAM;

typedef struct ISP_FISHEYE_PARAM_S
{
	int left_set;		//��Ϊ��������Ϊ��
	int top_set;		//��Ϊ����,��Ϊ��
	int left_status;	//��Ϊ��������Ϊ��
	int top_status;		//��Ϊ����,��Ϊ��
	int radius;			//���۰뾶
}ISP_FISHEYE_PARAM;

/**���ISPͼ��������ñ�*/
typedef struct ISP_CFG_TABLES_S
{
	ISP_GAMMA_PARAM				*gamma_table_param_linear;
	ISP_GAMMA_PARAM				*gamma_table_param_wdr;
	ISP_LTM_PARAM				*ltm_table_param_linear;
	ISP_LTM_PARAM				*ltm_table_param_wdr;
//	ISP_LENS_SHADING_PARAM*	lens_shading_param;
	ISP_SHARPENESS_PARAM        *sharpeness_param_linear;
	ISP_SHARPENESS_PARAM        *sharpeness_param_wdr;
	ISP_CONTRAST_PARAM          *contrast_param_linear;
	ISP_CONTRAST_PARAM          *contrast_param_wdr;
	ISP_NR3D_PARAM              *nr3d_param_linear;
	ISP_NR3D_PARAM              *nr3d_param_wdr;
	ISP_NR2D_PARAM           	*nr2d_param_linear;
	ISP_NR2D_PARAM           	*nr2d_param_wdr;
	ISP_YNR_PARAM				*ynr_param_linear;
	ISP_YNR_PARAM				*ynr_param_wdr;
	//ISP_GB_CFG					*gb_param_linear;
	//ISP_LC_CFG					*lc_param_linear;
	//ISP_LC_CFG					*lc_param_wdr;
	ISP_VPSS_PARAM				*vpss_param_linear;
	ISP_VPSS_PARAM				*vpss_param_wdr;
	//AE_DEFAULT_CFG				*ae_param_linear;
	//AE_DEFAULT_CFG				*ae_param_wdr;
	//AWB_DEFAULT_CFG				*awb_param_linear;
	//AWB_DEFAULT_CFG				*awb_param_wdr;
	U08							awb_config_data_linear[AWB_CONFIG_DATA_NUM];
	U08							awb_config_data_wdr[AWB_CONFIG_DATA_NUM];
	U08							ccm_config_data_linear[CCM_CONFIG_DATA_NUM];
	U08							ccm_config_data_wdr[CCM_CONFIG_DATA_NUM];
	U08							curve_tab_addr[HIK_GBCE_CURVE_TBL_SIZE];
	S08							awb_line_config[CFG_NAME_LEN];
	S08							awb_wdr_config[CFG_NAME_LEN];
	S08							ccm_line_config[CFG_NAME_LEN];
	S08							ccm_wdr_config[CFG_NAME_LEN];
	U08							*hist_addr[256 * 4];
	U08							*gamma_lut_addr[1024 * 2];
	U08							*gbce_lut_addr[1024 * 2];
	U16 						gbce_ac_curve_tab[HIK_GBCE_CURVE_TBL_SIZE];
}ISP_CFG_TABLES;


#ifdef __cplusplus
}
#endif

#endif /* __ISP_IMG_H__ */

