/** @file isp_coms_ctl.h
*  @note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*  @brief  该文件主要声明sensor coms和ctl文件中的函数,
*  
*  @author   yaojinbo
*  @date	   2015-05-26
*  @version  1.0
*  @note ///Description here 
*  @note History: 	   
*  @note	   <author>   <time>	<version >	 <desc>
*  @note	  yaojinbo	  2015-05-28  修改
*  @warning  
*/


#ifndef __ISP_COMS_CTL_H__
#define __ISP_COMS_CTL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_platform.h"
#include "isp_global_object.h" // 全局变量


/*************************** coms 文件中图像效果类参数声明 *************************/
//MN34425
extern ISP_SHARPENESS_PARAM g_mn34425_sharpeness_param_linear[PARAM_LEVEL_NUMS];
extern ISP_SHARPENESS_PARAM g_mn34425_sharpeness_param_wdr[PARAM_LEVEL_NUMS];
extern ISP_CONTRAST_PARAM g_mn34425_contrast_param_linear[PARAM_LEVEL_NUMS];
extern ISP_NR3D_PARAM g_mn34425_nr3d_param_linear[PARAM_LEVEL_NUMS];
extern ISP_NR3D_PARAM g_mn34425_nr3d_param_wdr[PARAM_LEVEL_NUMS];
extern ISP_NR2D_PARAM g_mn34425_nr2d_param_linear[PARAM_LEVEL_NUMS];
extern ISP_NR2D_PARAM g_mn34425_nr2d_param_wdr[PARAM_LEVEL_NUMS];
extern ISP_YNR_PARAM g_mn34425_ynr_param_linear[PARAM_LEVEL_NUMS];
extern ISP_YNR_PARAM g_mn34425_ynr_param_wdr[PARAM_LEVEL_NUMS];
extern ISP_LTM_PARAM g_mn34425_ltm_table_linear[PARAM_LEVEL_NUMS];
#if 0
extern ISP_GB_CFG g_mn34425_gb_param_linear[12];
extern ISP_LC_CFG g_mn34425_lc_param_linear;
extern ISP_LC_CFG g_mn34425_lc_param_wdr;
#endif
extern ISP_VPSS_PARAM g_mn34425_vpss_param_linear;
extern ISP_VPSS_PARAM g_mn34425_vpss_param_wdr;
extern AEC_Y_REF_LIST 			luma_ref_mn34425_linear;
extern AEC_Y_REF_LIST 			luma_ref_mn34425_wdr;
extern AEC_Y_REF_LIST 			luma_ref_mn34425_linear_hlc;
extern AEC_Y_REF_LIST 			luma_ref_mn34425_hlc;

//OV2735
extern ISP_SHARPENESS_PARAM g_ov2735_sharpeness_param_linear[PARAM_LEVEL_NUMS];
extern ISP_CONTRAST_PARAM g_ov2735_contrast_param_linear[PARAM_LEVEL_NUMS];
extern ISP_NR3D_PARAM g_ov2735_nr3d_param_linear[PARAM_LEVEL_NUMS];
extern ISP_NR2D_PARAM g_ov2735_nr2d_param_linear[PARAM_LEVEL_NUMS];
extern ISP_YNR_PARAM g_ov2735_ynr_param_linear[PARAM_LEVEL_NUMS];
extern ISP_LTM_PARAM g_ov2735_ltm_table_linear[PARAM_LEVEL_NUMS];
//extern ISP_GB_CFG g_ov2735_gb_param_linear[12];
//extern ISP_LC_CFG g_ov2735_lc_param_linear;
extern ISP_VPSS_PARAM g_ov2735_vpss_param_linear;
extern AEC_Y_REF_LIST 			luma_ref_ov2735_linear;
extern AEC_Y_REF_LIST 			luma_ref_ov2735_linear_hlc;

//K02
extern ISP_SHARPENESS_PARAM g_k02_sharpeness_param_linear[PARAM_LEVEL_NUMS];
extern ISP_CONTRAST_PARAM g_k02_contrast_param_linear[PARAM_LEVEL_NUMS];
extern ISP_NR3D_PARAM g_k02_nr3d_param_linear[PARAM_LEVEL_NUMS];
extern ISP_NR2D_PARAM g_k02_nr2d_param_linear[PARAM_LEVEL_NUMS];
extern ISP_YNR_PARAM g_k02_ynr_param_linear[PARAM_LEVEL_NUMS];
extern ISP_LTM_PARAM g_k02_ltm_table_linear[PARAM_LEVEL_NUMS];
//extern ISP_GB_CFG g_k02_gb_param_linear[12];
//extern ISP_LC_CFG g_k02_lc_param_linear;
extern ISP_VPSS_PARAM g_k02_vpss_param_linear;
extern AEC_Y_REF_LIST 			luma_ref_k02_linear;
extern AEC_Y_REF_LIST 			luma_ref_k02_linear_hlc;

//SP2309
extern ISP_SHARPENESS_PARAM g_sp2309_sharpeness_param_linear[PARAM_LEVEL_NUMS];
extern ISP_CONTRAST_PARAM g_sp2309_contrast_param_linear[PARAM_LEVEL_NUMS];
extern ISP_NR3D_PARAM g_sp2309_nr3d_param_linear[PARAM_LEVEL_NUMS];
extern ISP_NR2D_PARAM g_sp2309_nr2d_param_linear[PARAM_LEVEL_NUMS];
extern ISP_YNR_PARAM g_sp2309_ynr_param_linear[PARAM_LEVEL_NUMS];
extern ISP_LTM_PARAM g_sp2309_ltm_table_linear[PARAM_LEVEL_NUMS];
//extern ISP_GB_CFG g_sp2309_gb_param_linear[12];
//extern ISP_LC_CFG g_sp2309_lc_param_linear;
extern ISP_VPSS_PARAM g_sp2309_vpss_param_linear;
extern AEC_Y_REF_LIST 			luma_ref_sp2309_linear;
extern AEC_Y_REF_LIST 			luma_ref_sp2309_linear_hlc;
#if 0
/************************* cmos 文件中vi函数声明 ******************************************/
int get_vi_attr_mn34425(SENSOR_CFG_S sensor_cfg, ISP_VI_ATTR_S *vi_attr);
int get_vi_attr_ov2735(SENSOR_CFG_S sensor_cfg, ISP_VI_ATTR_S *vi_attr);
int get_vi_attr_k02(SENSOR_CFG_S sensor_cfg, ISP_VI_ATTR_S *vi_attr);
int get_vi_attr_sp2309(SENSOR_CFG_S sensor_cfg, ISP_VI_ATTR_S *vi_attr);

/************************* cmos 文件中默认参数函数声明 ******************************************/
U32 get_mn34425_default_param(ISP_DEFAULT_PARAM *gst_isp_def_param,HIK_BOOL b_Wdr_Mode);
U32 get_ov2735_default_param(ISP_DEFAULT_PARAM *gst_isp_def_param,HIK_BOOL b_Wdr_Mode);
U32 get_k02_default_param(ISP_DEFAULT_PARAM *gst_isp_def_param,HIK_BOOL b_Wdr_Mode);
U32 get_sp2309_default_param(ISP_DEFAULT_PARAM *gst_isp_def_param,HIK_BOOL b_Wdr_Mode);
#endif
#ifdef __cplusplus
}
#endif


#endif /*__ISP_COMS_CTL_H__ */

