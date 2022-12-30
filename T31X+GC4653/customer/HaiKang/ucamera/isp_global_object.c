/** @file isp_global_object.c
 *  @note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
 *  @brief isp 中全局变量定义。
 *  
 *  @author   yaojinbo
 *  @date     2015-05-26
 *  @version  1.0
 *  @note ///Description here 
 *  @note History:        
 *  @note     <author>   <time>    <version >   <desc>
 *  @note  	 yaojinbo    2015-05-28  修改
 *  @warning  
 */


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


#include <sys/wait.h>
#include "imp-common.h"

#define TAG "Sample-UCamera"

#include "ucam/usbcamera.h"
#include "anticopy/AntiCopy_Verify.h"

#include "hikisp/isp_inner.h"

/******************************* 结构体全局实例 ******************************/
struct video_isp_class g_isp_object;
ISP_CFG_TABLES *gp_isp_cfg_tables;
ISP_INNER_CTRL *gp_isp_inner_ctrl;
AF_PARAM af_statis;
ISP_PARAMS_CHANGED g_isp_params_change;
//struct isp_sensor_if jxf22_mipi;
u32 g_vmax;
/******************************* 全局变量 ******************************/

U32 g_dbg_level = 0x1;

HIK_BOOL g_sensor_reset_flag = HIK_FALSE;
ISP_VI_MODE_E g_sensor_type = SAMPLE_VI_MODE_1_D1;
S32 isp_hikio_fd = -1;		/* HIKIO设备文件的描述符 */

G_INFO_SHARE_MEM *g_share_mem = NULL;

S32 g_dbg_daynight = -1;

BOOL g_smart_ir_status = HIK_FALSE;
U32 hist_weight_max = HIST_WEIGHT_MAX;
BOOL set_hist_weight_flag = HIK_TRUE;
U08 g_ae_blc_mode;

ISP_FUNC_CALL g_isp_func_callback;

//ccm 0
S16 ccm_min[9] = {79,156,21,79,156,21,79,156,21};
//ccm100
S16 ccm_max[9] = {433,-156,-21,-79,356,-21,-79,-156,491};
//ccm
S16 ccm_cur[9] = {256,0,0,0,256,0,0,0,256};

#if 0
/*镜像模式像素偏移与enBayerType映射表*/
const unsigned char g_mirror_offset2fmt[4][4] = {
 { BAYER_RGGB, BAYER_GRBG, BAYER_GBRG, BAYER_BGGR},  //RGGB
 { BAYER_GRBG, BAYER_RGGB, BAYER_BGGR, BAYER_GBRG},  //GRBG
 { BAYER_BGGR, BAYER_GBRG, BAYER_GRBG, BAYER_RGGB},  //BGGR
 { BAYER_GBRG, BAYER_BGGR, BAYER_RGGB, BAYER_GRBG},  //GBRG
};
#endif
U16 pre_gamma_lut[160] = {0};
U16 MWB_LEVAL_R = 1000;
U16 MWB_LEVAL_B = 1000;
U08 g_apc_selection = 1;
ISP_FISHEYE_PARAM g_fisheye_param = {0};

 /* @fn 	 system_ret_check
   * @brief system系统函数调用返回值检测
   * @param   [in] status,system函数返回值
   * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
  */
 s32 system_ret_check(pid_t status)
{
	if (-1 == status)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, "system error!"); 
		return ISP_LIB_S_FAIL;
	} 
	else 
	{
		if (WIFEXITED(status))
		{
			if (0 == WEXITSTATUS(status)) 
			{
				return ISP_LIB_S_OK;
			} 
			else 
			{
				PRINT_INFO(DBG_KEY,g_dbg_level, "run shell script fail, script exit code: %d\n", WEXITSTATUS(status));
				return ISP_LIB_S_FAIL;
			}
		} 
		else 
		{
			PRINT_INFO(DBG_KEY,g_dbg_level, "exit status = [%d]\n", WEXITSTATUS(status));  
			return ISP_LIB_S_FAIL;
		}
	}
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


