/***********************************************************************
* 
* 版权信息：版权所有 (c) 2013, 杭州海康威视数字技术有限公司, 保留所有权利
* 
* 文件名称：ae.h
* 文件标识：HIKVISION
* 摘    要：海康威视
*  
*
* *当前版本：1.0.0(h1平台)
* 作    者: 刘刚
* 日    期：2016年6月30号
* 备    注: 曝光库对外接口声明
*
************************************************************************
*/

#ifndef _AE_LIB_H_
#define _AE_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mem_tab.h"

/*******************************************************************************************************************************
* 文件版本和时间宏定义
*******************************************************************************************************************************/
/* 当前版本号*/
#define HIK_AE_MAJOR_VERSION        	 5   		 /* 主版本号，接口改动、功能增加、架构变更时递增，最大63 */
#define HIK_AE_SUB_VERSION               4   		 /* 子版本号，性能优化、局部结构调整时递增，最大31 */
#define HIK_AE_REVISION_VERSION     	 30   		 /* 修正版本号，修正bug后递增，最大31 */

/* 版本日期*/
#define HIK_AE_VER_YEAR             	 16          /* 年*/
#define HIK_AE_VER_MONTH           		 9           /* 月*/
#define HIK_AE_VER_DAY             		 30          /* 日*/


/*******************************************************************************************************************************
* 宏定义
*******************************************************************************************************************************/
#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef int HRESULT;
#endif  /*_HRESULT_DEFINED*/

/* 错误码定义 */
#define HIK_AE_LIB_S_OK			    0x00000000      /* 成功*/		    
#define HIK_AE_LIB_S_FAIL		    0x80000001      /* 失败*/		    
#define HIK_AE_LIB_E_PARA_NULL	    0x80000002     	/* 参数指针为空*/
#define HIK_AE_LIB_E_MEM_NULL	    0x80000003	    /* 内存为空*/
#define HIK_AE_LIB_E_MEM_OVER       0x80000004      /* 内存溢出     */

#define HIK_AE_MTAB_NUM                 1      	    /* 所需内存*/
#define MAX_HDR_FRM			       		4        	/* hdr最大支持帧数*/

