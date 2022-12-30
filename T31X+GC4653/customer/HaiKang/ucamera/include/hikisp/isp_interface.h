/** @file isp_interface.h
*	@note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*	@brief isp模块对外头文件
*	
*	@author   yaojinbo
*	@date	  2015-05-26
*	@version  1.0
*	@note 
*	@note History:		  
*	@note	  <author>	 <time>    <version >	<desc>
*	@note	  yaojinbo	  2015-05-28  修改
*	@warning  
*	  All rights reserved. No Part of this file may be reproduced, stored
*	  in a retrieval system, or transmitted, in any form, or by any means,
*	  electronic, mechanical, photocopying, recording, or otherwise,
*	  without the prior consent of Hikvision, Inc.
*/

#ifndef __ISP_INTERFACE_H__
#define __ISP_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif
//#include "isp_inner.h"

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef int HRESULT;
#endif 	//!_HRESULT_DEFINED

#ifdef WIN32
#define ISP_ALIGN_BYTES  16
#else
#define ISP_ALIGN_BYTES  128
#endif


// 错误码定义
#define ISP_LIB_S_OK            0             //成功
#define ISP_LIB_S_FAIL          -1             //失败
#define ISP_LIB_E_PARA_NULL	 	0x40000000    //参数指针为空
#define ISP_LIB_E_MEM_NULL	   	0x40000001    //传入的内存为空
#define ISP_LIB_E_MEM_OVER	   	0x40000002    //内存溢出
#define ISP_LIB_E_PARAM_KEY	 	0x40000003    //错误的参数键值
#define ISP_LIB_E_PARAM_VAL	 	0x40000004    //错误的参数值

#define ISP_MOD_OPEN           1             /**ISP模块开启*/
#define ISP_MOD_CLOSE          0             /**ISP模块关闭*/

#define MAX_IR_LENS_GROUP (2)


