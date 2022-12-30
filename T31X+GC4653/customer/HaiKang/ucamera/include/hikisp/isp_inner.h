/** @file isp_inner.h
*	@note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*	@brief isp模块中内部定义头文件
*	
*	@author   yaojinbo
*	@date	  2015-05-26
*	@version  1.0
*	@note 
*	@note History:		  
*	@note	  <author>	 <time>    <version >	<desc>
*	@note	  yaojinbo	  2015-05-28  修改
*	@warning  
*/


#ifndef __ISP_INNER_H__
#define __ISP_INNER_H__

#ifdef __cplusplus
extern "C" {
#endif
#if 01
/**包含常用c库头文件和SDK头文件*/
#include "isp_platform.h"
/**sensor相关宏定义和函数接口定义*/
#include "sensor.h"
/**ISP 3a模块所需头文件*/
#include "isp_3a.h"
/**isp模块对外头文件*/
#include "isp_struct_type.h"
/**主要声明sensor coms和ctl文件中的函数*/
#include "isp_cmos_ctrl.h"
/**图像参数相关结构体、宏*/
#include "isp_img.h"
/**外部声明全局变量、结构体。*/
#include "isp_global_object.h"
/**isp模块对外头文件*/
#include "isp_interface.h"
/**POSIX封装函数头文件*/
//#include "pwrapper.h"
/**镜头相关头文件*/
//#include "isp_iris.h"
#endif
//#include "isp_platform.h"
//#include "isp_interface.h"
//#include "isp_global_object.h"
//#include "isp_struct_type.h"
//#include "isp_sensor_if.h"
/**错误打印立刻打印*/
#define CHECK_RET(state, error_code)						\
		if (state)													\
		{\
			printf("[%s]:%d line [%s] return %d ERROR\n",__FILE__,__LINE__ , __func__, error_code);	\
			return error_code;\
		}



/* *当前版本号*/
#define ISP_MAJOR_VERSION       1  /** 主版本号，接口改动、功能增加、架构变更时递增，最大63 */
#define ISP_SUB_VERSION         0  /* *子版本号，性能优化、局部结构调整、模块内集成其他库的主版本提升时递增，最大31 */
#define ISP_REVISION_VERSION    0  /* *修正版本号，修正bug后递增，最大31 */

#define COMPONENT_ISP_VERSION_IPZOOM "V1.0.0"
#define COMPONENT_ISP_PLATFORM_IPZOOM "E6_IPZOOM"

#define COMPONENT_ISP_VERSION_IPC "V1.0.0"
#define COMPONENT_ISP_PLATFORM_IPC "E6_IPC"

#define COMPONENT_MODULENAME "ISP"
#define COMPONENT_ISP_PLATVER "FH8856"

/**描述信息*/
#define ISP_VERSION_DESCRIPTION			"E6"


/**定义矫正数据存储文件名*/
#define DEFECT_PIXEL_FILE_NAME			"defectPixel.bin"
#define FIXED_PATTERN_NOISE_FILE_NAME	"fixedpatternnoise.bin"
#define LENS_SHADING_FILE_NAME			"lensshading.bin"

#define CHECK_PTR(ptr)					\
		do{ 							\
			if(NULL == (ptr)) 			\
			{							\
				printf("func:%s,line:%d, NULL pointer\n",__FUNCTION__,__LINE__);\
				return ISP_LIB_S_FAIL;	\
			}							\
		}while(0);

typedef enum 
{
	HIK_CALIB_DPC = 1,
	HIK_CALIB_FPN,
	HIK_CALIB_LENS_SHADING,
} HIK_CALIBRATION_TYPE;

// GBCE曲线枚举定义
enum{
	CURVE_AMBA = 1,	//amba 
	CURVE_HISI = 2,	//hisi
	CURVE_TI   = 3,	//ti
	CURVE_IR = 4,	//ir
	CURVE_SLASH = 5, //slash
};

/**缓存结构体 */
typedef struct _ISP_BUF_STRU_
{
	VOID  *start;	/**缓存起始位置*/
	VOID  *end;		/**缓存结束位置*/
	VOID  *cur_pos;	/**缓存空余起始位置*/
}ISP_BUF;

typedef struct ISP_CAPTURE_MODE_S_
{
	ISP_E1_CAPTURE_MODE	capture_mode; /**设置输出的帧率模式*/
	u32 image_width;
	u32 image_height;
	float frame_rate;
}ISP_CAPTURE_MODE;

typedef struct  _DC_IRIS_STRU_
{
	U32 enable;
	S32 sensitivity;
} DC_IRIS;

#if 01
/** isp内部建立的线程控制结构体*/
typedef struct _ISP_TASK_CONTROL_
{
	U32					enable;
	//pthread_t			id;
	S32					exit_flag;
}ISP_TASK_CONTROL;
#endif

/** 图像质量相关数据结构*/
typedef struct _IMAGE_QUALITY_STRU_
{
	S32			 saturation;		/** 饱和度*/
	S32			 brightness;		/** 亮度*/
	S32			 contrast;			/** 对比度*/
	S32			 hue;				/** 色调*/
	S32			 sharpness; 		/**  锐度*/
	S32			 color_conv_type;	/** 色彩转换矩阵类型*/
	S32			 brightness_up; 	/**  数字亮度补偿*/
	S32 		 icr_sensitivity;	/**  日夜切换灵敏度*/
} IMAGE_QUALITY;

/**图像增强处理相关数据结构*/
typedef struct _IMAGE_ENHANCE_STRU_
{
	S32 		 denoise_mode; /**0, 关闭；1，普通模式；2，专家模式*/
	S32			 spatial_denoise_enable; /**专家模式中，空域开关*/
	U32			 spatial_denoise_strength;/**3D 降噪中，空域强度*/

	S32			 temporal_denoise_enable; 	/**时域开关。*/
	U32			 temporal_denoise_strength;	/**时域强度。*/

	U32			 auto_local_exp_mode;//宽动态模式
	U32			 drc_mode;
	U32			 drc_level;

	U32			 backlight_comp_enable;
	U32			 backlight_comp_mode;

	U32			 day_night_mode; 
	U32			 high_light_control_enable;
	U32			 high_light_control_strength;

	U32			 anti_fog_strength;
	U32			 anti_fog_enable;
	U32			 gbce_enable;
	U32			 gbce_strength;
	U32			 scene_mode;
} IMAGE_ENHANCE;


/**ISP 架构的内部参数*/
typedef struct _ISP_INNER_PARAM_STRU_
{
	S32					fd_iav;		/** Operation handler of image kernel;*/
	U32					res_type;	/*HikIo resource type specified by application*/
	S32					isp_enable;
	HIK_AAA_PARAM		aaa_param;
	ISP_IRIS_ATTR		iris_param;
	IMAGE_QUALITY		img_quality;
	IMAGE_ENHANCE		img_enhance;
	ISP_TASK_CONTROL	isp_server_engin; /** isp 引擎控制参数*/   
	ISP_TASK_CONTROL	isp_inner_task;/** isp 内部线程控制参数*/   
	ISP_TASK_CONTROL	isp_image_debug;/** isp 图像调试工具控制参数*/ 
	ISP_TASK_CONTROL	isp_debug;/** isp debug线程控制参数*/ 
	
	VOID				*isp_inner_ctrl;
	VOID				*cfg_tables;

	/** 调试打印信息控制*/
	S32					print_en;
	U32					lens_type;
}ISP_INNER_PARAM;


/**ISP DEBUG信息相关函数声明*/
VOID 	isp_show_lib_version(ISP_INNER_PARAM *inner_param);
void 	isp_show_ae_version(ISP_INNER_PARAM *inner_param);
VOID	isp_show_ccm_version(ISP_INNER_PARAM *inner_param);
VOID	isp_show_awb_version(ISP_INNER_PARAM *inner_param);
VOID 	isp_show_gbce_version(ISP_INNER_PARAM *inner_param);
VOID 	isp_get_lib_version(ISP_LIB_VERSION *isp_lib_version);
HRESULT isp_debug_set_sensor_reg(ISP_INNER_PARAM *inner_param, SENSOR_REG_ATTR reg_attr);
HRESULT isp_debug_get_sensor_reg(ISP_INNER_PARAM *inner_param, SENSOR_REG_ATTR reg_attr);
HRESULT isp_debug_set_module_control(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_debug_get_module_control(ISP_INNER_PARAM *inner_param);
HRESULT isp_debug_tool_en(ISP_INNER_PARAM *inner_param,  S32 tools_en);
HRESULT isp_debug_set_awb_param(ISP_INNER_PARAM *inner_param);
HRESULT isp_debug_set_ccm_param(ISP_INNER_PARAM *inner_param);
HRESULT isp_debug_set_ae_key(S32 param_val);
HRESULT isp_debug_get_ae_key(S32 param_val);
HRESULT isp_debug_set_awb_key(S32 param_val);
HRESULT isp_debug_get_awb_key(S32 param_val);
HRESULT isp_debug_set_ccm_key(S32 param_val);
HRESULT isp_debug_set_gbce_key(S32 param_val);
HRESULT isp_debug_print_awb_params(ISP_INNER_PARAM * inner_param);
HRESULT isp_debug_print_ccm_params(ISP_INNER_PARAM * inner_param);
HRESULT isp_debug_print_gbce_params(ISP_INNER_PARAM *inner_param);
HRESULT isp_debug_print_gamma_params(ISP_INNER_PARAM * inner_param);
HRESULT isp_debug_set_gamma_params(ISP_INNER_PARAM * inner_param,S32 value);
HRESULT isp_debug_set_ltm_param(ISP_INNER_PARAM * inner_param, U32 weight);

HRESULT isp_load_isp_bin(void);
HRESULT isp_refresh_fh_param(ISP_INNER_PARAM *inner_param);


// GBCE功能相关函数
HRESULT isp_init_gbce(ISP_INNER_PARAM * inner_param);
HRESULT isp_set_gbce_en(ISP_INNER_PARAM *inner_param);
HRESULT isp_set_gbce_strength(ISP_INNER_PARAM *inner_param);
HRESULT isp_set_gbce_fuction(ISP_INNER_PARAM * inner_param);
HRESULT isp_switch_gbce_table(ISP_INNER_PARAM *inner_param,S32 param_val);
HRESULT isp_set_gbce_ctrl_key(S32 param_val);
HRESULT isp_set_anti_fog_status(ISP_INNER_PARAM *inner_param);
HRESULT isp_gbce_running(ISP_INNER_PARAM *inner_param);


/** 降噪函数声明*/
HRESULT isp_set_denoise_mode(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_spatial_denoise_strength(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_temporal_denoise_strength(ISP_INNER_PARAM *inner_param, S32 param_val);


/**ISP 内存分配相关函数声明*/
void 	isp_free_mem_tab(MEM_TAB *memTab, U32 cnt);
HRESULT isp_alloc_mem_tab(MEM_TAB *memTab, U32 cnt);
VOID*	isp_alloc_buffer(ISP_BUF *buf, U32 size);

/**前端参数接口声明*/
HRESULT isp_set_brightness(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_saturation(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_hue(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_day_night_mode(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_sharpeness(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_contrast(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_backlight_mode(ISP_INNER_PARAM *inner_param, ISP_BLC_PARAM *blc_info);
HRESULT isp_set_smart_ir(ISP_INNER_PARAM *inner_param, SMART_IR_PARAM smart_ir_param);
HRESULT isp_open_gpio_dynamic_lib(void);
HRESULT isp_set_gpio_dynamic_func(void);
HRESULT isp_init_dynamic_lib_hikio(U32 res_type);
HRESULT ISP_Set_Pwm_Duty(ISP_INNER_PARAM *inner_param, S32 param_val);


/**曝光相关函数声明*/
HRESULT hik_ae_run(void);
HRESULT hik_ae_init(ISP_INNER_PARAM *inner_param);
HRESULT isp_stop_ae(ISP_INNER_PARAM *inner_param);
HRESULT isp_start_ae(ISP_INNER_PARAM *inner_param);
HRESULT isp_set_shutter_fastest(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_shutter_slowest(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_shutter_manual(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_gain_max(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_gain_manual(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_get_bayer_luma(S32 *param_val);
HRESULT isp_get_gain_level(S32 *param_val);
HRESULT isp_get_exp_status( S32* exposure_status );
HRESULT isp_set_luma_ref(ISP_INNER_PARAM * inner_param);
HRESULT isp_set_icr_sensitivity(ISP_INNER_PARAM *inner_param, S32 icr_value);
HRESULT isp_get_daynight_flag(ISP_INNER_PARAM *inner_param,S32 *DayNightFlag);
HRESULT isp_get_current_db(U32 *pval);
HRESULT isp_set_ae_weight(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_ae_weight_strength(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_get_current_gain(U32 *pval);
HRESULT isp_get_current_long_exp(U32 *pval);
HRESULT isp_set_ae_print(ISP_INNER_PARAM *inner_param);


/**AF相关函数说明*/
HRESULT isp_set_af_stat_param(ISP_AF_STAT_PARAM *pst_af_stat_param);
HRESULT isp_get_af_stat_param(ISP_AF_STAT_PARAM *pst_af_stat_param);
HRESULT isp_write_af_stat_to_buf();


/**白平衡相关函数声明*/
HRESULT hik_awb_run(void);
HRESULT hik_awb_init(void);
HRESULT isp_stop_awb(ISP_INNER_PARAM *inner_param);
HRESULT isp_start_awb(ISP_INNER_PARAM *inner_param);
HRESULT isp_set_awb_speed(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_awb_mode(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_rgb_gain(ISP_INNER_PARAM *inner_param, HIK_WB_GAIN wb_gain);
HRESULT isp_get_rgb_gain(HIK_WB_GAIN *wb_gain);
HRESULT isp_refresh_awb_param(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_awb_print(ISP_INNER_PARAM *inner_param);


/**wdr相关函数声明*/
HRESULT isp_set_ltm_status(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_set_ltm_strength(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_get_auto_wdr_flag(ISP_INNER_PARAM * inner_param, S32* auto_wdr_flag);
HRESULT isp_set_wdr_status(ISP_INNER_PARAM *inner_param, S32 param_val);
HRESULT isp_refresh_isp_config(ISP_INNER_PARAM *inner_param);
HRESULT isp_set_wdr_param(ISP_INNER_PARAM *inner_param);
HRESULT isp_set_highlight_control_attr(ISP_INNER_PARAM *inner_param);


/**capturemode 相关*/
HRESULT isp_set_capture_mode(ISP_INNER_PARAM *inner_param, SENSOR_CFG_S sensor_cfg);
HRESULT isp_set_mirror_mode(ISP_INNER_PARAM *inner_param, S32 param_val);

/**获取sensor类型、当前分辨率、sensor重启函数声明*/
HRESULT isp_set_sensor_reset(ISP_INNER_PARAM *inner_param,S32 param_val);

/**光圈控制相关*/
HRESULT isp_set_iris_attr(ISP_INNER_PARAM *inner_param,ISP_IRIS_ATTR * iris_attr);

/**ISP启动、初始化以及参数联动函数声明*/
HRESULT isp_start(ISP_INNER_PARAM *inner_param);
HRESULT isp_sensor_init_cfg(ISP_INNER_PARAM *inner_param);
HRESULT isp_get_vi_attr(SENSOR_CFG_S sensor_cfg,VI_ATTR *vi_attr);
//HRESULT isp_get_viu_attr(SENSOR_CFG_S sensor_cfg, ISP_VI_ATTR_S *vi_attr);
HRESULT isp_sensor_reset(void);
HRESULT isp_sensor_unreset(void);
VOID isp_dist_sensor_type(S32 sensor_type);
HRESULT isp_get_io_handle(VOID);

HRESULT isp_capture_raw(ISP_INNER_PARAM *inner_param, int framecount);
VOID* 	isp_inner_loop(VOID *cancel);
HRESULT isp_set_gb_param(U32 agc);
HRESULT isp_set_tar_luma(U32 agc);
HRESULT isp_set_apc_param(ISP_INNER_PARAM *inner_param, U32 agc);
HRESULT isp_set_ltm_param(ISP_INNER_PARAM *inner_param, U32 *ltm_table);
HRESULT isp_get_ltm_param(ISP_INNER_PARAM *inner_param, U32 agc, U32 *ltm_table);
HRESULT isp_set_vpss_param(ISP_INNER_PARAM *inner_param, U32 agc);

HRESULT isp_get_chip_id(VOID);
HRESULT isp_set_anti_purple(ISP_INNER_PARAM *inner_param);
HRESULT isp_set_lc_param(ISP_INNER_PARAM *inner_param);

S32 get_sensor_mn34425_vmax();
S32 get_sensor_ov2735_vmax();
S32 get_sensor_k02_vmax();
S32 get_sensor_sp2309_vmax();

//鱼眼中心点校正相关函数
HRESULT isp_set_fisheye_offset(ISP_INNER_PARAM *inner_param, ISP_FISHEYE_OFFSET fisheye_offset);
HRESULT isp_get_fisheye_offset(ISP_INNER_PARAM *inner_param, ISP_FISHEYE_OFFSET *fisheye_offset);
HRESULT isp_sensor_register_cb(ISP_INNER_PARAM * inner_param);

/*******************************************************************************
 * 功 能：isp_gam_interface,
 * 参 数：
 *	gamma	君正的gamma指针，
 *	alpha	gamma的权重值0-255
 *
 * 返回值：返回状态码
 *********************************************************************************/
HRESULT isp_gam_interface(IMPISPGamma *gamma,U32 alpha);
#ifdef __cplusplus
}
#endif

#endif /* __ISP_INNER_H__ */

