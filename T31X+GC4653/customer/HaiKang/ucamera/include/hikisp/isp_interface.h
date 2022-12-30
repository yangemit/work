/** @file isp_interface.h
*	@note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*	@brief ispģ�����ͷ�ļ�
*	
*	@author   yaojinbo
*	@date	  2015-05-26
*	@version  1.0
*	@note 
*	@note History:		  
*	@note	  <author>	 <time>    <version >	<desc>
*	@note	  yaojinbo	  2015-05-28  �޸�
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


// �����붨��
#define ISP_LIB_S_OK            0             //�ɹ�
#define ISP_LIB_S_FAIL          -1             //ʧ��
#define ISP_LIB_E_PARA_NULL	 	0x40000000    //����ָ��Ϊ��
#define ISP_LIB_E_MEM_NULL	   	0x40000001    //������ڴ�Ϊ��
#define ISP_LIB_E_MEM_OVER	   	0x40000002    //�ڴ����
#define ISP_LIB_E_PARAM_KEY	 	0x40000003    //����Ĳ�����ֵ
#define ISP_LIB_E_PARAM_VAL	 	0x40000004    //����Ĳ���ֵ

#define ISP_MOD_OPEN           1             /**ISPģ�鿪��*/
#define ISP_MOD_CLOSE          0             /**ISPģ��ر�*/

#define MAX_IR_LENS_GROUP (2)