/*参数键值*/
typedef enum _AEC_PARAM_KEY
{
    AEC_AE_SWITCH_KEY             =  0x00000001,    /* AE开关*/
	AEC_Y_REF_RANGE_KEY           =  0x00000002,    /*参考亮度级数[0 11]*/
	AEC_LIGHT_FREQ_KEY            =  0x00000003,    /*供电频率,1:50HZ;2:60HZ*/
	AEC_AE_MODE_KEY               =  0x00000004,    /* ae模式*/
	AEC_EFFECT_INTERVAL_KEY       =  0x00000005,    /*生效间隔帧数*/
	AEC_RESOLUTION_FRAMERATE_KEY  =  0x00000006,    /*分辨率帧率*/ 
	AEC_SHUT_MAX_KEY              =  0x00000008,    /*最长曝光时间*/
	AEC_SHUT_MIN_KEY              =  0x00000009,    /*最短曝光时间*/
	AEC_GAIN_MAX_KEY              =  0x0000000A,    /*增益最大值(9p031:128) (033:255)*/
	AEC_GAIN_MIN_KEY              =  0x0000000B,    /*增益最小值*/
	AEC_Y_REF_KEY                 =  0x0000000C,    /*参考亮度级别*/   
	AEC_SENSOR_TYPE_KEY           =  0x0000000D,    /*前端类型*/ 
    AEC_GAIN_KEY                  =  0x0000000E,    /*当前增益*/ 
	AEC_SET_GAIN_REG              =  0x0000000F,    /*设置增益寄存器*/
	AEC_SET_SHUT_REG              =  0x00000010,    /*设置快门寄存器*/
	AEC_SET_APER_TYPE             =  0x00000011,    /*设置光圈类型*/         
	AEC_SET_IRIS_PARAM            =  0x00000012,    /*设置光圈参考电压等参数*/
	AEC_GAIN_DB_KEY               =  0x00000013,    /*当前增益DB数*/
	AEC_GAIN_ISO_KEY              =  0x00000014,    /*当前增益倍数*100, 用于R2平台的增益获取*/
	AEC_GAIN_ISP_KEY              =  0x00000015,    /*当前isp增益，用于R2平台的ISP增益获取*/
	AEC_GAIN_ISP_SFT_KEY       	  =  0x00000016,    /*ISP增益的精度，用于R2平台的增益精度获取*/    
	AEC_SET_AN41908A_MAX_VAL  	  =  0x00000017,    /*设置当前AN41908最大光圈值*/
	AEC_SET_AN41908A_MIN_VAL  	  =  0x00000018,    /*设置当前AN41908最小光圈值*/
	AEC_PLATFORM_TYPE_KEY         =  0x00000019,    /*平台类型配置键值*/
	AEC_SET_VMAX_KEY              =  0x0000001A,    /*VMAX设置键值*/

	AEC_CUR_LONG_EXPOSURE         =  0x0000001B,    /*慢快门当前倍数*/
	
	AEC_SET_CTL_SPEED_EN          =  0x00000020,    /*设置灵敏度控制开关，0：关，1：开，默认开*/
	
	AEC_SET_DR_YRA_EN             =  0x00000021,    /*设置根据动态范围调节开关，0：关，1：开，默认关*/
	AEC_SET_DR_YRA_LEVEL          =  0x00000022,    /*设置根据动态等级，[0, 100]，默认50*/
	AEC_SET_LL_YRA_EN             =  0x00000023,    /*设置根据照度水平调节开关，0：关，1：开，默认开*/
	AEC_SET_GA_YRA_EN			  =  0x00000025,	/*设置根据增益调整参考亮度开关，0:关，1:开，默认关闭*/
	AEC_CUR_EXPOSURE_STATUS       =  0x00000026,    /*获取曝光库的最大值*/
	AEC_EXPO_FPS_P_N 			  =  0x00000027,    /*外部配置制式0为P制1为N制*/
	 //irl related
    AEC_IRL_CTL_EN                =  0x00000030,    /*红外灯亮度控制使能开关， 0: 关，1：开，默认关*/
	AEC_IRL_MIN_DUTY              =  0x00000031,    /*设置占空比最小值[0, 100], 默认值0*/
	AEC_IRL_MID_DUTY              =  0x00000032,    /*设置占空比中间值[0, 100], 默认值100*/
	AEC_IRL_MAX_DUTY              =  0x00000033,    /*设置占空比最大值[0, 100]，默认值100*/
	AEC_IRL_MID_GAIN              =  0x00000034,    /*设置增益中间值[0, 100], 默认值40*/
	AEC_IRL_MANUAL_DUTY_FAR       =  0x00000035,    /*红外灯(远灯)亮度控制手动设置占空比，[0, 100],*/
	AEC_IRL_MANUAL_DUTY_NEAR      =  0x00000036,    /*红外灯(近灯)亮度控制手动设置占空比，[0, 100],*/
	
	AEC_IRL_PWM_GRP_TYPE	      =  0x00000037,    /*红外灯支持的组数*/
	AEC_PWM_MODE_SWITCH           =  0x00000038,    /*切换新老红外灯控制策略*/
	AEC_PWM_FREQ_FAR              =  0x00000039,    /*远灯的时钟频率*/
	AEC_PWM_FREQ_NEAR             =  0x0000003A,	/*近光灯的时钟频率*/
	AEC_PWM_LIMIT_CURRENT      	  =  0x0000003B,	/*限制红外灯电流*/
	AEC_IRL_MAX_FAR_DUTY		  =  0x0000003D,	/*设置占空比最大值[0, 100]，默认值100，远光灯*/
	AEC_IRL_MAX_NEAR_DUTY		  =  0x0000003E,	/*设置占空比最大值[0, 100]，默认值100，近光灯*/
	AEC_IRL_MIN_FAR_DUTY		  =  0x0000003F,	/*设置占空比最小值[0, 100]，默认值100，远光灯*/
	AEC_IRL_MIN_NEAR_DUTY		  =  0x00000040,	/*设置占空比最小值[0, 100]，默认值100，近光灯*/

	//iris related
	AEC_IRIS_DTC_EN						= 0x00000050,
	AEC_IRIS_DTC_SCHEME_SELECT			= 0x00000051,
	AEC_IRIS_DTC_ADP_STEP_OVER_EXPOSE	= 0x00000052,
	AEC_IRIS_DTC_ADP_STEP_SHOCK			= 0x00000053,
	AEC_IRIS_DTC_CUR_SENSITY	        = 0x00000054,

	//AF related
	AEC_AN41908A_CUR_LEVEL				= 0x00000060,		/*获取电动镜头当前光圈大小(提供AF 曲线测试)*/
	AEC_GET_GAIN_AF_TEST				= 0x00000061,		/*设置获取增益等级(提供AF 曲线测试)*/
	AEC_SET_USER_SHUT_TIME				= 0x00000062,		/*设置曝光时间1-40ms 实时生效(提供AF 曲线测试)*/
	//hdr related
	//remember :when update rate(include max and min rate), please refresh max shut to update max_expo_code
	AEC_SET_HDR_L_S1_MAX_RATE			= 0x00000070,		/*长帧与S1帧最大曝光比例,256代表1*/
	AEC_SET_HDR_L_S1_MIN_RATE			= 0x00000071,		/*长帧与S1帧最小曝光比例,256代表1*/
	AEC_SET_HDR_S1_S2_MAX_RATE			= 0x00000072,		/*S1与S2帧最大曝光比例,256代表1*/
	AEC_SET_HDR_S1_S2_MIN_RATE			= 0x00000073,		/*S1与S2帧最小曝光比例,256代表1*/
	AEC_SET_HDR_S2_S3_MAX_RATE			= 0x00000074,		/*S2与S3帧最大曝光比例,256代表1*/
	AEC_SET_HDR_S2_S3_MIN_RATE			= 0x00000075,		/*S2与S3帧最小曝光比例,256代表1*/
	
	AEC_SHUET_MUL_VAL                   = 0x00000076,
	AEC_GAIN_MUL_VAL                    = 0x00000077,
	AEC_EXPO_SENSITIVITY_ADJ_EN			= 0x00000100, 		/*曝光灵敏度自适应调整模块开关,【0，1】,默认开启*/
	AEC_EXPO_SPEEDUP_EN					= 0x00000104,		/*曝光加速方案使能开关，[0,1],默认开启*/
	AEC_EXPO_SPEEDUP_LEVEL				= 0x00000105,		/*曝光加速等级[0,100],默认为50*/

	//光圈延迟调整参数
	AEC_IRIS_DELAY_ADJ_EN				= 0x00000110,		/*自动光圈延迟调整模块使能开关,[0 1],默认关闭*/
	AEC_IRIS_DELAY_ADJ_TH				= 0x00000111,		/*自动光圈延迟启动时间阈值，单位s，默认100s*/
	AEC_IRIS_DELAY_STAT_TH				= 0x00000112, 		/*自动光圈进入延迟调整状态时间阈值，单位s，默认16s*/
	AEC_IRIS_DELAY_STAT_BREAK_TH		= 0x00000113,		/*自动光圈退出延迟调整状态时间阈值，单位s，默认6s*/

	/**************************算法内部参数，算法人员使用******************************/
	AEC_SPEED_OUT_RNG_TH          =  0x00010000,    /*亮度超出目标区间时间阈值(秒)，[0, 50]，默认6*/
	AEC_SPEED_HOLD_TH             =  0x00010001,    /*亮度持续不变时间阈值(秒)，[0, 10]，默认2*/
	AEC_SPEED_IN_RNG_TH           =  0x00010002,    /*亮度持续落入目标区间时间阈值(秒)，[0, 10]，默认3*/
	AEC_SPEED_GAIN_DB_TH          =  0x00010003,    /*关闭灵敏度控制增益阈值，[0, 50]，默认20*/
	AEC_DR_YRA_LUM_STABLE_CNT_TH  =  0x00010005,    /*亮度稳定时间阈值，[2, 20]，默认8*/

	AEC_DEBUG_PIN_KEY             =  0x80000000,    /*debug_pin*/
	AEC_DEBUG_LEVEL               =  0x80000001,    /*打印级别*/
	AEC_GET_LIB_INFO			  =  0x00006666		/*曝光库版本控制*/
}AEC_PARAM_KEY;