// 设置参数键值,对于开关类型的键值，设置0表示关，1表示开启功能。
typedef enum _ISP_PARAM_KEY_
{
	/*======================ISP SYSTEM MODULES ======================*/
	/**000:显示ISP库版本信息;	<Range>{}     				<Mode>G	*/
	ISP_LIB_VERSION_I = 0x0,
	
	/**001:ISP开关;			<Range>{0,1}     						<Mode>G/S	*/
	ISP_EN_I,
	
	/**002:3A开关 ;			<Range>{0,1}     						<Mode>G/S	*/
	ISP_3A_EN_I,
	
	/**003:sensor 软件复位;	<Range>{1}     						<Mode>S	*/
	ISP_SENSOR_RESET_I,
	
	/**004:ISP freeze switch ;		<Range>{0,1} 0:启动，1:  冻结 	<Mode>S	*/
	ISP_FREEZE_EN_I,
	
	/**005:3A开关 ;			<Range>[0-0xffffffff]     				<Mode>G/S	*/
	ISP_MODULE_EN_I,
	
	/*=========================AE MODULE =========================*/
	/**032:AE开关 ;			<Range>{0,1}     						<Mode>G/S	*/
	ISP_AE_EN_I = 0x20,
	
	/**033:目标亮度等级;	<Range>[0,100]    					<Mode>G/S	*/
	ISP_AE_BRIGHTNESS_LEVEL_I,
	
	/**034:当前场景亮度; 	<Range>[0,255]						<Mode>S  	*/
	ISP_AE_CUR_LUMA_VAL_I,
	
	/**035:防闪烁功能;     	<Range>{0,1},1:60Hz,2:50Hz			<Mode>G/S */
	ISP_AE_ANTI_FLICKER_I,
	
	/**036:最大曝光时间，单位us; <Range>[10,1000000]		<Mode>G/S */
	ISP_AE_SHUT_SLOWEST_I,
	
	/**037:最小曝光时间，单位us; <Range>[10,1000000]		<Mode>G/S */
	ISP_AE_SHUT_FASTEST_I,
	
	/**038:慢快门功能开关; <Range>{0,1}						<Mode>G/S */
	ISP_AE_SLOW_SHUT_EN_I,
	
	/**039:最大增益等级; <Range>[0,100]						<Mode>G/S */
	ISP_AE_GAIN_MAX_LEVEL_I,
	
	/**040:当前曝光时间; <Range>[10,1000000]					<Mode>G/S */
	ISP_AE_CUR_SHUT_TIME_I,
	
	/**041:当前增益等级; <Range>[0,100]						<Mode>G/S */
	ISP_AE_CUR_GAIN_LEVEL_I,
	
	/**042:当前增益，单位DB; <Range>[0-?]					<Mode>G	*/
	ISP_AE_GAIN_DB_I,
	
	/**043:当前光圈大小; <Range>[0x0,0x3ff]						<Mode>G 	*/
	ISP_AE_CUR_IRIS_VAL_I,
	
	/**044:当前曝光状态; <Range>{0,1,2}0:调整，1:稳定，2:曝光跳到极限。	<Mode>G 	*/
	ISP_AE_STATUS_I,
	
	/**045:当前日夜状态; <Range>{-1,0,1}0:切夜晚，1:切白天，-1不调整。	<Mode>G	*/
	ISP_DAY_NIGHT_FLAG_I,
	
	/**046:日夜切换灵敏度，灵敏度越高，切白天越容易; <Range>[0,7]		<Mode>G/S	*/
	ISP_ICR_SENSITIVITY_I,
	
	/**047:配置光圈参数; 			<Range>								<Mode>G/S */
	ISP_IRIS_PARAM_I,

	/**048: 当前自动宽动态判断状态; 	<Range>{-1,0,1}0:切线性，1:切宽动态，-1不调整。	<Mode>G	*/
	ISP_AUTO_WDR_FLAG_I,
	
	/*=========================AWB MODULE =========================*/
	/**080:自动白平衡开关 ;<Range>{0,1}     					<Mode>G/S	*/
	ISP_AWB_EN_I = 0x50,
	
	/**081:白平衡调节速度等级;	<Range>[0,100]    			<Mode>G/S	*/
	ISP_AWB_SPEED_LEVEL_I,
	
	/**082:白平衡模式;		<Range>枚举类型     				<Mode>G/S	*/
	ISP_AWB_MODE_I,
	
	/**083:手动设置或者获取RGB三通道增益;	<Range>[0,0xffffffff]  <Mode>G/S	*/
	ISP_AWB_RGB_GAIN_I,
	
	/**084:饱和度等级;		<Range>[0,100]     					<Mode>G/S	*/
	ISP_SATURATION_LEVEL_I,
	
	/**085:色调等级;		<Range>[0,100]     					<Mode>G/S	*/
	ISP_HUE_LEVEL_I,
	
	/**086:灰度范围;		<Range> {0,1}  0:0-255,1:16-235 		<Mode>G/S	*/
	ISP_COLOR_STYLE_I,
	
	/**087:日夜切换时黑白彩色转换开关; <Range>{0,1}0：切换到白天，1：切换到夜晚。	<Mode>G/S	*/
	ISP_DAY_NIGHT_MODE_I,

	/*=========================AF MODULE ==========================*/
	/**112:日夜切换时黑白彩色转换开关; <Range>{0,1}	<Mode>G/S	*/
	ISP_AF_EN_I = 0x70,
	
	/**113:传镜头当前倍率给ISP; <Range>						<Mode>G/S	*/
	ISP_AF_ZOOM_RATIO_I,

	/**114:传镜头畸变校正开关;	<Range> [0,1]				<Mode>G/S	*/
	ISP_AF_LDC_MODE_I,

	/**115:注册AF回调函数	*/
	ISP_AEAF_PARAM_FUNC_REGISTER_I,

	/**116:设置或者获取AF窗口函数*/
	ISP_AEAF_WININFO_FUNC_REGISTER_I,

	/**117:获取AFD参数*/
	ISP_GET_AEAF_PARAM_FUNC_I,

	/**118:从dsp获取共享内存首地址*/
	ISP_SHARE_MEM_I,

	/*=========================IMAGE ENHANCE MODULE ==========================*/
	/**144:对比度等级;			<Range>[0,100]     				<Mode>yG/S	*/
	ISP_CONTRAST_LEVEL_I = 0x90,
	
	/**145:锐化等级;			<Range>[0,100]    				<Mode>G/S	*/
	ISP_SHARPEN_LEVEL_I,
	
	/**146:降噪模式;			<Range>{0,1,2}     				<Mode>G/S	*/
	ISP_NR_MODE_I,
	
	/**147:普通降噪模块等级;<Range>[0,100]     				<Mode>G/S	*/
	ISP_NR_LEVEL_I,
	
	/**148:空域降噪等级;		<Range>[0,100]     				<Mode>G/S	*/
	ISP_SPATIAL_NR_LEVEL_I,
	
	/**149:时域降噪强等级;	<Range>[0,100]     				<Mode>G/S	*/
	ISP_TEMPORAL_NR_LEVEL_I,
	
	/**150:宽动态模式;			<Range> 参数是结构体   	<Mode>G/S	*/
	ISP_WDR_MODE_I,
	
	/**151宽动态等级:;			<Range>[0,100]   					<Mode>G/S	*/
	ISP_WDR_LEVEL_I,
	
	/**152:背光补偿模式;<Range>{0,7} 0：关闭；1：上；2：下；3：左；4：右；5：中；6：自动；7：自定义背光补偿<Mode>G/S	*/
	ISP_BACK_LIGHT_MODE_I,
	
	/**153:强光抑制开关;		<Range>{0,1}     					<Mode>G/S	*/
	ISP_HIGH_LIGHT_CONTROL_EN_I,
	
	/**154:乾光抑制等级;		<Range>[0,100]    				<Mode>G/S	*/
	ISP_HIGH_LIGHT_LEVEL_I,
	
	/**155:去雾功能开关;		<Range>{0,1}     					<Mode>G/S	*/
	ISP_ANTI_FOG_EN_I,
	
	/**156:去雾功能强度;		<Range>[0,100]     				<Mode>G/S	*/
	ISP_ANTI_FOG_LEVEL_I,
	
	/**157:GBCE开关;				<Range>{0,1}     					<Mode>G/S	*/
	ISP_GBCE_EN_I,
	
	/**158:强光抑制等级;		<Range>[0,100]     				<Mode>G/S	*/
	ISP_GBCE_LEVEL_I,
	
	/**159:smart ir模式;			<Range>  参数为结构体		<Mode>S	*/
	ISP_SMART_IR_ATTR_I,
	
	/**160:smart ir AE权重开关;	<Range>[0,1],0,关闭，1，开启:使用默认参数<Mode>G/S	*/
	ISP_SET_IR_AE_WEIGHT,
	
	/**161:设备安装的场景模式;	<Range>{0,1}     				<Mode>G/S	*/
	ISP_SET_SCENE_MODE_I,
	
	/**162:被动坏点检测开关;<Range>{0,1}     					<Mode>G/S	*/
	ISP_STATIC_BPC_DETECT_I,
	
	/**163:被动坏点校正开关;<Range>{0,1}     					<Mode>G/S	*/
	ISP_STATIC_BPC_COR_EN_I,
	
	/**164:主动坏点校正开关;<Range>{0,1}     					<Mode>G/S	*/
	ISP_DYNAMIC_BPC_COR_EN_I,
	
	/**165:capture mode;				<Range>参数为枚举类型	<Mode>G/S	*/
	ISP_CAPTURE_MODE_I,
	
	/**166:水平镜像功能开关;	<Range>{0,1}						<Mode>G/S	*/
	ISP_MIRROR_EN_I,

	/**167:设置配置红外灯	<Range>{}						<Mode>G/S	*/
	ISP_SET_INFRATED_I,

	/**168:smart ir AE权重等级;	<Range>[0,256],0-255,256:使用默认参数<Mode>G/S	*/
	ISP_SET_IR_AE_WEIGHT_LEVEL,
	
	/**169:鱼眼中心点位移			<Range>	参数为结构体		<Mode>G/S*/
	ISP_FISHEYE_OFFSET_I,

	/*=========================ISP DEBUG INTERFACE==========================*/
	/**192:debug打印信息等级;	<Range>[0x0,0xffffffff]				<Mode>S	*/
	ISP_DEBUG_PRINT_LEVEL_I = 0xC0,
	
	/**193:抓raw数据接口;		<Range>   						<Mode>G	*/
	ISP_CAP_RAW_FRAME_I,
	
	/**194:ISP调试工具开关;	<Range>{0,1}     					<Mode>G/S	*/
	ISP_TEST_TUNING_EN_I,
	
	/**195:内部线程开关;		<Range>{0,1}     					<Mode>G/S	*/
	ISP_INNER_THREAD_EN_I,
	
	/**196:设置sensor寄存器;	<Range>参数为结构体   	<Mode>S	*/
	ISP_SET_SENSOR_REG_I,
	
	/**197:获取sensor寄存器的值;	<Range>参数为结构体   <Mode>S	*/
	ISP_GET_SENSOR_REG_I,
	
	/**198:打印isp图像相关信息;	<Range>   					<Mode>G	*/
	ISP_VIDEO_RESOLUTION_I,
	
	/**199:动态加载白平衡配置文件;	<Range>{0,1}     		<Mode>G/S	*/
	ISP_AWB_DNYNIC_LOAD_CFG_I,
	
	/**200:动态加载CCM配置文件;<Range>    					<Mode>S	*/
	ISP_CCM_DNYNIC_LOAD_CFG_I,
	
	/**201:AE调试接口，value的高16位是AE的键值，value的低16位是KEY对应值;	<Range>[0x0,0xffffffff]	     		<Mode>G/S	*/
	ISP_DEBUG_AE_ENTRY_I,
	
	/**202:AWB调试接口，value的高16位是AWB的键值，value的低16位是KEY对应值;<Range>[0x0,0xffffffff]	    	<Mode>G/S	*/
	ISP_DEBUG_AWB_ENTRY_I,
	
	/**203:CCM调试接口，value的高16位是CCM的键值，value的低16位是KEY对应值;<Range>[0x0,0xffffffff]	    	<Mode>G/S	*/
	ISP_DEBUG_CCM_ENTRY_I,
	
	/**204:GBCE调试接口，value的高16位是GBCE的键值，value的低16位是KEY对应值;	<Range>[0x0,0xffffffff]	    	<Mode>G/S	*/
	ISP_DEBUG_GBCE_ENTRY_I,
	
	/**205:demosic调试接口;		<Range>    						<Mode>S	*/
	ISP_DEBUG_DEMOSAIC_I,
	
	/**206:sharpen调试接口;		<Range>    						<Mode>S	*/
	ISP_DEBUG_SHARPEN_I,
	
	/**207:色彩抑制调试接口;<Range>    						<Mode>S	*/
	ISP_DEBUG_COLOR_SUPPRESS_I,
	
	/**208:降噪调试接口;		<Range>    						<Mode>S	*/
	ISP_DEBUG_DENOISE_I,
	
	/**209:gamma调试接口;		<Range>    						<Mode>S	*/
	ISP_DEBUG_GAMMA_I,
	
	/**210:黑电平调试接口;	<Range>    						<Mode>G/S	*/
	ISP_DEBUG_BLC_ENTRY_I,
	
	/**211:PWM调试接口;			<Range>   						<Mode>G/S	*/
	ISP_DEBUG_PWM_I,
	
	/**212:P-IRIS调试接口;		<Range>     						<Mode>G/S	*/
	ISP_DEBUG_P_IRIS_PARAM_I,

	/**213:动态加载isp bin配置文件;	<Range>{0,1}     		<Mode>G/S	*/
	ISP_BIN_DNYNIC_LOAD_CFG_I,

	/**214:AWB调试接口; <Range>[0x0,0xffffffff]		<Mode>G/S */
	ISP_DEBUG_GET_AWB_I,
	
	/**215:AE调试接口; <Range>[0x0,0xffffffff]		<Mode>G/S */
	ISP_DEBUG_GET_AE_I,
	
	/**216:LTM调试接口; <Range>[0x0,0xffffffff]		<Mode>G/S */
	ISP_DEBUG_SET_LTM_I,

	/**217:VPSS SCALER调试接口; <Range>[0x0,0xffffffff]		<Mode>G/S */
	ISP_DEBUG_VPSS_SCALER_I,

	/*=========================OTHER INTERFACE==========================*/
	/**256:机芯AE信息回传;		<Range>     						<Mode>G/S	*/
	ISP_AE_PARAM_CALLBACK 		= 0x100,
}ISP_PARAM_KEY;


