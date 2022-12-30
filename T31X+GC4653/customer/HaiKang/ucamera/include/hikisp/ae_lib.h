/***********************************************************************
* 
* ��Ȩ��Ϣ����Ȩ���� (c) 2013, ���ݺ����������ּ������޹�˾, ��������Ȩ��
* 
* �ļ����ƣ�ae.h
* �ļ���ʶ��HIKVISION
* ժ    Ҫ����������
*  
*
* *��ǰ�汾��1.0.0(h1ƽ̨)
* ��    ��: ����
* ��    �ڣ�2016��6��30��
* ��    ע: �ع�����ӿ�����
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
* �ļ��汾��ʱ��궨��
*******************************************************************************************************************************/
/* ��ǰ�汾��*/
#define HIK_AE_MAJOR_VERSION        	 5   		 /* ���汾�ţ��ӿڸĶ����������ӡ��ܹ����ʱ���������63 */
#define HIK_AE_SUB_VERSION               4   		 /* �Ӱ汾�ţ������Ż����ֲ��ṹ����ʱ���������31 */
#define HIK_AE_REVISION_VERSION     	 30   		 /* �����汾�ţ�����bug����������31 */

/* �汾����*/
#define HIK_AE_VER_YEAR             	 16          /* ��*/
#define HIK_AE_VER_MONTH           		 9           /* ��*/
#define HIK_AE_VER_DAY             		 30          /* ��*/


/*******************************************************************************************************************************
* �궨��
*******************************************************************************************************************************/
#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef int HRESULT;
#endif  /*_HRESULT_DEFINED*/

/* �����붨�� */
#define HIK_AE_LIB_S_OK			    0x00000000      /* �ɹ�*/		    
#define HIK_AE_LIB_S_FAIL		    0x80000001      /* ʧ��*/		    
#define HIK_AE_LIB_E_PARA_NULL	    0x80000002     	/* ����ָ��Ϊ��*/
#define HIK_AE_LIB_E_MEM_NULL	    0x80000003	    /* �ڴ�Ϊ��*/
#define HIK_AE_LIB_E_MEM_OVER       0x80000004      /* �ڴ����     */

#define HIK_AE_MTAB_NUM                 1      	    /* �����ڴ�*/
#define MAX_HDR_FRM			       		4        	/* hdr���֧��֡��*/

