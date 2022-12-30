/** @file isp_3a.h
*  @note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*  @brief   isp3Aģ�����ͷ�ļ�
*  
*  @author   yaojinbo
*  @date	   2015-05-26
*  @version  1.0
*  @note 3a.h�ļ�ֻ����3a��ؽṹ�塢�꣬���������isp_struct_type.h��
*  @note History: 	   
*  @note	   <author>   <time>	<version >	 <desc>
*  @note	  yaojinbo	  2015-05-28  �޸�
*  @warning  
*/


#ifndef __ISP_3A_H__
#define __ISP_3A_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_platform.h"
#include "isp_interface.h"
#include "awb_lib.h"
#include "ccm_lib.h"
#include "gbce_lib.h"
#include "ae_lib.h"
//#include "hal_hikio.h"


#define AE_ZONE_ALL				(256)
#define AE_ZONE_ROW				(16)
#define AE_ZONE_COLUMN			(16)


#define AWB_ZONE_ALL			(225)
#define AWB_ZONE_ROW			(15)
#define AWB_ZONE_COLUMN			(15)

//Smart IR Ȩ�����ֵ
#define	HIST_WEIGHT_MAX			 32

typedef struct isp_ae_weight_table_
{
	U8 u8Weight[AE_ZONE_ALL];
}isp_ae_weight_table_t;


/********************** HIK ��ƽ��������ݽṹ����**********************/
/**hik����İ�ƽ��ģʽ ��ͻ��˴����ֵһһ��Ӧ*/
typedef enum
{
	HIK_WB_MANUAL_MODE 			= 0,/**< 0-�ֶ���ƽ��(MWB)*/ 
	HIK_WB_AWB1_MODE			= 1,/**<1-�Զ���ƽ��1(AWB1)*/
	HIK_WB_AWB2_MODE			= 2,/**<2-�Զ���ƽ��2(AWB2)*/
	HIK_WB_LOCK_MODE			= 3,/**<3-������ƽ��(Locked WB)*/
	HIK_WB_INDOOR_MODE 			= 4,/**<4-����(Indoor)*/
	HIK_WB_OUTDOOR_MODE			= 5,/**<5-����(Outdoor)*/
	HIK_WB_FLUORESCENT_MODE	 	= 6,/**<6-�չ��(Fluorescent Lamp)*/
	HIK_WB_SODIUM_MODE 		 	= 7,/**<7-�Ƶ�ģʽ(Sodium Lamp)*/
	HIK_WB_AUTO_TRACK_MODE 	 	= 8,/**<8-�Զ����٣�Auto-Track*/
	HIK_WB_ONE_PUSH_MODE		= 9,/**<9-һ�ΰ�ƽ�⣨One Push*/
	HIK_WB_AUTO_OUTDOOR_MODE	= 10,/**<10-�����Զ���Auto-Outdoor*/
	HIK_WB_AUTO_SODIUM_MODE	 	= 11,/**<11-�Ƶ��Զ�(Auto-Sodiumlight)*/
	HIK_WB_MERCURY_MODE			= 12,/**<12-ˮ����ģʽ(Mercury Lamp),*/
	HIK_WB_AUTO_MODE			= 13,/**<13-�Զ���ƽ��(Auto)*/
	HIK_WB_INCANDESCENT_MODE	= 14,/**<14-�׳�ƣ���Incandescent Lamp*/
	HIK_WB_WARM_MODE			= 15,/**<15-ů��ƣ���Warm Light Lamp*/
	HIK_WB_NATURAL_MODE			= 16,/**<16-��Ȼ�⣬��Natural Light*/
	HIK_WB_MODE_NUMBER 			= 17,/**<17-�Զ���ƽ��(Auto)*/
} HIK_WB_MODE;

/**��Ӧ�ò㽻����AWBģʽ*/
typedef enum 
{
	AWB_AUTO = 0,
	AWB_LOCK,
	AWB_MANUAL,
}HIK_AWB_MODE;

/**WDRģʽ����ͬ��WDRģʽ���ò�ͬ��AWB����*/
typedef enum
{
	AWB_LINE = 0,/**<����ģʽ�µ�AWB*/
	AWB_WDR,/**<��̬ģʽ�µ�AWB*/
}HIK_DATA_MODE;

/**AWB��������*/
typedef struct _HIK_WB_GAIN_ 
{
	u32 r_gain;
	u32 g_gain;
	u32 b_gain;
} HIK_WB_GAIN;

/**AWBͳ����Ϣ����*/
typedef struct _ISP_AWB_GSTAT_ 
{
	u32 r_gstat;
	u32 g_gstat;
	u32 b_gstat;
} ISP_AWB_GSTAT;

typedef struct ISP_AWB_STATUS_S
{
	HIK_AWB_MODE	awb_mode;
	HIK_DATA_MODE	data_mode;
	U16 			rgain;
	U16 			ggain;
	U16 			bgain;
	ISP_AWB_GSTAT awb_gstat;
} AWB_STATUS;

/**��ƽ����ز���*/
typedef struct  _HIK_AWB_PARAM_STRU_
{
	BOOL			auto_enable;
	HIK_WB_MODE		mode;	 
	S32				speed;
	HIK_WB_GAIN		manual_wb_gain;
}HIK_AWB_PARAM;