typedef enum isp_vi_mode_e
{
	/** APTINA */
	APTINA_AR0130 = 0,

	/** SONY */ 
	SONY_IMX323 = 0x20,
	
	/** PANASONIC */
	PANASONIC_MN34220 = 0x40,
	PANASONIC_MN34422 = 0x41,
	PANASONIC_MN34425 = 0x42,

	//OmniVision
	OMNI_OV9712 = 0x60,
	OMNI_OV4689 = 0x61,
	OMNI_OV5658 = 0x62,
	OMNI_OS08A10= 0x63,
	OMNI_OS05A10= 0x64,
	OMNI_HIK187 = 0x65,
	OMNI_OS02C10= 0x66,
	OMNI_OS02D10= 0x67,
	OMNI_OS02D20= 0x68,
	OMNI_OS02K10= 0x69,
	OMNI_OS04B10= 0x6a,
	OMNI_HIK188 = 0x6b,
	OMNI_OS04C10= 0x6c, 

	
	//SHARP CCD
	SP_RJ33 = 0x80,

	//Silicon optronics,Inc.
	SOI_K02 = 0xA0,
	SOI_JXF37 = 0xA1,
	
	SAMPLE_VI_MODE_1_D1 = 0xFFFF,

}ISP_VI_MODE_E;


