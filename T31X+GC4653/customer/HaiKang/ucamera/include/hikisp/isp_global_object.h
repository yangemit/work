/** @file isp_global_object.h
*  @note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*  @brief isp妯￠敓鏂ゆ嫹閿熸枻鎷峰叏閿熻鎲嬫嫹閿熸枻鎷烽敓鏂ゆ嫹鍥熼敓鏂ゆ嫹閿熼叺鍑ゆ嫹鍕熼敓锟�
*  
*  @author   yaojinbo
*  @date     2015-05-26
*  @version  1.0
*  @note 閿熸枻鎷峰ご閿熶茎纭锋嫹鍙敓鏂ゆ嫹鍏ㄩ敓琛楁唻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鐤ュ畾閿熸枻鎷�
*  @note History:        
*  @note     <author>   <time>    <version >   <desc>
*  @note  	 yaojinbo    2015-05-28  閿熺潾闈╂嫹
*  @warning  
*/

#ifndef __ISP_GLOBAL_OBJECT_H__
#define __ISP_GLOBAL_OBJECT_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "isp_img.h"
#include "isp_3a.h"
#include "isp_struct_type.h"



#define AWB_CONFIG_PATH				"/system/etc/isp/awb/"
#define CCM_CONFIG_PATH				"/system/etc/isp/ccm/"

//GBCE閿熸枻鎷烽敓鏂ゆ嫹閿熶茎纭锋嫹
#define GBCE_AC_CURVE_TAB_AMBA		"/home/e6_isp_config/gbce/GBCE_AC_CURVE_TAB_1.dat"
#define GBCE_AC_CURVE_TAB_HISI		"/home/e6_isp_config/gbce/GBCE_AC_CURVE_TAB_2.dat"
#define GBCE_AC_CURVE_TAB_TI		"/home/e6_isp_config/gbce/GBCE_AC_CURVE_TAB_3.dat"

/* AWB閿熸枻鎷烽敓鏂ゆ嫹閿熶茎纭锋嫹 */
#if 0
#define AWB_CONFIG_MN34425_LINERA		"awb_1_s01_0_cfg.bin"
#define AWB_CONFIG_MN34425_WDR			"awb_1_s01_0_cfg.bin"
#define AWB_CONFIG_OV2735_LINERA		"awb_1_s02_0_cfg.bin"
#define AWB_CONFIG_OV2735_WDR			"awb_1_s02_0_cfg.bin"
#define AWB_CONFIG_OV2735_PB401F_LINERA	"awb_1_s02_1_cfg.bin"
#define AWB_CONFIG_OV2735_PB401F_WDR	"awb_1_s02_1_cfg.bin"
#define AWB_CONFIG_K02_LINERA			"awb_1_s03_0_cfg.bin"
#define AWB_CONFIG_K02_WDR				"awb_1_s03_0_cfg.bin"
#define AWB_CONFIG_SP2309_LINERA		"awb_1_s04_0_cfg.bin"
#define AWB_CONFIG_SP2309_WDR			"awb_1_s04_0_cfg.bin"
#else
#define AWB_CONFIG_OS02D20_LINERA		"awb_1_s01_0_cfg.bin"
#define AWB_CONFIG_OS04C10_LINERA		"awb_1_s02_0_cfg.bin"
#define AWB_CONFIG_JXF37_LINERA			"awb_1_s03_0_cfg.bin"
#endif

/* CCM閿熸枻鎷烽敓鏂ゆ嫹閿熶茎纭锋嫹 */
#if 0
#define CCM_CONFIG_MN34425_LINEAR	"ccm_1_s01_0_cfg.bin"
#define CCM_CONFIG_MN34425_WDR		"ccm_1_s01_1_cfg.bin"
#define CCM_CONFIG_OV2735_LINEAR	"ccm_1_s02_0_cfg.bin"
#define CCM_CONFIG_OV2735_WDR		"ccm_1_s02_0_cfg.bin"
#define CCM_CONFIG_K02_LINEAR		"ccm_1_s03_0_cfg.bin"
#define CCM_CONFIG_K02_WDR			"ccm_1_s03_0_cfg.bin"
#define CCM_CONFIG_SP2309_LINEAR	"ccm_1_s04_0_cfg.bin"
#define CCM_CONFIG_SP2309_WDR		"ccm_1_s04_0_cfg.bin"
#else
#define CCM_CONFIG_OS02D20_OUTDOOR		"ccm_1_s01_0_cfg.bin"
#define CCM_CONFIG_OS02D20_INDOOR		"ccm_1_s01_1_cfg.bin"
#define CCM_CONFIG_OS04C10_OUTDOOR		"ccm_1_s02_0_cfg.bin"
#define CCM_CONFIG_OS04C10_INDOOR		"ccm_1_s02_1_cfg.bin"
#define CCM_CONFIG_JXF37_OUTDOOR		"ccm_1_s03_0_cfg.bin"
#define CCM_CONFIG_JXF37_INDOOR			"ccm_1_s03_1_cfg.bin"