/*������ֵ*/
typedef enum _AEC_PARAM_KEY
{
    AEC_AE_SWITCH_KEY             =  0x00000001,    /* AE����*/
	AEC_Y_REF_RANGE_KEY           =  0x00000002,    /*�ο����ȼ���[0 11]*/
	AEC_LIGHT_FREQ_KEY            =  0x00000003,    /*����Ƶ��,1:50HZ;2:60HZ*/
	AEC_AE_MODE_KEY               =  0x00000004,    /* aeģʽ*/
	AEC_EFFECT_INTERVAL_KEY       =  0x00000005,    /*��Ч���֡��*/
	AEC_RESOLUTION_FRAMERATE_KEY  =  0x00000006,    /*�ֱ���֡��*/ 
	AEC_SHUT_MAX_KEY              =  0x00000008,    /*��ع�ʱ��*/
	AEC_SHUT_MIN_KEY              =  0x00000009,    /*����ع�ʱ��*/
	AEC_GAIN_MAX_KEY              =  0x0000000A,    /*�������ֵ(9p031:128) (033:255)*/
	AEC_GAIN_MIN_KEY              =  0x0000000B,    /*������Сֵ*/
	AEC_Y_REF_KEY                 =  0x0000000C,    /*�ο����ȼ���*/   
	AEC_SENSOR_TYPE_KEY           =  0x0000000D,    /*ǰ������*/ 
    AEC_GAIN_KEY                  =  0x0000000E,    /*��ǰ����*/ 
	AEC_SET_GAIN_REG              =  0x0000000F,    /*��������Ĵ���*/
	AEC_SET_SHUT_REG              =  0x00000010,    /*���ÿ��żĴ���*/
	AEC_SET_APER_TYPE             =  0x00000011,    /*���ù�Ȧ����*/         
	AEC_SET_IRIS_PARAM            =  0x00000012,    /*���ù�Ȧ�ο���ѹ�Ȳ���*/
	AEC_GAIN_DB_KEY               =  0x00000013,    /*��ǰ����DB��*/
	AEC_GAIN_ISO_KEY              =  0x00000014,    /*��ǰ���汶��*100, ����R2ƽ̨�������ȡ*/
	AEC_GAIN_ISP_KEY              =  0x00000015,    /*��ǰisp���棬����R2ƽ̨��ISP�����ȡ*/
	AEC_GAIN_ISP_SFT_KEY       	  =  0x00000016,    /*ISP����ľ��ȣ�����R2ƽ̨�����澫�Ȼ�ȡ*/    
	AEC_SET_AN41908A_MAX_VAL  	  =  0x00000017,    /*���õ�ǰAN41908����Ȧֵ*/
	AEC_SET_AN41908A_MIN_VAL  	  =  0x00000018,    /*���õ�ǰAN41908��С��Ȧֵ*/
	AEC_PLATFORM_TYPE_KEY         =  0x00000019,    /*ƽ̨�������ü�ֵ*/
	AEC_SET_VMAX_KEY              =  0x0000001A,    /*VMAX���ü�ֵ*/

	AEC_CUR_LONG_EXPOSURE         =  0x0000001B,    /*�����ŵ�ǰ����*/
	
	AEC_SET_CTL_SPEED_EN          =  0x00000020,    /*���������ȿ��ƿ��أ�0���أ�1������Ĭ�Ͽ�*/
	
	AEC_SET_DR_YRA_EN             =  0x00000021,    /*���ø��ݶ�̬��Χ���ڿ��أ�0���أ�1������Ĭ�Ϲ�*/
	AEC_SET_DR_YRA_LEVEL          =  0x00000022,    /*���ø��ݶ�̬�ȼ���[0, 100]��Ĭ��50*/
	AEC_SET_LL_YRA_EN             =  0x00000023,    /*���ø����ն�ˮƽ���ڿ��أ�0���أ�1������Ĭ�Ͽ�*/
	AEC_SET_GA_YRA_EN			  =  0x00000025,	/*���ø�����������ο����ȿ��أ�0:�أ�1:����Ĭ�Ϲر�*/
	AEC_CUR_EXPOSURE_STATUS       =  0x00000026,    /*��ȡ�ع������ֵ*/
	AEC_EXPO_FPS_P_N 			  =  0x00000027,    /*�ⲿ������ʽ0ΪP��1ΪN��*/
	 //irl related
    AEC_IRL_CTL_EN                =  0x00000030,    /*��������ȿ���ʹ�ܿ��أ� 0: �أ�1������Ĭ�Ϲ�*/
	AEC_IRL_MIN_DUTY              =  0x00000031,    /*����ռ�ձ���Сֵ[0, 100], Ĭ��ֵ0*/
	AEC_IRL_MID_DUTY              =  0x00000032,    /*����ռ�ձ��м�ֵ[0, 100], Ĭ��ֵ100*/
	AEC_IRL_MAX_DUTY              =  0x00000033,    /*����ռ�ձ����ֵ[0, 100]��Ĭ��ֵ100*/
	AEC_IRL_MID_GAIN              =  0x00000034,    /*���������м�ֵ[0, 100], Ĭ��ֵ40*/
	AEC_IRL_MANUAL_DUTY_FAR       =  0x00000035,    /*�����(Զ��)���ȿ����ֶ�����ռ�ձȣ�[0, 100],*/
	AEC_IRL_MANUAL_DUTY_NEAR      =  0x00000036,    /*�����(����)���ȿ����ֶ�����ռ�ձȣ�[0, 100],*/
	
	AEC_IRL_PWM_GRP_TYPE	      =  0x00000037,    /*�����֧�ֵ�����*/
	AEC_PWM_MODE_SWITCH           =  0x00000038,    /*�л����Ϻ���ƿ��Ʋ���*/
	AEC_PWM_FREQ_FAR              =  0x00000039,    /*Զ�Ƶ�ʱ��Ƶ��*/
	AEC_PWM_FREQ_NEAR             =  0x0000003A,	/*����Ƶ�ʱ��Ƶ��*/
	AEC_PWM_LIMIT_CURRENT      	  =  0x0000003B,	/*���ƺ���Ƶ���*/
	AEC_IRL_MAX_FAR_DUTY		  =  0x0000003D,	/*����ռ�ձ����ֵ[0, 100]��Ĭ��ֵ100��Զ���*/
	AEC_IRL_MAX_NEAR_DUTY		  =  0x0000003E,	/*����ռ�ձ����ֵ[0, 100]��Ĭ��ֵ100�������*/
	AEC_IRL_MIN_FAR_DUTY		  =  0x0000003F,	/*����ռ�ձ���Сֵ[0, 100]��Ĭ��ֵ100��Զ���*/
	AEC_IRL_MIN_NEAR_DUTY		  =  0x00000040,	/*����ռ�ձ���Сֵ[0, 100]��Ĭ��ֵ100�������*/

	//iris related
	AEC_IRIS_DTC_EN						= 0x00000050,
	AEC_IRIS_DTC_SCHEME_SELECT			= 0x00000051,
	AEC_IRIS_DTC_ADP_STEP_OVER_EXPOSE	= 0x00000052,
	AEC_IRIS_DTC_ADP_STEP_SHOCK			= 0x00000053,
	AEC_IRIS_DTC_CUR_SENSITY	        = 0x00000054,

	//AF related
	AEC_AN41908A_CUR_LEVEL				= 0x00000060,		/*��ȡ�綯��ͷ��ǰ��Ȧ��С(�ṩAF ���߲���)*/
	AEC_GET_GAIN_AF_TEST				= 0x00000061,		/*���û�ȡ����ȼ�(�ṩAF ���߲���)*/
	AEC_SET_USER_SHUT_TIME				= 0x00000062,		/*�����ع�ʱ��1-40ms ʵʱ��Ч(�ṩAF ���߲���)*/
	//hdr related
	//remember :when update rate(include max and min rate), please refresh max shut to update max_expo_code
	AEC_SET_HDR_L_S1_MAX_RATE			= 0x00000070,		/*��֡��S1֡����ع����,256����1*/
	AEC_SET_HDR_L_S1_MIN_RATE			= 0x00000071,		/*��֡��S1֡��С�ع����,256����1*/
	AEC_SET_HDR_S1_S2_MAX_RATE			= 0x00000072,		/*S1��S2֡����ع����,256����1*/
	AEC_SET_HDR_S1_S2_MIN_RATE			= 0x00000073,		/*S1��S2֡��С�ع����,256����1*/
	AEC_SET_HDR_S2_S3_MAX_RATE			= 0x00000074,		/*S2��S3֡����ع����,256����1*/
	AEC_SET_HDR_S2_S3_MIN_RATE			= 0x00000075,		/*S2��S3֡��С�ع����,256����1*/
	
	AEC_SHUET_MUL_VAL                   = 0x00000076,
	AEC_GAIN_MUL_VAL                    = 0x00000077,
	AEC_EXPO_SENSITIVITY_ADJ_EN			= 0x00000100, 		/*�ع�����������Ӧ����ģ�鿪��,��0��1��,Ĭ�Ͽ���*/
	AEC_EXPO_SPEEDUP_EN					= 0x00000104,		/*�ع���ٷ���ʹ�ܿ��أ�[0,1],Ĭ�Ͽ���*/
	AEC_EXPO_SPEEDUP_LEVEL				= 0x00000105,		/*�ع���ٵȼ�[0,100],Ĭ��Ϊ50*/

	//��Ȧ�ӳٵ�������
	AEC_IRIS_DELAY_ADJ_EN				= 0x00000110,		/*�Զ���Ȧ�ӳٵ���ģ��ʹ�ܿ���,[0 1],Ĭ�Ϲر�*/
	AEC_IRIS_DELAY_ADJ_TH				= 0x00000111,		/*�Զ���Ȧ�ӳ�����ʱ����ֵ����λs��Ĭ��100s*/
	AEC_IRIS_DELAY_STAT_TH				= 0x00000112, 		/*�Զ���Ȧ�����ӳٵ���״̬ʱ����ֵ����λs��Ĭ��16s*/
	AEC_IRIS_DELAY_STAT_BREAK_TH		= 0x00000113,		/*�Զ���Ȧ�˳��ӳٵ���״̬ʱ����ֵ����λs��Ĭ��6s*/

	/**************************�㷨�ڲ��������㷨��Աʹ��******************************/
	AEC_SPEED_OUT_RNG_TH          =  0x00010000,    /*���ȳ���Ŀ������ʱ����ֵ(��)��[0, 50]��Ĭ��6*/
	AEC_SPEED_HOLD_TH             =  0x00010001,    /*���ȳ�������ʱ����ֵ(��)��[0, 10]��Ĭ��2*/
	AEC_SPEED_IN_RNG_TH           =  0x00010002,    /*���ȳ�������Ŀ������ʱ����ֵ(��)��[0, 10]��Ĭ��3*/
	AEC_SPEED_GAIN_DB_TH          =  0x00010003,    /*�ر������ȿ���������ֵ��[0, 50]��Ĭ��20*/
	AEC_DR_YRA_LUM_STABLE_CNT_TH  =  0x00010005,    /*�����ȶ�ʱ����ֵ��[2, 20]��Ĭ��8*/

	AEC_DEBUG_PIN_KEY             =  0x80000000,    /*debug_pin*/
	AEC_DEBUG_LEVEL               =  0x80000001,    /*��ӡ����*/
	AEC_GET_LIB_INFO			  =  0x00006666		/*�ع��汾����*/
}AEC_PARAM_KEY;