/*外部相关参数*/
/*供电频率*/
typedef enum _LIGHT_FREQ_
{
	LIGHT_FREQ_50HZ             =	1,	  	  /*供电频率为50hz*/
	LIGHT_FREQ_60HZ	 			=	2		  /*供电频率为60hz*/	
}LIGHT_FREQ;

/*曝光模式*/
typedef enum _AEC_MODE 
{	
	AEC_SHUT_GAIN               =	0,        /* 快门增益模式*/
	AEC_IRIS_SHUT_GAIN          =	1,        /* 光圈快门增益模式*/
	AEC_IRIS_ONLY               =	2,        /* 仅光圈模式,p_iris不能使用这个模式*/
	AEC_MODE_ALL                              /* 模式总数*/
}AEC_MODE;

/*曝光状态*/
typedef enum 
{
	EXPOSE_TURNING          	=	0,        /*曝光正在调整*/
	EXPOSE_STABLE               =	1,        /*曝光已稳定*/
	EXPOSE_REACH_LIMIT    		=	2         /*曝光达到极限*/
}EXPOSE_RESULT_TYPE;

/*红外灯类型*/
typedef enum _AEC_INFRARED_TYPE                       
{
	AEC_INFRARED_NEAR           =	0,		  /*红外近灯*/
	AEC_INFRARED_FAR            =	1,	      /*红外远灯*/
	AEC_INFRARED_TYPE_ALL       =	2		  /*红外灯类型总数*/
}AEC_INFRARED_TYPE;

