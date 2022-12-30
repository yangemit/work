/** @file isp_inner.h
*	@note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*	@brief sensor��غ궨��ͺ����ӿڶ���
*	
*	@author   yaojinbo
*	@date	  2015-05-26
*	@version  1.0
*	@note 
*	@note History:		  
*	@note	  <author>	 <time>    <version >	<desc>
*	@note	  yaojinbo	  2015-05-28  �޸�
*	@warning  
*/


#ifndef _SENSOR_H_
#define _SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_platform.h"

#define SENSOR_TRANS_I2C 1/**<I2C BUS*/
#define SENSOR_TRANS_SPI 2/**<SPI BUS*/

#define MN34425_I2C_ADDR	(0x6C>>1)
//#define OV2735_I2C_ADDR		(0x7a>>1)
#define OV2735_I2C_ADDR		(0x78>>1)
#define K02_I2C_ADDR		(0x8C>>1)
#define SP2309_I2C_ADDR		(0x7a>>1)

#define I2C_DEVICE		"/dev/i2c-0"
#define FH_CLK_DEVICE	"/dev/fh_clk_miscdev"

#define SENSOR_MAX_REGS 12

/**����sensor���䷽ʽ�ṹ��*/
typedef struct _BUS_ATTR_S_
{
	U32 device_addr;
	U32 addr_len;
	U32 data_len;
	U32 trans_type;
}BUS_ATTR;

typedef struct _SENSOR_REG_DATA
{
	U16 reg_addr;//�Ĵ�����ַ
	U16 reg_value;//�Ĵ���ֵ
	U08 delay_frame_num;//�ӳ�֡��,ֵΪ0����1��ʱ���ʾ��ǰ֡������ȥ.
	U08 update;//��������Ĵ����Ƿ���Ҫ����
	U16 extra_attr;//��չ����,����K02,����group�Ĵ�����ַ.
}SENSOR_REG_DATA;

/**����sensor��ؼĴ����ṹ��*/
typedef struct _SENSOR_REG_INFO_
{
	U32 reg_num;//��¼�ļĴ�������
	SENSOR_REG_DATA st_reg_data[SENSOR_MAX_REGS];
	U16 write_trigger;//�Ƿ񴥷�һ��groupд����.
	U16 extra_attr;//��չ����,����k02,��¼��һ֡������.
}SENSOR_REGS_INFO;

//Sensorʱ����ؽṹ��
typedef struct _ISP_CLK_PARAM_STRU_
{
	S32				sensor_id;
	U32				power_on_seq;
	unsigned long	sensor_inck;
	S32				delay;
}ISP_CLK_PARAM;

S32 sensor_read_reg(U16 reg,U16 *data);
S32 sensor_write_reg(U16 reg,U16 data);
S32 sensor_reg_cb(VOID);
//U32 get_default_param(ISP_DEFAULT_PARAM *gst_isp_def_param,HIK_BOOL b_Wdr_Mode);

/**mn34425*/
void cfg_capuremode_mn34425(void);
S32 sensor_init_mn34425(VOID);
S32 sensor_set_gain_mn34425(U32 gain);
S32 sensor_get_gain_mn34425(U32 *gain);
S32 sensor_set_intt_mn34425(U32 intt);
S32 sensor_get_intt_mn34425(U32 *intt);
S32 sensor_set_fmt_mn34425(int format);
S32 sensor_set_mipi_mn34425(void);
S32 sensor_set_exposure_ratio_mn34425(U32 exposure_ratio);
S32 sensor_get_exposure_ratio_mn34425(U32 *exposure_ratio);
S32 sensor_get_attribute_mn34425(char *name, U32 *value);

/**ov2735*/
void cfg_capuremode_ov2735(U32 fps);
S32 sensor_init_ov2735(VOID);
S32 sensor_set_gain_ov2735(U32 gain);
S32 sensor_get_gain_ov2735(U32 *gain);
S32 sensor_set_intt_ov2735(U32 intt);
S32 sensor_get_intt_ov2735(U32 *intt);
S32 sensor_set_fmt_ov2735(int format);
S32 sensor_set_mipi_ov2735(void);
S32 sensor_set_exposure_ratio_ov2735(U32 exposure_ratio);
S32 sensor_get_exposure_ratio_ov2735(U32 *exposure_ratio);
S32 sensor_get_attribute_ov2735(char *name, U32 *value);

/**k02*/
void cfg_capuremode_k02(void);
S32 sensor_init_k02(VOID);
S32 sensor_set_gain_k02(U32 gain);
S32 sensor_get_gain_k02(U32 *gain);
S32 sensor_set_intt_k02(U32 intt);
S32 sensor_get_intt_k02(U32 *intt);
S32 sensor_set_fmt_k02(int format);
S32 sensor_set_mipi_k02(void);
S32 sensor_set_exposure_ratio_k02(U32 exposure_ratio);
S32 sensor_get_exposure_ratio_k02(U32 *exposure_ratio);
S32 sensor_get_attribute_k02(char *name, U32 *value);
S32 k02_reg_to_buf(U16 addr,U16 data);
S32 k02_isp_write_reg(void);

/**sp2309*/
void cfg_capuremode_sp2309(void);
S32 sensor_init_sp2309(VOID);
S32 sensor_set_gain_sp2309(U32 gain);
S32 sensor_get_gain_sp2309(U32 *gain);
S32 sensor_set_intt_sp2309(U32 intt);
S32 sensor_get_intt_sp2309(U32 *intt);
S32 sensor_set_fmt_sp2309(int format);
S32 sensor_set_mipi_sp2309(void);
S32 sensor_set_exposure_ratio_sp2309(U32 exposure_ratio);
S32 sensor_get_exposure_ratio_sp2309(U32 *exposure_ratio);
S32 sensor_get_attribute_sp2309(char *name, U32 *value);
void sp2309_isp_manual_blc(void);

#ifdef __cplusplus
}
#endif

#endif /* _SENSOR_H_ */