/*�ⲿ��ز���*/
/*����Ƶ��*/
typedef enum _LIGHT_FREQ_
{
	LIGHT_FREQ_50HZ             =	1,	  	  /*����Ƶ��Ϊ50hz*/
	LIGHT_FREQ_60HZ	 			=	2		  /*����Ƶ��Ϊ60hz*/	
}LIGHT_FREQ;

/*�ع�ģʽ*/
typedef enum _AEC_MODE 
{	
	AEC_SHUT_GAIN               =	0,        /* ��������ģʽ*/
	AEC_IRIS_SHUT_GAIN          =	1,        /* ��Ȧ��������ģʽ*/
	AEC_IRIS_ONLY               =	2,        /* ����Ȧģʽ,p_iris����ʹ�����ģʽ*/
	AEC_MODE_ALL                              /* ģʽ����*/
}AEC_MODE;

/*�ع�״̬*/
typedef enum 
{
	EXPOSE_TURNING          	=	0,        /*�ع����ڵ���*/
	EXPOSE_STABLE               =	1,        /*�ع����ȶ�*/
	EXPOSE_REACH_LIMIT    		=	2         /*�ع�ﵽ����*/
}EXPOSE_RESULT_TYPE;

/*���������*/
typedef enum _AEC_INFRARED_TYPE                       
{
	AEC_INFRARED_NEAR           =	0,		  /*�������*/
	AEC_INFRARED_FAR            =	1,	      /*����Զ��*/
	AEC_INFRARED_TYPE_ALL       =	2		  /*�������������*/
}AEC_INFRARED_TYPE;