// ���ò�����ֵ,���ڿ������͵ļ�ֵ������0��ʾ�أ�1��ʾ�������ܡ�
typedef enum _ISP_PARAM_KEY_
{
	/*======================ISP SYSTEM MODULES ======================*/
	/**000:��ʾISP��汾��Ϣ;	<Range>{}     				<Mode>G	*/
	ISP_LIB_VERSION_I = 0x0,
	
	/**001:ISP����;			<Range>{0,1}     						<Mode>G/S	*/
	ISP_EN_I,
	
	/**002:3A���� ;			<Range>{0,1}     						<Mode>G/S	*/
	ISP_3A_EN_I,
	
	/**003:sensor �����λ;	<Range>{1}     						<Mode>S	*/
	ISP_SENSOR_RESET_I,
	
	/**004:ISP freeze switch ;		<Range>{0,1} 0:������1:  ���� 	<Mode>S	*/
	ISP_FREEZE_EN_I,
	
	/**005:3A���� ;			<Range>[0-0xffffffff]     				<Mode>G/S	*/
	ISP_MODULE_EN_I,
	
	/*=========================AE MODULE =========================*/
	/**032:AE���� ;			<Range>{0,1}     						<Mode>G/S	*/
	ISP_AE_EN_I = 0x20,
	
	/**033:Ŀ�����ȵȼ�;	<Range>[0,100]    					<Mode>G/S	*/
	ISP_AE_BRIGHTNESS_LEVEL_I,
	
	/**034:��ǰ��������; 	<Range>[0,255]						<Mode>S  	*/
	ISP_AE_CUR_LUMA_VAL_I,
	
	/**035:����˸����;     	<Range>{0,1},1:60Hz,2:50Hz			<Mode>G/S */
	ISP_AE_ANTI_FLICKER_I,
	
	/**036:����ع�ʱ�䣬��λus; <Range>[10,1000000]		<Mode>G/S */
	ISP_AE_SHUT_SLOWEST_I,
	
	/**037:��С�ع�ʱ�䣬��λus; <Range>[10,1000000]		<Mode>G/S */
	ISP_AE_SHUT_FASTEST_I,
	
	/**038:�����Ź��ܿ���; <Range>{0,1}						<Mode>G/S */
	ISP_AE_SLOW_SHUT_EN_I,
	
	/**039:�������ȼ�; <Range>[0,100]						<Mode>G/S */
	ISP_AE_GAIN_MAX_LEVEL_I,
	
	/**040:��ǰ�ع�ʱ��; <Range>[10,1000000]					<Mode>G/S */
	ISP_AE_CUR_SHUT_TIME_I,
	
	/**041:��ǰ����ȼ�; <Range>[0,100]						<Mode>G/S */
	ISP_AE_CUR_GAIN_LEVEL_I,
	
	/**042:��ǰ���棬��λDB; <Range>[0-?]					<Mode>G	*/
	ISP_AE_GAIN_DB_I,
	
	/**043:��ǰ��Ȧ��С; <Range>[0x0,0x3ff]						<Mode>G 	*/
	ISP_AE_CUR_IRIS_VAL_I,
	
	/**044:��ǰ�ع�״̬; <Range>{0,1,2}0:������1:�ȶ���2:�ع��������ޡ�	<Mode>G 	*/
	ISP_AE_STATUS_I,
	
	/**045:��ǰ��ҹ״̬; <Range>{-1,0,1}0:��ҹ��1:�а��죬-1��������	<Mode>G	*/
	ISP_DAY_NIGHT_FLAG_I,
	
	/**046:��ҹ�л������ȣ�������Խ�ߣ��а���Խ����; <Range>[0,7]		<Mode>G/S	*/
	ISP_ICR_SENSITIVITY_I,
	
	/**047:���ù�Ȧ����; 			<Range>								<Mode>G/S */
	ISP_IRIS_PARAM_I,

	/**048: ��ǰ�Զ���̬�ж�״̬; 	<Range>{-1,0,1}0:�����ԣ�1:�п�̬��-1��������	<Mode>G	*/
	ISP_AUTO_WDR_FLAG_I,
	
	/*=========================AWB MODULE =========================*/
	/**080:�Զ���ƽ�⿪�� ;<Range>{0,1}     					<Mode>G/S	*/
	ISP_AWB_EN_I = 0x50,
	
	/**081:��ƽ������ٶȵȼ�;	<Range>[0,100]    			<Mode>G/S	*/
	ISP_AWB_SPEED_LEVEL_I,
	
	/**082:��ƽ��ģʽ;		<Range>ö������     				<Mode>G/S	*/
	ISP_AWB_MODE_I,
	
	/**083:�ֶ����û��߻�ȡRGB��ͨ������;	<Range>[0,0xffffffff]  <Mode>G/S	*/
	ISP_AWB_RGB_GAIN_I,
	
	/**084:���Ͷȵȼ�;		<Range>[0,100]     					<Mode>G/S	*/
	ISP_SATURATION_LEVEL_I,
	
	/**085:ɫ���ȼ�;		<Range>[0,100]     					<Mode>G/S	*/
	ISP_HUE_LEVEL_I,
	
	/**086:�Ҷȷ�Χ;		<Range> {0,1}  0:0-255,1:16-235 		<Mode>G/S	*/
	ISP_COLOR_STYLE_I,
	
	/**087:��ҹ�л�ʱ�ڰײ�ɫת������; <Range>{0,1}0���л������죬1���л���ҹ��	<Mode>G/S	*/
	ISP_DAY_NIGHT_MODE_I,

	/*=========================AF MODULE ==========================*/
	/**112:��ҹ�л�ʱ�ڰײ�ɫת������; <Range>{0,1}	<Mode>G/S	*/
	ISP_AF_EN_I = 0x70,
	
	/**113:����ͷ��ǰ���ʸ�ISP; <Range>						<Mode>G/S	*/
	ISP_AF_ZOOM_RATIO_I,

	/**114:����ͷ����У������;	<Range> [0,1]				<Mode>G/S	*/
	ISP_AF_LDC_MODE_I,

	/**115:ע��AF�ص�����	*/
	ISP_AEAF_PARAM_FUNC_REGISTER_I,

	/**116:���û��߻�ȡAF���ں���*/
	ISP_AEAF_WININFO_FUNC_REGISTER_I,

	/**117:��ȡAFD����*/
	ISP_GET_AEAF_PARAM_FUNC_I,

	/**118:��dsp��ȡ�����ڴ��׵�ַ*/
	ISP_SHARE_MEM_I,

	/*=========================IMAGE ENHANCE MODULE ==========================*/
	/**144:�Աȶȵȼ�;			<Range>[0,100]     				<Mode>yG/S	*/
	ISP_CONTRAST_LEVEL_I = 0x90,
	
	/**145:�񻯵ȼ�;			<Range>[0,100]    				<Mode>G/S	*/
	ISP_SHARPEN_LEVEL_I,
	
	/**146:����ģʽ;			<Range>{0,1,2}     				<Mode>G/S	*/
	ISP_NR_MODE_I,
	
	/**147:��ͨ����ģ��ȼ�;<Range>[0,100]     				<Mode>G/S	*/
	ISP_NR_LEVEL_I,
	
	/**148:������ȼ�;		<Range>[0,100]     				<Mode>G/S	*/
	ISP_SPATIAL_NR_LEVEL_I,
	
	/**149:ʱ����ǿ�ȼ�;	<Range>[0,100]     				<Mode>G/S	*/
	ISP_TEMPORAL_NR_LEVEL_I,
	
	/**150:��̬ģʽ;			<Range> �����ǽṹ��   	<Mode>G/S	*/
	ISP_WDR_MODE_I,
	
	/**151��̬�ȼ�:;			<Range>[0,100]   					<Mode>G/S	*/
	ISP_WDR_LEVEL_I,
	
	/**152:���ⲹ��ģʽ;<Range>{0,7} 0���رգ�1���ϣ�2���£�3����4���ң�5���У�6���Զ���7���Զ��屳�ⲹ��<Mode>G/S	*/
	ISP_BACK_LIGHT_MODE_I,
	
	/**153:ǿ�����ƿ���;		<Range>{0,1}     					<Mode>G/S	*/
	ISP_HIGH_LIGHT_CONTROL_EN_I,
	
	/**154:Ǭ�����Ƶȼ�;		<Range>[0,100]    				<Mode>G/S	*/
	ISP_HIGH_LIGHT_LEVEL_I,
	
	/**155:ȥ���ܿ���;		<Range>{0,1}     					<Mode>G/S	*/
	ISP_ANTI_FOG_EN_I,
	
	/**156:ȥ����ǿ��;		<Range>[0,100]     				<Mode>G/S	*/
	ISP_ANTI_FOG_LEVEL_I,
	
	/**157:GBCE����;				<Range>{0,1}     					<Mode>G/S	*/
	ISP_GBCE_EN_I,
	
	/**158:ǿ�����Ƶȼ�;		<Range>[0,100]     				<Mode>G/S	*/
	ISP_GBCE_LEVEL_I,
	
	/**159:smart irģʽ;			<Range>  ����Ϊ�ṹ��		<Mode>S	*/
	ISP_SMART_IR_ATTR_I,
	
	/**160:smart ir AEȨ�ؿ���;	<Range>[0,1],0,�رգ�1������:ʹ��Ĭ�ϲ���<Mode>G/S	*/
	ISP_SET_IR_AE_WEIGHT,
	
	/**161:�豸��װ�ĳ���ģʽ;	<Range>{0,1}     				<Mode>G/S	*/
	ISP_SET_SCENE_MODE_I,
	
	/**162:���������⿪��;<Range>{0,1}     					<Mode>G/S	*/
	ISP_STATIC_BPC_DETECT_I,
	
	/**163:��������У������;<Range>{0,1}     					<Mode>G/S	*/
	ISP_STATIC_BPC_COR_EN_I,
	
	/**164:��������У������;<Range>{0,1}     					<Mode>G/S	*/
	ISP_DYNAMIC_BPC_COR_EN_I,
	
	/**165:capture mode;				<Range>����Ϊö������	<Mode>G/S	*/
	ISP_CAPTURE_MODE_I,
	
	/**166:ˮƽ�����ܿ���;	<Range>{0,1}						<Mode>G/S	*/
	ISP_MIRROR_EN_I,

	/**167:�������ú����	<Range>{}						<Mode>G/S	*/
	ISP_SET_INFRATED_I,

	/**168:smart ir AEȨ�صȼ�;	<Range>[0,256],0-255,256:ʹ��Ĭ�ϲ���<Mode>G/S	*/
	ISP_SET_IR_AE_WEIGHT_LEVEL,
	
	/**169:�������ĵ�λ��			<Range>	����Ϊ�ṹ��		<Mode>G/S*/
	ISP_FISHEYE_OFFSET_I,

	/*=========================ISP DEBUG INTERFACE==========================*/
	/**192:debug��ӡ��Ϣ�ȼ�;	<Range>[0x0,0xffffffff]				<Mode>S	*/
	ISP_DEBUG_PRINT_LEVEL_I = 0xC0,
	
	/**193:ץraw���ݽӿ�;		<Range>   						<Mode>G	*/
	ISP_CAP_RAW_FRAME_I,
	
	/**194:ISP���Թ��߿���;	<Range>{0,1}     					<Mode>G/S	*/
	ISP_TEST_TUNING_EN_I,
	
	/**195:�ڲ��߳̿���;		<Range>{0,1}     					<Mode>G/S	*/
	ISP_INNER_THREAD_EN_I,
	
	/**196:����sensor�Ĵ���;	<Range>����Ϊ�ṹ��   	<Mode>S	*/
	ISP_SET_SENSOR_REG_I,
	
	/**197:��ȡsensor�Ĵ�����ֵ;	<Range>����Ϊ�ṹ��   <Mode>S	*/
	ISP_GET_SENSOR_REG_I,
	
	/**198:��ӡispͼ�������Ϣ;	<Range>   					<Mode>G	*/
	ISP_VIDEO_RESOLUTION_I,
	
	/**199:��̬���ذ�ƽ�������ļ�;	<Range>{0,1}     		<Mode>G/S	*/
	ISP_AWB_DNYNIC_LOAD_CFG_I,
	
	/**200:��̬����CCM�����ļ�;<Range>    					<Mode>S	*/
	ISP_CCM_DNYNIC_LOAD_CFG_I,
	
	/**201:AE���Խӿڣ�value�ĸ�16λ��AE�ļ�ֵ��value�ĵ�16λ��KEY��Ӧֵ;	<Range>[0x0,0xffffffff]	     		<Mode>G/S	*/
	ISP_DEBUG_AE_ENTRY_I,
	
	/**202:AWB���Խӿڣ�value�ĸ�16λ��AWB�ļ�ֵ��value�ĵ�16λ��KEY��Ӧֵ;<Range>[0x0,0xffffffff]	    	<Mode>G/S	*/
	ISP_DEBUG_AWB_ENTRY_I,
	
	/**203:CCM���Խӿڣ�value�ĸ�16λ��CCM�ļ�ֵ��value�ĵ�16λ��KEY��Ӧֵ;<Range>[0x0,0xffffffff]	    	<Mode>G/S	*/
	ISP_DEBUG_CCM_ENTRY_I,
	
	/**204:GBCE���Խӿڣ�value�ĸ�16λ��GBCE�ļ�ֵ��value�ĵ�16λ��KEY��Ӧֵ;	<Range>[0x0,0xffffffff]	    	<Mode>G/S	*/
	ISP_DEBUG_GBCE_ENTRY_I,
	
	/**205:demosic���Խӿ�;		<Range>    						<Mode>S	*/
	ISP_DEBUG_DEMOSAIC_I,
	
	/**206:sharpen���Խӿ�;		<Range>    						<Mode>S	*/
	ISP_DEBUG_SHARPEN_I,
	
	/**207:ɫ�����Ƶ��Խӿ�;<Range>    						<Mode>S	*/
	ISP_DEBUG_COLOR_SUPPRESS_I,
	
	/**208:������Խӿ�;		<Range>    						<Mode>S	*/
	ISP_DEBUG_DENOISE_I,
	
	/**209:gamma���Խӿ�;		<Range>    						<Mode>S	*/
	ISP_DEBUG_GAMMA_I,
	
	/**210:�ڵ�ƽ���Խӿ�;	<Range>    						<Mode>G/S	*/
	ISP_DEBUG_BLC_ENTRY_I,
	
	/**211:PWM���Խӿ�;			<Range>   						<Mode>G/S	*/
	ISP_DEBUG_PWM_I,
	
	/**212:P-IRIS���Խӿ�;		<Range>     						<Mode>G/S	*/
	ISP_DEBUG_P_IRIS_PARAM_I,

	/**213:��̬����isp bin�����ļ�;	<Range>{0,1}     		<Mode>G/S	*/
	ISP_BIN_DNYNIC_LOAD_CFG_I,

	/**214:AWB���Խӿ�; <Range>[0x0,0xffffffff]		<Mode>G/S */
	ISP_DEBUG_GET_AWB_I,
	
	/**215:AE���Խӿ�; <Range>[0x0,0xffffffff]		<Mode>G/S */
	ISP_DEBUG_GET_AE_I,
	
	/**216:LTM���Խӿ�; <Range>[0x0,0xffffffff]		<Mode>G/S */
	ISP_DEBUG_SET_LTM_I,

	/**217:VPSS SCALER���Խӿ�; <Range>[0x0,0xffffffff]		<Mode>G/S */
	ISP_DEBUG_VPSS_SCALER_I,

	/*=========================OTHER INTERFACE==========================*/
	/**256:��оAE��Ϣ�ش�;		<Range>     						<Mode>G/S	*/
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
	int         abf_type;  					// 0,��֧�֣��ͺ�1�� 
	int 		af_type;					// 0,��֧�֣��ͺ�1��
} AF_PARAM;