/*红外灯组数类型*/
typedef enum _INFRARED_GRP_TYPE               
{
	ONE_GROUP   				=	1,		  /*支持一组红外灯*/
	TWO_GROUP  					=	2		  /*支持两组红外灯*/
}INFRARED_GRP_TYPE;

/*红外限流类型*/
typedef enum _LIMIT_INFRARED_CURRENT					
{
	LIMIT_INFRARED_DISABLE 		=	0,	      /*不需要限制红外灯电流*/
	LIMIT_INFRARED_ENABLE  		=	1		  /*需要限制红外灯电流*/
}LIMIT_INFRARED_CURRENT;

/*光源模式*/
typedef enum _AEC_LIGHT_MODE
{
	AEC_NORM_LIGHT              =	0,        /*自然光*/
	AEC_ARTIFICIAL_LIGHT        =	1,        /*人造光*/
	AEC_ARTIFICIAL_NO_FLICKER   =	2         /*非闪烁人造光*/
}AEC_LIGHT_MODE;

/*自动光圈自适应模块方案*/
typedef enum _IRIS_DETECT_SCHEME_               
{
	IRIS_DETECT_SCHEME_A     	=	1,		  /*自动光圈灵敏度自适应调节*/
	IRIS_DETECT_SCHEME_B     	=	2		  /*出现异常时直接切成手动光圈模式*/
}IRIS_DETECT_SCHEME;

typedef enum _AEC_PLATFORM_TYPE_               
{
	PLATFORM_TYPE_E1   		    =	1,	      /*E1平台*/
	PLATFORM_TYPE_E4   		    =	2,		  /*E4平台*/
	PLATFORM_TYPE_E6   		    =	3,		  /*E6平台*/
	
}AEC_PLATFORM_TYPE;

/*传感器类型*/
typedef enum _AEC_SENSOR_TYPE
{
    //ov
	CMOS_OS02E10_LINEAR			=	0xe,
	
	CMOS_OS04C10_LINEAR		  	=   0x10,
	CMOS_OS04C10_2to1_LINE	  	=   0x11,
  //apt
	
    //others
	SENSOR_TYPE_ALL
}AEC_SENSOR_TYPE;

/*光圈镜头类型*/
typedef enum _AEC_APERTURE_TYPE
{
	APERTURE_SLOW               =	0,         /* DC驱动 */
	APERTURE_FAST               =	1,         /* DC驱动 */
	APERTURE_AN41908A           =	2,         /* DC驱动 */
    APERTURE_MV13VP288IR     	=	3,         /* P-iris, Tarom MV13VP288IR */
    APERTURE_LM25JC5MMIR     	=	4,         /* P-iris, Kowa LM25JC5MMIR */
    APERTURE_HV3816P8MPIR   	=	5,         /* P-iris,HIK HV3816P8MPIR*/
    APERTURE_TYPE_ALL                          /* 光圈类型总数*/
}AEC_APERTURE_TYPE;