/*�������������*/
typedef enum _INFRARED_GRP_TYPE               
{
	ONE_GROUP   				=	1,		  /*֧��һ������*/
	TWO_GROUP  					=	2		  /*֧����������*/
}INFRARED_GRP_TYPE;

/*������������*/
typedef enum _LIMIT_INFRARED_CURRENT					
{
	LIMIT_INFRARED_DISABLE 		=	0,	      /*����Ҫ���ƺ���Ƶ���*/
	LIMIT_INFRARED_ENABLE  		=	1		  /*��Ҫ���ƺ���Ƶ���*/
}LIMIT_INFRARED_CURRENT;

/*��Դģʽ*/
typedef enum _AEC_LIGHT_MODE
{
	AEC_NORM_LIGHT              =	0,        /*��Ȼ��*/
	AEC_ARTIFICIAL_LIGHT        =	1,        /*�����*/
	AEC_ARTIFICIAL_NO_FLICKER   =	2         /*����˸�����*/
}AEC_LIGHT_MODE;

/*�Զ���Ȧ����Ӧģ�鷽��*/
typedef enum _IRIS_DETECT_SCHEME_               
{
	IRIS_DETECT_SCHEME_A     	=	1,		  /*�Զ���Ȧ����������Ӧ����*/
	IRIS_DETECT_SCHEME_B     	=	2		  /*�����쳣ʱֱ���г��ֶ���Ȧģʽ*/
}IRIS_DETECT_SCHEME;

typedef enum _AEC_PLATFORM_TYPE_               
{
	PLATFORM_TYPE_E1   		    =	1,	      /*E1ƽ̨*/
	PLATFORM_TYPE_E4   		    =	2,		  /*E4ƽ̨*/
	PLATFORM_TYPE_E6   		    =	3,		  /*E6ƽ̨*/
	
}AEC_PLATFORM_TYPE;

/*����������*/
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