#endif

#define IS_WDR_NONE(mode)	(WDR_MODE_NONE == (mode)||WDR_DIGITAL == (mode)) // 閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓琛楀尅鎷烽敓鏂ゆ嫹鎬�
#define IS_WDR_TRUE(mode)	((mode) >= WDR_MODE_BUILT_IN && (mode) < WDR_DIGITAL)

#define IR_MODE_AUTO    0
#define IR_MODE_MANUAL  1

/* PWM閿熸枻鎷峰洘閿熸枻鎷烽敓锟� */
typedef struct pwm {
	int chan;
	int period;  ///< unit: ns
	int duty;
}PWD_INFO_T;

#define DEVICE_HIKIO	"/dev/hikio"
#define MISC_IOC_MAGIC    'M'
#define HAL_SET_PWM     _IOW(MISC_IOC_MAGIC, 2, struct pwm*)


/******************************* 閿熺粨鏋勯敓鏂ゆ嫹鍏ㄩ敓鏂ゆ嫹瀹為敓鏂ゆ嫹 ******************************/
extern ISP_INNER_CTRL *gp_isp_inner_ctrl;
extern ISP_CFG_TABLES *gp_isp_cfg_tables;
extern AF_PARAM af_statis;
extern ISP_PARAMS_CHANGED g_isp_params_change;
#if 0
extern ISP_VI_HW_ATTR gst_mn34425_vi_hw_attr;
extern AWB_DEFAULT_CFG gst_mn34425_awb_cfg;
extern AE_DEFAULT_CFG gst_mn34425_ae_cfg;
extern ISP_VI_HW_ATTR gst_ov2735_vi_hw_attr;
extern AWB_DEFAULT_CFG gst_ov2735_awb_cfg;
extern AE_DEFAULT_CFG gst_ov2735_ae_cfg;
extern ISP_VI_HW_ATTR gst_k02_vi_hw_attr;
extern AWB_DEFAULT_CFG gst_k02_awb_cfg;
extern AE_DEFAULT_CFG gst_k02_ae_cfg;
extern ISP_VI_HW_ATTR gst_sp2309_vi_hw_attr;
extern AWB_DEFAULT_CFG gst_sp2309_awb_cfg;
extern AE_DEFAULT_CFG gst_sp2309_ae_cfg;
#endif
/******************************* 鍏ㄩ敓琛楁唻鎷烽敓鏂ゆ嫹 ******************************/
extern U32 g_dbg_level;
extern HIK_BOOL g_sensor_reset_flag;
extern ISP_VI_MODE_E g_sensor_type;
extern S32 isp_hikio_fd;

extern G_INFO_SHARE_MEM *g_share_mem;

extern ISP_FUNC_CALL g_isp_func_callback;

extern BOOL g_smart_ir_status;
extern BOOL set_hist_weight_flag;
extern U32 hist_weight_max;

extern S16 ccm_min[9];
extern S16 ccm_max[9];
extern S16 ccm_cur[9];

extern BOOL g_smart_ir_status;

extern U08 g_ae_blc_mode;
extern ISP_GAMMA_PARAM g_gamma_table_hisi[11];
extern ISP_GAMMA_PARAM g_gamma_table_amba[11];
extern ISP_GAMMA_PARAM g_gamma_table_ti[11];
extern ISP_GAMMA_PARAM g_gamma_table_fh[11];
extern ISP_GAMMA_PARAM g_gamma_table_wdr[11];

extern const unsigned char g_mirror_offset2fmt[4][4];
extern U16 pre_gamma_lut[160];
extern U16 MWB_LEVAL_R;
extern U16 MWB_LEVAL_B;
extern isp_ae_weight_table_t g_isp_ae_weight_table[8];
extern isp_ae_weight_table_t g_isp_ae_weight_table_fisheye[8];
extern U08 g_apc_selection;
extern ISP_FISHEYE_PARAM g_fisheye_param;
//extern isp_sensor_if jxf22_mipi;
extern u32 g_vmax;

#ifdef __cplusplus
}
#endif


#endif /* __ISP_GLOBAL_OBJECT_H__ */