// ���������ز���
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
	AF_PARAM			af_params;		//��ͷ����
	unsigned int		lens_type;		//���۾�ͷ����
    char            	reserved[60];
}ISP_CREATE_PARAM;



// ISP �汾��Ϣ
typedef struct ISP_LIB_VERSION_S 
{
	unsigned char	major;				/* ���汾�ţ��ӿڸĶ����������ӡ��ܹ����ʱ���� */
	unsigned char	sub_version;		/* �Ӱ汾�ţ������Ż����ֲ��ṹ������ģ���ڼ�������������汾����ʱ���� */
	unsigned char	revision_version;	/* �����汾�ţ�����bug����� */
	char			mk_date_time[64];			/*����150424-18:58:00*/
	char			description[64];
}ISP_LIB_VERSION;



// isp������Ҫ�Ĳ����ṹ��
typedef struct ISP_PROC_PARAM_STRU_
{
    int             width;            // ͼ����
	int             height;           // ͼ��߶�
	char            reserved[64];

} ISP_PROC_PARAM;



typedef struct 
{
	unsigned int x; //ͼ������ˮƽ����
    unsigned int y; //ͼ�����Ĵ�ֱ����
    unsigned int w; //ͼ�񴰿ڿ�����ܳ���VIDEO_PARAM_CFG�ṹ����image_viW, ��С64
    unsigned int h; //ͼ�񴰿ڸ�����ܳ���VIDEO_PARAM_CFG�ṹ����image_viH, ��С32
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
	unsigned int reg_addr;//sensor�Ĵ�����ַ
	unsigned int reg_data;//sensor�Ĵ���ֵ
	unsigned int reg_num;//sensor�Ĵ�������
}SENSOR_REG_ATTR;