/*��Ȧ��ͷ����*/
typedef enum _AEC_APERTURE_TYPE
{
	APERTURE_SLOW               =	0,         /* DC���� */
	APERTURE_FAST               =	1,         /* DC���� */
	APERTURE_AN41908A           =	2,         /* DC���� */
    APERTURE_MV13VP288IR     	=	3,         /* P-iris, Tarom MV13VP288IR */
    APERTURE_LM25JC5MMIR     	=	4,         /* P-iris, Kowa LM25JC5MMIR */
    APERTURE_HV3816P8MPIR   	=	5,         /* P-iris,HIK HV3816P8MPIR*/
    APERTURE_TYPE_ALL                          /* ��Ȧ��������*/
}AEC_APERTURE_TYPE;

/*�ֱ���֡��*/
/*����!!!: ����֡�ʷ���,��ֹ������,����֡�ʺ������ż�⺯���ж�*/
/*��Ӳ�ͬ֡�ʺ󣬸���֡�ʺ������ż�⺯��*/
typedef enum _AEC_RESOLUTION_FRAMERATE_TYPE
{
	//**********����Ϊ25fps����
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

	//**********����Ϊ30fps����
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

	//***********����Ϊ50fps����
	AEC_50FPS_START = 0x200,
	AEC_1080P_50FPS,
	AEC_2160P_50FPS,
	AEC_50FPS_END,

	//************����Ϊ60fps����
	AEC_60FPS_START = 0x300,
	AEC_1080P_60FPS,
	AEC_2160P_60FPS,
	AEC_60FPS_END,

	//***********����Ϊ20fps����
	AEC_20FPS_START = 0x400,
	AEC_2048x1536_20FPS,
	AEC_20FPS_END,

	//***********����Ϊ15fps����
	AEC_15FPS_START = 0x500,
	AEC_2048x1536_15FPS,
	AEC_15FPS_END,

	//***********����Ϊ12.5fps����
	AEC_12P5FPS_START = 0x600,
	AEC_2048x1536_12P5FPS,
	AEC_12P5FPS_END,

	AEC_RESOLUTION_ALL 
}AEC_RESOLUTION_FRAMERATE_TYPE;


/*******************************************************************************************************************************
* �ṹ������
*******************************************************************************************************************************/
/*�ع�״̬����*/
typedef struct _EXPOSE_STATUS_PARAMS_
{
	unsigned int 				aec_gain_level; 				//0-100
	unsigned int 				aec_expose_time[MAX_HDR_FRM];   //��΢��Ϊ��λ
	unsigned int 				aec_expose_line[MAX_HDR_FRM];	//�ع���,����Ϊ����/��֡����֡1����֡2����֡3
	unsigned int 				aec_y_ref;  					//��ǰ�ο�����
	EXPOSE_RESULT_TYPE 			aec_status_result; 				//��ǰ�ع�״̬	
}EXPOSE_STATUS_PARAMS;

/*���⽻������*/
typedef struct _AEC_PROCESS_PARAM
{
	int              		    average_y;           			//gammaǰ������ƽ��ֵ
	unsigned int     		    *hist;                    		//ֱ��ͼָ�룬Ŀǰʹ�õ���H3A�����ȵ�ֱ��ͼ
	EXPOSE_STATUS_PARAMS  		aec_ret_params;  				//�ع�״̬
}AEC_PROCESS_PARAM;

/*�ⲿ��������*/
typedef struct _AEC_PARAM 
{
	/*�����õĲ���*/
    AEC_PLATFORM_TYPE           		ae_platform_type;             /*ƽ̨����*/      
	AEC_SENSOR_TYPE   				    ae_sensor_type;               /*����������*/    
	AEC_RESOLUTION_FRAMERATE_TYPE       ae_resolution_framerate;      /*�ֱ���֡������*/
	LIGHT_FREQ						    light_freq;					  /*��ʽ*/
	int                                 vmax_val;
} AEC_PARAM;

/*�ⲿ���òο����ȱ�*/
typedef struct _AEC_Y_REF_LIST
{
	int y_ref[15];			/*ÿ6dbһ���ο����ȣ����֧�ֵ�84db*/
}AEC_Y_REF_LIST;


/*******************************************************************************************************************************
* �ӿں���
*******************************************************************************************************************************/
/*******************************************************************************************************************************
* ��  �ܣ���ȡ�����ڴ��С
* ��  ����param - �����ṹָ��
* ����ֵ�����ش�����
* ��  ע�������ṹ�� buffer_size����������ʾ�����ڴ��С
*******************************************************************************************************************************/
HRESULT AEC_GetMemSize(AEC_PARAM *aec_param, MEM_TAB mem_tab[HIK_AE_MTAB_NUM]);