/*分辨率帧率*/
/*警告!!!: 按照帧率分组,禁止随便放置,方便帧率和慢快门检测函数判断*/
/*添加不同帧率后，更改帧率和慢快门检测函数*/
typedef enum _AEC_RESOLUTION_FRAMERATE_TYPE
{
	//**********以下为25fps集合
	AEC_25FPS_START = 0x60,
	AEC_720P_25FPS,
	AEC_960P_25FPS,
	AEC_1080P_25FPS,
	AEC_QXGA_25FPS,
	AEC_4M_25FPS,//2688*1536@25fps
	AEC_5M_25FPS,//2560*1920@25fps
	AEC_6M_25FPS,//3072*2048
	AEC_2160P_25FPS,
	AEC_2048x1536_25FPS,
	AEC_2560X1440_25FPS,
	AEC_25FPS_END,

	//**********以下为30fps集合
	AEC_30FPS_START = 0x100,
	AEC_720P_30FPS,
	AEC_960P_30FPS,
	AEC_1080P_30FPS,
	AEC_QXGA_30FPS,
	AEC_4M_30FPS,//2688*1536@30fps
	AEC_5M_30FPS,//2560*1920@30fps
	AEC_6M_30FPS,//3072*2048
	AEC_2160P_30FPS,
	AEC_2048x1536_30FPS,
	AEC_2560X1440_30FPS,
	AEC_30FPS_END,

	//***********以下为50fps集合
	AEC_50FPS_START = 0x200,
	AEC_1080P_50FPS,
	AEC_2160P_50FPS,
	AEC_50FPS_END,

	//************以下为60fps集合
	AEC_60FPS_START = 0x300,
	AEC_1080P_60FPS,
	AEC_2160P_60FPS,
	AEC_60FPS_END,

	//***********以下为20fps集合
	AEC_20FPS_START = 0x400,
	AEC_2048x1536_20FPS,
	AEC_20FPS_END,

	//***********以下为15fps集合
	AEC_15FPS_START = 0x500,
	AEC_2048x1536_15FPS,
	AEC_15FPS_END,

	//***********以下为12.5fps集合
	AEC_12P5FPS_START = 0x600,
	AEC_2048x1536_12P5FPS,
	AEC_12P5FPS_END,

	AEC_RESOLUTION_ALL 
}AEC_RESOLUTION_FRAMERATE_TYPE;


/*******************************************************************************************************************************
* 结构体声明
*******************************************************************************************************************************/
/*曝光状态参数*/
typedef struct _EXPOSE_STATUS_PARAMS_
{
	unsigned int 				aec_gain_level; 				//0-100
	unsigned int 				aec_expose_time[MAX_HDR_FRM];   //以微秒为单位
	unsigned int 				aec_expose_line[MAX_HDR_FRM];	//曝光行,排列为线性/长帧，短帧1，短帧2，短帧3
	unsigned int 				aec_y_ref;  					//当前参考亮度
	EXPOSE_RESULT_TYPE 			aec_status_result; 				//当前曝光状态	
}EXPOSE_STATUS_PARAMS;

/*内外交互参数*/
typedef struct _AEC_PROCESS_PARAM
{
	int              		    average_y;           			//gamma前的亮度平均值
	unsigned int     		    *hist;                    		//直方图指针，目前使用的是H3A块亮度的直方图
	EXPOSE_STATUS_PARAMS  		aec_ret_params;  				//曝光状态
}AEC_PROCESS_PARAM;

/*外部参数配置*/
typedef struct _AEC_PARAM 
{
	/*可配置的参数*/
    AEC_PLATFORM_TYPE           		ae_platform_type;             /*平台类型*/      
	AEC_SENSOR_TYPE   				    ae_sensor_type;               /*传感器类型*/    
	AEC_RESOLUTION_FRAMERATE_TYPE       ae_resolution_framerate;      /*分辨率帧率类型*/
	LIGHT_FREQ						    light_freq;					  /*制式*/
	int                                 vmax_val;
} AEC_PARAM;

/*外部设置参考亮度表*/
typedef struct _AEC_Y_REF_LIST
{
	int y_ref[15];			/*每6db一个参考亮度，最大支持到84db*/
}AEC_Y_REF_LIST;