typedef enum E1_CAPTURE_MODE
{
	VI_CAPTURE_MODE_720P25 		 = 5,
	VI_CAPTURE_MODE_720P30 		 = 6,
	
	VI_CAPTURE_MODE_XVGAP25 	 = 10,	// 960-25/30
	VI_CAPTURE_MODE_XVGAP30,

	VI_CAPTURE_MODE_1080P15 = 15,
	VI_CAPTURE_MODE_1080P20 = 16,
	VI_CAPTURE_MODE_1080P25 	 = 17,	// 17
	VI_CAPTURE_MODE_1080P30,			// 18
	VI_CAPTURE_MODE_1080P50,			// 19
	VI_CAPTURE_MODE_1080P60,			// 20
	
	VI_CAPTURE_MODE_2048X1536P25 = 24,
	VI_CAPTURE_MODE_2048X1536P30 = 25,
	VI_CAPTURE_MODE_2048X1536P20 = 26,

	VI_CAPTURE_MODE_2560X1440P25 = 48,

	VI_CAPTURE_MODE_2048X1536P50 = 52,
	VI_CAPTURE_MODE_2048X1536P60 = 53,
	
	VI_CAPTURE_MODE_2592X1944P25 = 72,
	VI_CAPTURE_MODE_2592X1944P30 = 73,

	VI_CAPTURE_MODE_XVGAP50 	 = 87,	// 960-50/60
	VI_CAPTURE_MODE_XVGAP60		 = 88,

	VI_CAPTURE_MODE_2560X1920P25 = 94,
	VI_CAPTURE_MODE_2560X1920P20, 		//95
	VI_CAPTURE_MODE_2560X1920P30,		//96	
	
}ISP_E1_CAPTURE_MODE;