/******************************************************************************
* ��  �ܣ�����ae���
* ��  ����aec_param     ----�ⲿ�����ļ����
*         
* ����ֵ��NULL��ʧ��
* ��  ע:����֮ǰҪ�ȶ�ae_sensor_type��capt_fd��v4l2_std��ֵ
*                �����9d131�����hikio_fd��ֵ
******************************************************************************/
HRESULT AEC_Create(AEC_PARAM *aec_param, MEM_TAB mem_tab[HIK_AE_MTAB_NUM], void **hAeHandle);

/******************************************************************************
* ��  �ܣ�ִ��ae�㷨
* ��  ����average_y  ---��ǰ֡��ƽ������
*         hAeHanle   ---ae���
*         
* ����ֵ��0:ִ��ʧ��  1:ִ�гɹ�
* ��  ע��ae�㷨��ִ�й���
******************************************************************************/
HRESULT AEC_Process(void * hAeHandle,AEC_PROCESS_PARAM *aec_process_param);

/******************************************************************************
* ��  �ܣ���ȡae��ǰ����
* ��  ����hAeHandle      -IO ģ����
*         param_key   -I ������
*         param_val   -I ������ֵ
* ����ֵ��0:ִ��ʧ��  1:ִ�гɹ�
* ��  ע������ae����֮ǰ��Ҫ��ȡ��ǰ��������
******************************************************************************/
HRESULT AEC_GetKeyParam(void *hAeHandle,AEC_PARAM_KEY param_key,int *param_val);

/******************************************************************************
* ��  �ܣ�����ae��ǰ���� 
* ��  ����hAeHandle      -IO ģ����
*         param_key   -I ������
*         param_val   -O ������ֵ
* ����ֵ��0:ִ��ʧ��  1:ִ�гɹ�
* ��  ע����ae�Ĳ������ö�Ҫͨ���ú���������
******************************************************************************/
HRESULT AEC_SetKeyParam(void *hAeHandle,AEC_PARAM_KEY param_key,int param_val);

/******************************************************************************
* ��  �ܣ���������ڲ�״̬��Ϣ�Ļص�����ָ��
* ��  ����handle        -IO XXXģ����
*         chan_id       -I ����ͨ����id
*         callback_parm -I 
�ص�������������ָ�룬ʵ���ڻص������е��ÿ��ⲿ���ݣ��粻��Ҫ����NULL
*         callback_func -I �ص�����ָ�룬��ʽΪvoid (*debug_info_callback)(
void *debug_info, U32 size, void *callback_param);
* ����ֵ������״̬��
* ��  ע�������ڲ���callback_func����Ϊ�ص�����ָ��
********************************************************************************/
HRESULT AEC_SetDebugInfoCallback(void  *hAeHandle, unsigned short   chan_id,  void  *callback_param, void  *callback_func);

/******************************************************************************
* ��  �ܣ���ȡ��ǰae��汾��Ϣ
* ��  ������
* ����ֵ�����ذ汾��Ϣ
* ��  ע���汾��Ϣ��ʽΪ���汾�ţ��꣨7λ�����£�4λ�����գ�5λ��
*         ���а汾��Ϊ�����汾�ţ�6λ�����Ӱ汾�ţ�5λ���������汾�ţ�5λ��

******************************************************************************/
HRESULT AEC_GetVersion();

/******************************************************************************
* ��  �ܣ�ɾ��ae��Դ
* ��  ����hAeHandle      -IO ģ����

* ����ֵ��0:ִ��ʧ��  1:ִ�гɹ�
* ��  ע:
******************************************************************************/
HRESULT AEC_Delete(void           *hAeHandle);

/******************************************************************************
* ��  �ܣ��ⲿ���ò�ͬ���漶��(��Ӧ�ο�����50)��ͼ��ο�����
* ��  ����hAeHanle           ---ae���
*         y_ref_list  ---�ο������б�
* ����ֵ��0:ִ��ʧ��  1:ִ�гɹ�
* ��  ע��ae�㷨��ִ�й���
******************************************************************************/
HRESULT AEC_SetYRef(void  *hAeHandle,  AEC_Y_REF_LIST   *y_ref_list);

/******************************************************************************
* ��  �ܣ�ɾ���ع���
* ��  ������
* ����ֵ��0:ִ��ʧ��  1:ִ�гɹ�
* ��  ע��
*        
******************************************************************************/
HRESULT AEC_Delete(void           *hAeHandle);

#ifdef __cplusplus
}
#endif 

#endif  /* _AE_LIB_H_ */