/*******************************************************************************************************************************
* 接口函数
*******************************************************************************************************************************/
/*******************************************************************************************************************************
* 功  能：获取所需内存大小
* 参  数：param - 参数结构指针
* 返回值：返回错误码
* 备  注：参数结构中 buffer_size变量用来表示所需内存大小
*******************************************************************************************************************************/
HRESULT AEC_GetMemSize(AEC_PARAM *aec_param, MEM_TAB mem_tab[HIK_AE_MTAB_NUM]);

/******************************************************************************
* 功  能：创建ae句柄
* 参  数：aec_param     ----外部配置文件句柄
*         
* 返回值：NULL：失败
* 备  注:创建之前要先对ae_sensor_type，capt_fd，v4l2_std赋值
*                如果是9d131还需对hikio_fd赋值
******************************************************************************/
HRESULT AEC_Create(AEC_PARAM *aec_param, MEM_TAB mem_tab[HIK_AE_MTAB_NUM], void **hAeHandle);

/******************************************************************************
* 功  能：执行ae算法
* 参  数：average_y  ---当前帧的平均亮度
*         hAeHanle   ---ae句柄
*         
* 返回值：0:执行失败  1:执行成功
* 备  注：ae算法的执行过程
******************************************************************************/
HRESULT AEC_Process(void * hAeHandle,AEC_PROCESS_PARAM *aec_process_param);

/******************************************************************************
* 功  能：获取ae当前参数
* 参  数：hAeHandle      -IO 模块句柄
*         param_key   -I 参数键
*         param_val   -I 参数键值
* 返回值：0:执行失败  1:执行成功
* 备  注：设置ae参数之前先要获取当前参数配置
******************************************************************************/
HRESULT AEC_GetKeyParam(void *hAeHandle,AEC_PARAM_KEY param_key,int *param_val);

/******************************************************************************
* 功  能：设置ae当前参数 
* 参  数：hAeHandle      -IO 模块句柄
*         param_key   -I 参数键
*         param_val   -O 参数键值
* 返回值：0:执行失败  1:执行成功
* 备  注：对ae的参数配置都要通过该函数来进行
******************************************************************************/
HRESULT AEC_SetKeyParam(void *hAeHandle,AEC_PARAM_KEY param_key,int param_val);

/******************************************************************************
* 功  能：设置输出内部状态信息的回调函数指针
* 参  数：handle        -IO XXX模块句柄
*         chan_id       -I 设置通道号id
*         callback_parm -I 
回调函数参数数据指针，实现在回调函数中调用库外部数据，如不需要可填NULL
*         callback_func -I 回调函数指针，形式为void (*debug_info_callback)(
void *debug_info, U32 size, void *callback_param);
* 返回值：返回状态码
* 备  注：函数内部将callback_func设置为回调函数指针
********************************************************************************/
HRESULT AEC_SetDebugInfoCallback(void  *hAeHandle, unsigned short   chan_id,  void  *callback_param, void  *callback_func);

/******************************************************************************
* 功  能：获取当前ae库版本信息
* 参  数：无
* 返回值：返回版本信息
* 备  注：版本信息格式为：版本号＋年（7位）＋月（4位）＋日（5位）
*         其中版本号为：主版本号（6位）＋子版本号（5位）＋修正版本号（5位）

******************************************************************************/
HRESULT AEC_GetVersion();

/******************************************************************************
* 功  能：删除ae资源
* 参  数：hAeHandle      -IO 模块句柄

* 返回值：0:执行失败  1:执行成功
* 备  注:
******************************************************************************/
HRESULT AEC_Delete(void           *hAeHandle);

/******************************************************************************
* 功  能：外部设置不同增益级下(对应参考亮度50)，图像参考亮度
* 参  数：hAeHanle           ---ae句柄
*         y_ref_list  ---参考亮度列表
* 返回值：0:执行失败  1:执行成功
* 备  注：ae算法的执行过程
******************************************************************************/
HRESULT AEC_SetYRef(void  *hAeHandle,  AEC_Y_REF_LIST   *y_ref_list);

/******************************************************************************
* 功  能：删除曝光句柄
* 参  数：无
* 返回值：0:执行失败  1:执行成功
* 备  注：
*        
******************************************************************************/
HRESULT AEC_Delete(void           *hAeHandle);

#ifdef __cplusplus
}
#endif 

#endif  /* _AE_LIB_H_ */