typedef struct  _AF_PARAM_STRU_
{
	int         abf_type;  					// 0,不支持，型号1， 
	int 		af_type;					// 0,不支持，型号1，
} AF_PARAM;


// 创建句柄相关参数
typedef struct ISP_CREATE_PARAM_STRU
{
	ISP_VI_MODE_E		sensor_id;		//sensor id
	unsigned int		res_type;		//res_type(hikio)
	void				*buf;			// buf poiner
	unsigned int		buf_size;		// buf size
	int					fd_iav;			// Operation handler of image kernel,if no handler just ignore it;
	int					max_width;		// width of max vin(bayer)
	int					max_height;		// height of max vin(bayer) 
    int					print_en;		// print info,only 0 and 1 are accepted, 0 - disable, 1 - enable;
	AF_PARAM			af_params;		//镜头类型
	unsigned int		lens_type;		//鱼眼镜头类型
    char            	reserved[60];
}ISP_CREATE_PARAM;



// ISP 版本信息
typedef struct ISP_LIB_VERSION_S 
{
	unsigned char	major;				/* 主版本号，接口改动、功能增加、架构变更时递增 */
	unsigned char	sub_version;		/* 子版本号，性能优化、局部结构调整、模块内集成其他库的主版本提升时递增 */
	unsigned char	revision_version;	/* 修正版本号，修正bug后递增 */
	char			mk_date_time[64];			/*例如150424-18:58:00*/
	char			description[64];
}ISP_LIB_VERSION;