/**AWB ���ƽṹ�嶨��*/
typedef struct ISP_AWB_CTRL_S
{
	VOID *handle;
	MEM_TAB memTab[HIK_AWB_MTAB_NUM];
	AWB_PARAM createParam;
	AWB_PROCESS_PARAM procParam;
	U16 g_stat_buf_out_arr[2*1024];
	short *data_addr;
	AWB_STATUS awb_status;
	u32 awb_lux;//ae���ݵĻ�����������ֵ
} AWB_CTRL;

/**CCM���ƽṹ�嶨��*/
typedef struct ISP_CCM_CTRL_S
{
	VOID*			handle;
	CCM_PARAM		ccm_param;	
	MEM_TAB 		mem_tab[HIK_CCM_MTAB_NUM];
	CCM_PROCESS_PARAM procParam;
} CCM_CTRL;



 
/********************** HIK �Զ��ع�������ݽṹ����**********************/
/** ��˸ģʽ����*/
typedef enum
{
	HIK_ANTI_FLICKER_50HZ = 0,
	HIK_ANTI_FLICKER_60HZ = 1,
} ANTI_FLK_MODE;

/**�Զ��ع����ģʽ����*/
typedef enum 
{
	HIK_AE_SPOT_METERING = 0,
	HIK_AE_CENTER_METERING,
	HIK_AE_AVERAGE_METERING,
	HIK_AE_CUSTOM_METERING,
	HIK_AE_METERING_TYPE_NUMBER,
}HIK_AE_METERING_MODE;

/**��ͷ�ṹ�嶨��*/
typedef struct ISP_LENS_PARAM_S
{
	s32 af_type;
	s32 pre_zoom_ratio;
	s32 cur_zoom_ratio;
}ISP_LENS_PARAM;

/**��Ȧ���ƽṹ�嶨��*/
typedef struct ISP_IRIS_CTRL_S_
{
	u32	max_aperture_value;
	u32	min_aperture_value;
	u32	pre_max_gain;
	u32	max_gain;
	u32 iris_enable;
}ISP_IRIS_CTRL;

/**AE���ƽṹ�嶨��*/
typedef struct ISP_AEC_CTRL_S
{
    VOID *handle;
    MEM_TAB memTab[HIK_AE_MTAB_NUM];
    AEC_PARAM createParam;
    AEC_PROCESS_PARAM procParam;
	ISP_LENS_PARAM lensParam;
	SMART_IR_PARAM ir_ctrl;
	ISP_IRIS_CTRL iris_ctrl;
	AEC_Y_REF_LIST *y_ref_map_linear;
	AEC_Y_REF_LIST *y_ref_map_wdr;
	AEC_Y_REF_LIST *y_ref_map_linear_hlc;
	AEC_Y_REF_LIST *y_ref_map_hlc;
	U32 total_gain;
}ISP_AEC_CTRL;

/** �Զ��ع���ز���*/
typedef struct  _HIK_AE_PARAM_STRU_
{
	BOOL			 		auto_enable;
	U32						exposure_level;

	ANTI_FLK_MODE	 		anti_flicker_mode;
	U32				 		shutter_time_min;
	U32				 		shutter_time_max;
	U32					 	shutter_time;
	U32						gain_max;
	U32				 		gain;

	/**��������ز���*/
	S32						transition_counter;  // ��ʼֵ0
	S32						vin_tick;			  // ��ʼֵ0 

	HIK_AE_METERING_MODE	metering_mode;
}HIK_AE_PARAM;

/********************** HIK GBCE������ݽṹ����**********************/
typedef enum
{
	IPIPE_GBCE_METHOD_Y_VALUE=0,/*Cr CB unmodified*/
	IPIPE_GBCE_METHOD_GAIN_TBL=1
} IPIPE_GBCE_METHOD_T;

typedef struct 
{
	/** Defect Correction Enable */
	U16 enable;
	IPIPE_GBCE_METHOD_T typ;
	U16 * lookup_table;
}ipipe_gbce_cfg_t;

typedef struct
{
	void				*handle;
	MEM_TAB				memTab[HIK_GBCE_MTAB_NUM];
	GBCE_PARAM			createParam;
	GBCE_PROCESS_PARAM	procParam;
	ipipe_gbce_cfg_t	ipipeGbceCfg;
} GBCE_CTRL;

 
/***********************�Զ��۽�������ݽṹ����**********************/
typedef struct _HIK_AF_PARAM_
{
	U32			 auto_enable;
	U32			 lens_type;
	U16			 af_mode;
	U16			 af_tile_mode;
	U16			 zm_dist;
	U16			 fs_dist;
	U16			 fs_near;
	U16			 fs_far;
} HIK_AF_PARAM;

typedef enum KEY_TO_SHARE_MEM
{
	R_OVER = 0,//AF���ȡ��AFͳ����Ϣ��,��ΪR_OVER
	W_OVER = 1,//ISPд��ͳ����Ϣ,��ΪW_OVER
	R_W_ING = 2,//дAFͳ����Ϣǰ��ΪR_W_ING
	UNINITIALIZED = 3
}G_KEY_TO_MEM;

typedef struct INFO_SHARE_MEM
{
	G_KEY_TO_MEM flag;
	//ISP_AF_STAT  afd_param;
}G_INFO_SHARE_MEM;


typedef struct _HIK_AAA_PARAM_
{
	HIK_AWB_PARAM		 awb_param;
	HIK_AE_PARAM		 ae_param;
	HIK_AF_PARAM		 af_param;
}HIK_AAA_PARAM;


#ifdef __cplusplus
}
#endif

#endif /* __ISP_3A_H__ */
 