typedef struct ISP_IRIS_ARRT_S
{
	unsigned int  aeMode; // 0:�Զ���Ȧ�� 1:�ֶ���Ȧ(DC-IRIS)��2: P-IRIS����1...
	unsigned int  sensitivity; //�Զ���Ȧ������(DC-IRISʱ��Ч);
	unsigned int  pirisMode; // P-IRISģʽ����Ч��0:�Զ���Ȧ��1:�̶���Ȧ;
	unsigned int  pirisLevel; // P-IRIS�̶���Ȧ����Ч����Ȧ����1~100
	unsigned int  res[4];
} ISP_IRIS_ATTR;


typedef struct VI_ATTR_S
{
	int		bitLen; //bayer����λ������ץȡbayer����
	int		width; //�͸�viģ���bayer���ݿ��
	int		height; //�͸�viģ���bayer���ݸ߶�
	float	f32FrameRate;   /* RW. */
	int 	max_width;
	int 	max_height;
}VI_ATTR;

typedef struct
{
	unsigned char mode;						//����ƿ���ģʽ1: �ֶ� 0:�Զ�
	unsigned char IRGrpNo;					//���������
	unsigned char level;  					//�������Ƶȼ�
	unsigned char grpLevel[MAX_IR_LENS_GROUP];//�������Ƶȼ�
	unsigned char anti_overexp;				//��������ؿ���
	unsigned char IR_status; 					//�����״̬  0:�ر�  1:����
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

//�����豸���ĵ�λ�ƽṹ�壬��dsp�ṹ��һ��
typedef struct ISP_FISHEYE_OFFSET_S
{
	int left;	//��Ϊ��������Ϊ��
    int top;	//��Ϊ����,��Ϊ��
    int radius;	//���۰뾶
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
 * �ӿں���
 *********************************************************************************/

/*******************************************************************************
 * ��  �ܣ���ȡ��ǰ����汾��Ϣ
 * ��  ������
 * ����ֵ�����ذ汾��Ϣ
 * ��  ע���汾��Ϣ��ʽΪ���汾�ţ��꣨7λ�����£�4λ�����գ�5λ��
 *         ���а汾��Ϊ�����汾�ţ�6λ�����Ӱ汾�ţ�5λ���������汾�ţ�5λ��
 *******************************************************************************/
HRESULT ISP_GetVersion(ISP_LIB_VERSION *isp_lib_version);


/******************************************************************************
 * ��  ��: ��ȡ���ڲ�������ڴ��С
 * ��  ��: param - ���������ز���
 *
 * ����ֵ: ״̬��
 * ��  ע:
 *******************************************************************************/
HRESULT ISP_GetMemSize(ISP_CREATE_PARAM *param);



/******************************************************************************
 * ��  ��: �������ʵ��
 * ��  ��:
 *         param       - ���������ز���
 *         handle      - ���
 *
 * ����ֵ: ״̬��
 * ��  ע:
 *******************************************************************************/
HRESULT ISP_Create(ISP_CREATE_PARAM *param, VOID **handle);
//HRESULT ISP_Create(ISP_INNER_PARAM *param);

/******************************************************************************
 * �� �ܣ��㷨��������ýӿ�, ����ISP����ز�����
 *                    ע�������ISP_Create()֮����á�
 * �� ����
 *         handle     -IO ģ����
 *         param_key  -I  ������
 *         param        -I  ����ָ��
 *
 * ����ֵ������״̬��
 ********************************************************************************/
HRESULT ISP_SetKeyParam(void *handle, ISP_PARAM_KEY param_key, void *param);



/*******************************************************************************
 * �� �ܣ��㷨�������ȡ�ӿ�, ���ISP����ز�����
 *                    ע�������ISP_Create()֮����á�
 * �� ����
 *         handle      -IO ģ����
 *         param_key   -I ������
 *         param         -O ����ָ��
 *
 * ����ֵ������״̬��
 *********************************************************************************/
HRESULT ISP_GetKeyParam(void *handle, ISP_PARAM_KEY param_key, void *param);


/******************************************************************************
 * ��  ��: ��ȡ��ISP��صļ����ؼ�����
 * ��  ��:
 *         sensor_id	- Senser ID����
 *         vi_attr		- vi���Խṹ��
 *
 * ����ֵ: ״̬��
 * ��  ע:
 *******************************************************************************/
HRESULT ISP_GetViAttr(SENSOR_CFG_S sensor_cfg, VI_ATTR *vi_attr);

/******************************************************************************
 * ��  ��: ��ȡ�ں���ISP�ж�״̬
 * ��  ��:
 *         sensor_id	- Senser ID����
 *         vi_attr		- vi���Խṹ��
 *
 * ����ֵ: ״̬��
 * ��  ע:
 *******************************************************************************/
HRESULT ISP_GetViHwStatus(void	*pstStat);


/******************************************************************************
 * ��  ��: ISPԤ��ʼ���ӿڣ�������sensorʱ�ӡ���λ���������
 * ��  ��:
 *		   sensor_id - isp����ͷ�ļ��ж����sensor����
 *
 * ����ֵ: ״̬��
 * ��  ע:���һ����Sensorʱ��Ҫ��Ӷ�Ӧ��֧�֡�
 *
 *******************************************************************************/
HRESULT ISP_PreInit(ISP_VI_MODE_E sensor_id);


/*******************************************************************************
 * �� �ܣ�DSP��ISPע�ắ��
 * �� ����
 *		   param		- ����ָ��
 *
 * ����ֵ������״̬��
 *********************************************************************************/

HRESULT ISP_Register_CallBackFunc(ISP_FUNC_CALL *param);

#ifdef __cplusplus
}
#endif


#endif /* __ISP_INTERFACE_H__ */