// isp处理需要的参数结构体
typedef struct ISP_PROC_PARAM_STRU_
{
    int             width;            // 图像宽度
	int             height;           // 图像高度
	char            reserved[64];

} ISP_PROC_PARAM;



typedef struct 
{
	unsigned int x; //图像中心水平坐标
    unsigned int y; //图像中心垂直坐标
    unsigned int w; //图像窗口宽最大不能超过VIDEO_PARAM_CFG结构体中image_viW, 最小64
    unsigned int h; //图像窗口高最大不能超过VIDEO_PARAM_CFG结构体中image_viH, 最小32
}ISP_WINDOW_RECT_S;

typedef struct ISP_BLC_PARAM_S
{
	unsigned int 		blc_mode;
	ISP_WINDOW_RECT_S	blc_window_rec;
}ISP_BLC_PARAM;


typedef struct _SENSOR_CFG_S
{
	int sensor_id;
	int capture_mode;
	int wdr_mode;
}SENSOR_CFG_S;

typedef struct SENSOR_REG_ATTR_S
{
	unsigned int reg_addr;//sensor寄存器地址
	unsigned int reg_data;//sensor寄存器值
	unsigned int reg_num;//sensor寄存器个数
}SENSOR_REG_ATTR;

typedef struct ISP_IRIS_ARRT_S
{
	unsigned int  aeMode; // 0:自动光圈， 1:手动光圈(DC-IRIS)，2: P-IRIS类型1...
	unsigned int  sensitivity; //自动光圈灵敏度(DC-IRIS时有效);
	unsigned int  pirisMode; // P-IRIS模式下有效，0:自动光圈，1:固定光圈;
	unsigned int  pirisLevel; // P-IRIS固定光圈下有效，光圈级别，1~100
	unsigned int  res[4];
} ISP_IRIS_ATTR;


typedef struct VI_ATTR_S
{
	int		bitLen; //bayer数据位宽，用于抓取bayer数据
	int		width; //送给vi模块的bayer数据宽度
	int		height; //送给vi模块的bayer数据高度
	float	f32FrameRate;   /* RW. */
	int 	max_width;
	int 	max_height;
}VI_ATTR;

typedef struct
{
	unsigned char mode;						//红外灯控制模式1: 手动 0:自动
	unsigned char IRGrpNo;					//红外灯组数
	unsigned char level;  					//整体红外灯等级
	unsigned char grpLevel[MAX_IR_LENS_GROUP];//分组红外灯等级
	unsigned char anti_overexp;				//防红外过曝开关
	unsigned char IR_status; 					//红外灯状态  0:关闭  1:开启
	unsigned char res[5];
}SMART_IR_PARAM;


typedef enum _ISP_WDR_MODE_E_
{
	WDR_MODE_NONE = 0,
	WDR_MODE_BUILT_IN,
	
	WDR_MODE_2To1_LINE = 2,
	WDR_MODE_2To1_FRAME,
	WDR_MODE_2To1_FRAME_FULL_RATE,

	WDR_MODE_3To1_LINE,
	WDR_MODE_3To1_FRAME,
	WDR_MODE_3To1_FRAME_FULL_RATE,

	WDR_MODE_4To1_LINE,
	WDR_MODE_4To1_FRAME,
	WDR_MODE_4To1_FRAME_FULL_RATE,

	WDR_DIGITAL = 0x10,
} ISP_WDR_MODE_E;

//===========================================================
typedef struct ISP_INIT_PARAM_S_
{
	ISP_VI_MODE_E sensor_id;
	unsigned int fps;
	unsigned int width;
	unsigned int height;
	unsigned int buf_size;
	unsigned int buf_addr;
	unsigned int align;
	char reserved[128];
}ISP_INIT_PARAM;


typedef int (*isp_pend)(int IspDev);
typedef int (*isp_post)(int IspDev);

typedef struct  _ISP_FUNC_CALL_
{
	isp_pend				isp_sem_pend;
	isp_post				isp_sem_post;
} ISP_FUNC_CALL;


typedef struct _ISP_SHARE_BUF_INFO_
{
	int len;
	char *buf;

} ISP_SHARE_BUF_INFO;

//鱼眼设备中心点位移结构体，与dsp结构体一致
typedef struct ISP_FISHEYE_OFFSET_S
{
	int left;	//左为负数，右为正
    int top;	//上为负数,下为正
    int radius;	//鱼眼半径
}ISP_FISHEYE_OFFSET;

int ISP_Exit(void *handle);

/************************************************2020.5.22 LHY ADD ********************************************/
struct video_isp_class 
{
	void *handle;
	ISP_CREATE_PARAM create_param;
};

//================================================================

/********************************************************************************
 * 接口函数
 *********************************************************************************/

/*******************************************************************************
 * 功  能：获取当前编码版本信息
 * 参  数：无
 * 返回值：返回版本信息
 * 备  注：版本信息格式为：版本号＋年（7位）＋月（4位）＋日（5位）
 *         其中版本号为：主版本号（6位）＋子版本号（5位）＋修正版本号（5位）
 *******************************************************************************/
HRESULT ISP_GetVersion(ISP_LIB_VERSION *isp_lib_version);


/******************************************************************************
 * 功  能: 获取库内部所需的内存大小
 * 参  数: param - 创建句柄相关参数
 *
 * 返回值: 状态码
 * 备  注:
 *******************************************************************************/
HRESULT ISP_GetMemSize(ISP_CREATE_PARAM *param);



/******************************************************************************
 * 功  能: 创建句柄实例
 * 参  数:
 *         param       - 创建句柄相关参数
 *         handle      - 句柄
 *
 * 返回值: 状态码
 * 备  注:
 *******************************************************************************/
HRESULT ISP_Create(ISP_CREATE_PARAM *param, VOID **handle);
//HRESULT ISP_Create(ISP_INNER_PARAM *param);

/******************************************************************************
 * 功 能：算法库参数设置接口, 设置ISP的相关参数。
 *                    注意必须在ISP_Create()之后调用。
 * 参 数：
 *         handle     -IO 模块句柄
 *         param_key  -I  参数键
 *         param        -I  参数指针
 *
 * 返回值：返回状态码
 ********************************************************************************/
HRESULT ISP_SetKeyParam(void *handle, ISP_PARAM_KEY param_key, void *param);



/*******************************************************************************
 * 功 能：算法库参数获取接口, 获得ISP的相关参数。
 *                    注意必须在ISP_Create()之后调用。
 * 参 数：
 *         handle      -IO 模块句柄
 *         param_key   -I 参数键
 *         param         -O 参数指针
 *
 * 返回值：返回状态码
 *********************************************************************************/
HRESULT ISP_GetKeyParam(void *handle, ISP_PARAM_KEY param_key, void *param);


/******************************************************************************
 * 功  能: 获取与ISP相关的几个关键参数
 * 参  数:
 *         sensor_id	- Senser ID参数
 *         vi_attr		- vi属性结构体
 *
 * 返回值: 状态码
 * 备  注:
 *******************************************************************************/
HRESULT ISP_GetViAttr(SENSOR_CFG_S sensor_cfg, VI_ATTR *vi_attr);

/******************************************************************************
 * 功  能: 获取内核中ISP中断状态
 * 参  数:
 *         sensor_id	- Senser ID参数
 *         vi_attr		- vi属性结构体
 *
 * 返回值: 状态码
 * 备  注:
 *******************************************************************************/
HRESULT ISP_GetViHwStatus(void	*pstStat);


/******************************************************************************
 * 功  能: ISP预初始化接口，包括了sensor时钟、复位等相关配置
 * 参  数:
 *		   sensor_id - isp对外头文件中定义的sensor类型
 *
 * 返回值: 状态码
 * 备  注:添加一款新Sensor时需要添加对应的支持。
 *
 *******************************************************************************/
HRESULT ISP_PreInit(ISP_VI_MODE_E sensor_id);


/*******************************************************************************
 * 功 能：DSP向ISP注册函数
 * 参 数：
 *		   param		- 参数指针
 *
 * 返回值：返回状态码
 *********************************************************************************/

HRESULT ISP_Register_CallBackFunc(ISP_FUNC_CALL *param);

#ifdef __cplusplus
}
#endif


#endif /* __ISP_INTERFACE_H__ */

