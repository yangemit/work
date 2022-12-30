/** @file isp_init.c
 *  @note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
 *  @brief isp 初始化函数接口。
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



#include "imp-common.h"

#define TAG "Sample-UCamera"

#include "ucam/usbcamera.h"
#include "anticopy/AntiCopy_Verify.h"

#include "hikisp/isp_inner.h"
#include <fcntl.h>

#define RT_SDK_PRIORITY 85

//static rt_thread_t g_thread_isp = 0;
#if 0
#define _NAME(n, s) n##_##s
#define NAME(n) _NAME(n, OVOS02_MIPI)
#endif

/* @fn      isp_alloc_buffer
 * @brief 在缓存buf中分配一块大小为size, 128个字节对齐的内存块
 * @param   [in] buf,缓存管理结构体
 * @param   [in] size,分配内存的大小
 * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
VOID *isp_alloc_buffer(ISP_BUF *buf, U32 size)
{
	VOID *tmp_buf;
	U32   free_size;

	/** 缓存中空余内存起始位置 */
	tmp_buf = buf->cur_pos;

	/**计算缓存中的空余空间大小 */
	free_size = (U32)buf->end - (U32)buf->cur_pos;

	/** 空间不够，返回空指针*/
	if (free_size < size)
	{
		tmp_buf = NULL;
	}
	else
	{
		/**更新空余指针位置 */
		buf->cur_pos = (VOID*)(((U32)tmp_buf + size + (ISP_ALIGN_BYTES-1)) & (~(ISP_ALIGN_BYTES-1)));
	}

	return tmp_buf;
}


/************************************************************************************************
 * 功 能：加载白平衡配置文件
 * 参 数：
 *
 *         inner_param     -     内部参数	
 * 
 * 返回值：返回状态码
 *************************************************************************************************/
HRESULT isp_load_awb_table(ISP_INNER_PARAM *inner_param)
{
	S32 ret;
	S32 file, count;
	S08 awb_line_config[128] = {0};
	S08 awb_wdr_config[128] = {0};

	switch(gp_isp_inner_ctrl->sensor_id)
	{
		case OMNI_OS02D20:
			snprintf(awb_line_config, sizeof(awb_line_config) - 1, "%s%s", AWB_CONFIG_PATH, AWB_CONFIG_OS02D20_LINERA);
			snprintf(awb_wdr_config, sizeof(awb_wdr_config) - 1, "%s%s", AWB_CONFIG_PATH, AWB_CONFIG_OS02D20_LINERA);
			break;
		case OMNI_OS04C10:
			snprintf(awb_line_config, sizeof(awb_line_config) - 1, "%s%s", AWB_CONFIG_PATH, AWB_CONFIG_OS04C10_LINERA);
			snprintf(awb_wdr_config, sizeof(awb_wdr_config) - 1, "%s%s", AWB_CONFIG_PATH, AWB_CONFIG_OS04C10_LINERA);
			break;
		case SOI_JXF37:
			snprintf(awb_line_config, sizeof(awb_line_config) - 1, "%s%s", AWB_CONFIG_PATH, AWB_CONFIG_JXF37_LINERA);
			snprintf(awb_wdr_config, sizeof(awb_wdr_config) - 1, "%s%s", AWB_CONFIG_PATH, AWB_CONFIG_JXF37_LINERA);
			break;
        default:
            break;
	}
		
	PRINT_INFO(DBG_FUNCTION,g_dbg_level, "load awb %s %s\n", awb_line_config, awb_wdr_config);
	//set awb_line params
	if ((file = open(awb_line_config, O_RDONLY, 0)) < 0)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, " awb_line.bin cannot be opened\n");
		CHECK_RET(1, ISP_LIB_S_FAIL);
	}
	if((ret = lseek(file, 0, SEEK_SET)) != 0)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, " awb_line lseek is filed:%d\n",ret);
		SAFE_CLOSE(file);
		return ISP_LIB_S_FAIL;
	}
	if ((count = read(file, gp_isp_cfg_tables->awb_config_data_linear, AWB_CONFIG_DATA_NUM)) != AWB_CONFIG_DATA_NUM)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, " read awb_line.bin linear error %d \n",count);
		SAFE_CLOSE(file);
		return ISP_LIB_S_FAIL;
	}
	SAFE_CLOSE(file);


	//Set awb_wdr params
	if ((file = open(awb_wdr_config, O_RDONLY, 0)) < 0)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, " awb_wdr.bin cannot be opened\n");
		CHECK_RET(1, ISP_LIB_S_FAIL);
	}
	if ((ret = lseek(file, 0, SEEK_SET)) != 0)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, " awb_wdr lseek is filed:%d\n",ret);
		SAFE_CLOSE(file);
		return ISP_LIB_S_FAIL;
	}
	if ((count = read(file, gp_isp_cfg_tables->awb_config_data_wdr, AWB_CONFIG_DATA_NUM)) != AWB_CONFIG_DATA_NUM)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, " read awb_wdr.bin wdr error %d \n",count);
		SAFE_CLOSE(file);
		return ISP_LIB_S_FAIL;
	}
	SAFE_CLOSE(file);
	
	return ret;
}


/************************************************************************************************
 * 功 能： 加载CCM配置，色彩矩阵联动使用
 * 参 数：
 *
 *         inner_param     -     内部参数	
 * 
 * 返回值：返回状态码
 *************************************************************************************************/
HRESULT isp_load_ccm_table(ISP_INNER_PARAM *inner_param)
{
	S32 ret;
	S32 file, count;
	S08 ccm_line_config[128] = {0};
	S08 ccm_wdr_config[128] = {0};

	switch(gp_isp_inner_ctrl->sensor_id)
	{
		case OMNI_OS02D20:
			snprintf(ccm_line_config, sizeof(ccm_line_config) - 1, "%s%s", CCM_CONFIG_PATH, CCM_CONFIG_OS02D20_OUTDOOR);
			snprintf(ccm_wdr_config, sizeof(ccm_wdr_config) - 1, "%s%s", CCM_CONFIG_PATH, CCM_CONFIG_OS02D20_OUTDOOR);
			break;
		case OMNI_OS04C10:
			snprintf(ccm_line_config, sizeof(ccm_line_config) - 1, "%s%s", CCM_CONFIG_PATH, CCM_CONFIG_OS04C10_OUTDOOR);
			snprintf(ccm_wdr_config, sizeof(ccm_wdr_config) - 1, "%s%s", CCM_CONFIG_PATH, CCM_CONFIG_OS04C10_OUTDOOR);
			break;
		case SOI_JXF37:
			snprintf(ccm_line_config, sizeof(ccm_line_config) - 1, "%s%s", CCM_CONFIG_PATH, CCM_CONFIG_JXF37_OUTDOOR);
			snprintf(ccm_wdr_config, sizeof(ccm_wdr_config) - 1, "%s%s", CCM_CONFIG_PATH, CCM_CONFIG_JXF37_OUTDOOR);
			break;
		default:
				break;
	}
	

	//snprintf(ccm_line_config, sizeof(ccm_line_config) - 1, "%s%s", CCM_CONFIG_PATH, CCM_CONFIG_OS04C10_OUTDOOR);
	//snprintf(ccm_wdr_config, sizeof(ccm_wdr_config) - 1, "%s%s", CCM_CONFIG_PATH, CCM_CONFIG_OS04C10_OUTDOOR);

	PRINT_INFO(DBG_FUNCTION,g_dbg_level, "load ccm %s %s\n", ccm_line_config, ccm_wdr_config);
	//set ccm_line params
	if ((file = open(ccm_line_config, O_RDONLY, 0)) < 0)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, " ccm_line.bin cannot be opened\n");
		CHECK_RET(1, ISP_LIB_S_FAIL);
	}
	if((ret = lseek(file, 0, SEEK_SET)) != 0)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, " ccm_line lseek is filed:%d\n",ret);
		SAFE_CLOSE(file);
		return ISP_LIB_S_FAIL;
	}
	if ((count = read(file, gp_isp_cfg_tables->ccm_config_data_linear, CCM_CONFIG_DATA_NUM)) != CCM_CONFIG_DATA_NUM)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, " read ccm_line.bin linear error %d \n",count);
		SAFE_CLOSE(file);
		return ISP_LIB_S_FAIL;
	}
	SAFE_CLOSE(file);

	//Set ccm_wdr params
	if ((file = open(ccm_wdr_config, O_RDONLY, 0)) < 0)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, " ccm_wdr.bin cannot be opened\n");
		CHECK_RET(1, ISP_LIB_S_FAIL);
	}
	if ((ret = lseek(file, 0, SEEK_SET)) != 0)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, " ccm_wdr lseek is filed:%d\n",ret);
		SAFE_CLOSE(file);
		return ISP_LIB_S_FAIL;
	}
	if ((count = read(file, gp_isp_cfg_tables->ccm_config_data_wdr, CCM_CONFIG_DATA_NUM)) != CCM_CONFIG_DATA_NUM)
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, " read ccm_wdr.bin wdr error %d \n",count);
		SAFE_CLOSE(file);
		return ISP_LIB_S_FAIL;
	}
	SAFE_CLOSE(file);
	
	return ISP_LIB_S_OK;
}


/************************************************************************************************
 * 功 能： 删除AWB CCM配置文件
 * 参 数：
 *
 *         inner_param     -     内部参数	
 * 
 * 返回值：返回状态码
 *************************************************************************************************/
HRESULT isp_remove_cfg_file(ISP_INNER_PARAM *inner_param)
{
	S32 ret = ISP_LIB_S_OK;
	S08 shell_cmd[128] = {0};
	S08 cfg_name_linear[CFG_NAME_LEN] = {0};
	S08 cfg_name_wdr[CFG_NAME_LEN] = {0};
	
	/** delet awb  param files*/
	/** 1.将配置文件移到上层目录保存*/
	strncpy(cfg_name_linear,gp_isp_cfg_tables->awb_line_config,sizeof(cfg_name_linear));//线性配置文件	
	snprintf(shell_cmd,sizeof(shell_cmd),"cp %s%s %s../", AWB_CONFIG_PATH,cfg_name_linear,AWB_CONFIG_PATH);
	if (system(shell_cmd) < 0)
	{
		PRINT_INFO(DBG_KEY, g_dbg_level, "system exec %s fail!\n", shell_cmd);
		ret = ISP_LIB_S_FAIL;
	}
	strncpy(cfg_name_wdr,gp_isp_cfg_tables->awb_wdr_config,sizeof(cfg_name_wdr));//宽动态配置文件	
	if(strcmp(cfg_name_linear,cfg_name_wdr) != 0)
	{
		memset(shell_cmd,0,sizeof(shell_cmd));
		snprintf(shell_cmd,sizeof(shell_cmd),"cp %s%s %s../",AWB_CONFIG_PATH,cfg_name_wdr,AWB_CONFIG_PATH);
		if (system(shell_cmd) < 0)
		{
			PRINT_INFO(DBG_KEY, g_dbg_level, "system exec %s fail!\n", shell_cmd);
			ret = ISP_LIB_S_FAIL;
		}
	}
	
	/** 2.删除所有的配置文件*/
	memset(shell_cmd,0,sizeof(shell_cmd));
	snprintf(shell_cmd,sizeof(shell_cmd),"rm %s*",AWB_CONFIG_PATH);
	if (system(shell_cmd) < 0)
	{
		PRINT_INFO(DBG_KEY, g_dbg_level, "system exec rm awb/* fail!\n");
		ret = ISP_LIB_S_FAIL;
	}
	
	/** 3.还原配置文件*/
	memset(shell_cmd,0,sizeof(shell_cmd));
	snprintf(shell_cmd,sizeof(shell_cmd),"cp %s../*.bin %s",AWB_CONFIG_PATH, AWB_CONFIG_PATH);
	if (system(shell_cmd) < 0)
	{
		PRINT_INFO(DBG_KEY, g_dbg_level, "system exec %s fail!\n", shell_cmd);
		ret = ISP_LIB_S_FAIL;
	}
	
	/** 4.删除备份的配置文件*/
	memset(shell_cmd,0,sizeof(shell_cmd));
	snprintf(shell_cmd,sizeof(shell_cmd),"rm %s../*.bin",AWB_CONFIG_PATH);
	if (system(shell_cmd) < 0)
	{
		PRINT_INFO(DBG_KEY, g_dbg_level, "system exec rm *.bin fail!\n");
		ret = ISP_LIB_S_FAIL;
	}
	PRINT_INFO(DBG_FUNCTION, g_dbg_level, "del awb file success!\n");

	/** delet ccm  param files*/
	/** 1.将配置文件移到上层目录保存*/
	memset(shell_cmd,0,sizeof(shell_cmd));
	memset(cfg_name_linear,0,sizeof(cfg_name_linear));
	memset(cfg_name_wdr,0,sizeof(cfg_name_wdr));
	strncpy(cfg_name_linear,gp_isp_cfg_tables->ccm_line_config,sizeof(cfg_name_linear));//线性配置文件	
	snprintf(shell_cmd,sizeof(shell_cmd),"cp %s%s %s../",CCM_CONFIG_PATH,cfg_name_linear,CCM_CONFIG_PATH);
	if (system(shell_cmd) < 0)
	{
		PRINT_INFO(DBG_KEY, g_dbg_level, "system exec %s fail!\n", shell_cmd);
		ret = ISP_LIB_S_FAIL;
	}
	strncpy(cfg_name_wdr,gp_isp_cfg_tables->ccm_wdr_config,sizeof(cfg_name_wdr));//宽动态配置文件	
	if(strcmp(cfg_name_linear,cfg_name_wdr) != 0)
	{
		memset(shell_cmd,0,sizeof(shell_cmd));
		snprintf(shell_cmd,sizeof(shell_cmd),"cp %s%s %s../",CCM_CONFIG_PATH,cfg_name_wdr,CCM_CONFIG_PATH);
		if (system(shell_cmd) < 0)
		{
			PRINT_INFO(DBG_KEY, g_dbg_level, "system exec %s fail!\n", shell_cmd);
			ret = ISP_LIB_S_FAIL;
		}
	}
	
	/** 2.删除所有的配置文件*/
	memset(shell_cmd,0,sizeof(shell_cmd));
	snprintf(shell_cmd,sizeof(shell_cmd),"rm %s*",CCM_CONFIG_PATH);
	if (system(shell_cmd) < 0)
	{
		PRINT_INFO(DBG_KEY, g_dbg_level, "system exec rm awb/* fail!\n");
		ret = ISP_LIB_S_FAIL;
	}

	/** 3.还原配置文件*/
	memset(shell_cmd,0,sizeof(shell_cmd));
	snprintf(shell_cmd,sizeof(shell_cmd),"cp %s../*.bin %s",CCM_CONFIG_PATH,CCM_CONFIG_PATH);
	if (system(shell_cmd) < 0)
	{
		PRINT_INFO(DBG_KEY, g_dbg_level, "system exec %s fail!\n", shell_cmd);
		ret = ISP_LIB_S_FAIL;
	}
	
	/** 4.删除备份的配置文件*/
	memset(shell_cmd,0,sizeof(shell_cmd));
	snprintf(shell_cmd,sizeof(shell_cmd),"rm %s../*.bin",CCM_CONFIG_PATH);
	if (system(shell_cmd) < 0)
	{
		PRINT_INFO(DBG_KEY, g_dbg_level, "system exec rm *.bin fail!\n");
		ret = ISP_LIB_S_FAIL;
	}
	PRINT_INFO(DBG_FUNCTION, g_dbg_level, "del ccm files success!\n");
	return ret;
}

#if 0
/* @fn      isp_gbce_running
  * @brief 图像调试工具开关
  * @param   [in] inner_param,isp内部参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_gbce_running(ISP_INNER_PARAM *inner_param)
{
#if 1
	S32 ret = 0, i;
	U32 gain_level = 0;
	ISP_GAMMA_CFG gamma_attr;
 	ISP_HIST_STAT hist_stat;
	U32 hist_out[256];
	U16 gamma_table_160[160];
	
	/*设置GBCE 曲线*/
	//宽动态开启的时候，不设置gamma表,如果关闭GBCE，需从新设置对比度
	if((1 == inner_param->img_enhance.gbce_enable)&&(IS_WDR_NONE(inner_param->img_enhance.auto_local_exp_mode)))
	{
		ret = API_ISP_GetHist(&hist_stat);
		CHECK_RET(ret < 0, ISP_LIB_S_FAIL);
		
		//直方图32bin插值为256bin
		for (i = 0; i < 31; i++)
		{
			hist_out[8*i]	  = 8 * hist_stat.u32histBin[i][0] >> 6;
			hist_out[8*i + 1] = (7 * hist_stat.u32histBin[i][0] + 1 * hist_stat.u32histBin[i + 1][0]) >> 6;
			hist_out[8*i + 2] = (6 * hist_stat.u32histBin[i][0] + 2 * hist_stat.u32histBin[i + 1][0]) >> 6;
			hist_out[8*i + 3] = (5 * hist_stat.u32histBin[i][0] + 3 * hist_stat.u32histBin[i + 1][0]) >> 6; 	
			hist_out[8*i + 4] = (4 * hist_stat.u32histBin[i][0] + 4 * hist_stat.u32histBin[i + 1][0]) >> 6;
			hist_out[8*i + 5] = (3 * hist_stat.u32histBin[i][0] + 5 * hist_stat.u32histBin[i + 1][0]) >> 6;
			hist_out[8*i + 6] = (2 * hist_stat.u32histBin[i][0] + 6 * hist_stat.u32histBin[i + 1][0]) >> 6; 	
			hist_out[8*i + 7] = (1 * hist_stat.u32histBin[i][0] + 7 * hist_stat.u32histBin[i + 1][0]) >> 6; 	
		}
		hist_out[248] = hist_stat.u32histBin[31][0] >> 3;
		hist_out[249] = hist_stat.u32histBin[31][0] >> 3;
		hist_out[250] = hist_stat.u32histBin[31][0] >> 3;
		hist_out[251] = hist_stat.u32histBin[31][0] >> 3;
		hist_out[252] = hist_stat.u32histBin[31][0] >> 3;
		hist_out[253] = hist_stat.u32histBin[31][0] >> 3;
		hist_out[254] = hist_stat.u32histBin[31][0] >> 3;
		hist_out[255] = hist_stat.u32histBin[31][0] >> 3;
		
		for(i = 0; i < 256; i++)
		{
			*(gp_isp_inner_ctrl->gbceCtrl.procParam.hist + i) = hist_out[i];
		}

		isp_get_current_gain(&gain_level);
		gp_isp_inner_ctrl->gbceCtrl.procParam.gain_lev = gain_level;
		ret = GBCE_Process(gp_isp_inner_ctrl->gbceCtrl.handle, &gp_isp_inner_ctrl->gbceCtrl.procParam);

		//160个点在0~4095上,0~256采样间隔是4,256~1024采样间隔是16,1024~4096采样间隔是64
		for(i = 0 ; i <= 64 ; i++)
		{
			gamma_table_160[i] = *(gp_isp_inner_ctrl->gbceCtrl.procParam.gamma_lut + i);
		}
		for(i = 65 ; i <= 112 ; i++)
		{
			gamma_table_160[i] = *(gp_isp_inner_ctrl->gbceCtrl.procParam.gamma_lut + 64 + 4 * (i-64));
		}
		for(i = 113 ; i < 160 ; i++)
		{
			gamma_table_160[i] = *(gp_isp_inner_ctrl->gbceCtrl.procParam.gamma_lut + 256 + 16 * (i-112));
		}
		
		//如果gamma表变化时才设置
		ret = memcmp(pre_gamma_lut, gamma_table_160, sizeof(U16) * 160);
		if(0 != ret)
		{
			ret = API_ISP_GetGammaCfg(&gamma_attr);
			CHECK_RET(ret < 0, ISP_LIB_S_FAIL);
			
			gamma_attr.bGammaEn = FH_TRUE;
			gamma_attr.eCGammaBuiltInIdx = GAMMA_CURVE_22;
			gamma_attr.eYGammaBuiltInIdx = GAMMA_CURVE_22;
			memcpy(gamma_attr.u16CGamma, gamma_table_160, sizeof(U16) * 160);
			ret |= API_ISP_SetGammaCfg(&gamma_attr);
			memcpy(pre_gamma_lut, gamma_table_160, sizeof(U16) * 160);	
		}
	}
#endif	
	return ISP_LIB_S_OK;
}
#endif
#if 0

/* @fn      isp_server_run
  * @brief 运行isp SDK内部线程
  * @param   [in] cancel,线程是否退出
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
VOID* isp_server_run(ISP_INNER_PARAM *inner_param)
{
	U32 cur_system_agc = 0;
	static S32 last_system_agc = -1;
	U32 ret = 0;
	U32 isp_ltm_table[128] = {0};

	/**set thread name*/
	//setPthreadName("isp_server");
	
	while(!inner_param->isp_server_engin.exit_flag)
	{
		//API_ISP_Run超时返回时，获取统计信息会出错，需进行判断
		if (API_ISP_Run() != ISP_LIB_S_OK)
		{
			PRINT_INFO(DBG_KEY, g_dbg_level,"API_ISP_Run ERROR_ISP_WAIT_PICEND_FAILED\n");
			continue;
		}
		hik_ae_run();
		hik_awb_run();

		ret = isp_get_current_db(&cur_system_agc);
		if(ret != ISP_LIB_S_OK)
		{
			continue;
		}

		if((PARAMS_NO_CHANGED != g_isp_params_change) || (cur_system_agc != last_system_agc))
		{	
			//绿平衡与增益联动
			isp_set_gb_param(cur_system_agc);
			
			//ltm新算法所需的tar_luma与增益联动
			isp_set_tar_luma(cur_system_agc);

			//锐化算子切换与增益联动
			isp_set_apc_param(inner_param, cur_system_agc);

			//vpss的scaler模块滤波器系数与增益联动
			isp_set_vpss_param(inner_param, cur_system_agc);

			//ltm曲线权重随增益联动
			isp_get_ltm_param(inner_param, cur_system_agc, isp_ltm_table);
			isp_set_ltm_param(inner_param, isp_ltm_table);
			g_isp_params_change &= _PARAM_LTM_CHANGED;
			
			last_system_agc = cur_system_agc;
		}

		//设置af统计信息给共享内存区域,首先要dsp已经创建了共享内存
		if(NULL != g_share_mem)
		{
			isp_write_af_stat_to_buf();
		}	

		isp_gbce_running(inner_param);

	}
	PRINT_INFO(DBG_KEY, g_dbg_level,"isp_server_run exit\n");
	return NULL;
}
#else

/* @fn      isp_server_run
  * @brief 运行isp SDK内部线程
  * @param   [in] cancel,线程是否退出
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
VOID* isp_server_run()
{
	IMPISPWaitFrameAttr sFrameAttr;
	memset(&sFrameAttr,0,sizeof(IMPISPWaitFrameAttr));
	sFrameAttr.timeout = 40;//阻塞40ms
	while(1)
	{
		usleep(10000);
		//IMP_ISP_Tuning_WaitFram获取帧VD,获取失败提前结束此次循环
		if (IMP_ISP_Tuning_WaitFrame(&sFrameAttr) != ISP_LIB_S_OK)
		{
			continue;
		}
		//启动HIK_AWB
		hik_awb_run();
	}
	return NULL;
}

#endif
#if 0
/* @fn      isp_debug_tool_en
  * @brief 图像调试工具开关
  * @param   [in] inner_param,isp内部参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_debug_tool_en(ISP_INNER_PARAM *inner_param,  S32 tools_en)
{
	//int ret = 0;
	/**调试工具服务器开启*/
	if(tools_en)
	{
		if(!inner_param->isp_image_debug.exit_flag)
		{
			PRINT_INFO(DBG_FUNCTION, g_dbg_level,"isp tunning tool already open,return!!\n");
			return ISP_LIB_S_OK;
		}
		inner_param->isp_image_debug.exit_flag = 0;
		pthread_create(&inner_param->isp_image_debug.id, NULL, (void* (*)(void*))udp_dbi_thread,&inner_param->isp_image_debug.exit_flag);
		PRINT_INFO(DBG_FUNCTION, g_dbg_level,"open isp tunning tool!!\n");
	}
	/**关闭调试工具*/
	else
	{
		inner_param->isp_image_debug.exit_flag = 1;
		PRINT_INFO(DBG_FUNCTION, g_dbg_level,"close isp tunning tool!!\n");
	}
	return ISP_LIB_S_OK;
}
#endif
#if 0
/* @fn      isp_config_sensor_params
  * @brief isp初始参数配置。
  * @param   [in] inner_param,isp内部参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_config_sensor_params(ISP_INNER_PARAM *inner_param)
{
	switch(gp_isp_inner_ctrl->sensor_id)
	{
		case PANASONIC_MN34425:
			gp_isp_cfg_tables->ae_param_linear = &gst_mn34425_ae_cfg;
			gp_isp_cfg_tables->awb_param_linear = &gst_mn34425_awb_cfg;	
			gp_isp_cfg_tables->contrast_param_linear = g_mn34425_contrast_param_linear;
			gp_isp_cfg_tables->contrast_param_wdr = g_mn34425_contrast_param_linear;
			gp_isp_cfg_tables->ltm_table_param_linear = g_mn34425_ltm_table_linear;
			gp_isp_cfg_tables->ltm_table_param_wdr = g_mn34425_ltm_table_linear;
			gp_isp_cfg_tables->gamma_table_param_linear = g_gamma_table_fh;
			gp_isp_cfg_tables->gamma_table_param_wdr = g_gamma_table_wdr;
			gp_isp_cfg_tables->gb_param_linear = g_mn34425_gb_param_linear;
			gp_isp_cfg_tables->sharpeness_param_linear = g_mn34425_sharpeness_param_linear;
			gp_isp_cfg_tables->sharpeness_param_wdr = g_mn34425_sharpeness_param_wdr;
			gp_isp_cfg_tables->nr3d_param_linear = g_mn34425_nr3d_param_linear;
			gp_isp_cfg_tables->nr3d_param_wdr = g_mn34425_nr3d_param_wdr;
			gp_isp_cfg_tables->nr2d_param_linear = g_mn34425_nr2d_param_linear;
			gp_isp_cfg_tables->nr2d_param_wdr = g_mn34425_nr2d_param_wdr;
			gp_isp_cfg_tables->ynr_param_linear = g_mn34425_ynr_param_linear;
			gp_isp_cfg_tables->ynr_param_wdr = g_mn34425_ynr_param_wdr;
			gp_isp_cfg_tables->lc_param_linear = &g_mn34425_lc_param_linear;
			gp_isp_cfg_tables->lc_param_wdr = &g_mn34425_lc_param_wdr;
			gp_isp_cfg_tables->vpss_param_linear = &g_mn34425_vpss_param_linear;
			gp_isp_cfg_tables->vpss_param_wdr = &g_mn34425_vpss_param_wdr;
			/**ae*/
			gp_isp_inner_ctrl->aec_ctrl.y_ref_map_linear = &luma_ref_mn34425_linear;
			gp_isp_inner_ctrl->aec_ctrl.y_ref_map_wdr = &luma_ref_mn34425_wdr;
			gp_isp_inner_ctrl->aec_ctrl.y_ref_map_linear_hlc = &luma_ref_mn34425_linear_hlc;
			gp_isp_inner_ctrl->aec_ctrl.y_ref_map_hlc = &luma_ref_mn34425_hlc;

			snprintf(gp_isp_cfg_tables->awb_line_config, CFG_NAME_LEN, "%s", AWB_CONFIG_MN34425_LINERA);
			snprintf(gp_isp_cfg_tables->awb_wdr_config, CFG_NAME_LEN, "%s", AWB_CONFIG_MN34425_WDR);
			snprintf(gp_isp_cfg_tables->ccm_line_config, CFG_NAME_LEN, "%s", CCM_CONFIG_MN34425_LINEAR);
			snprintf(gp_isp_cfg_tables->ccm_wdr_config, CFG_NAME_LEN, "%s", CCM_CONFIG_MN34425_WDR);
			break;
		
		case OMNI_OV2735:
			gp_isp_cfg_tables->ae_param_linear = &gst_ov2735_ae_cfg;
			gp_isp_cfg_tables->awb_param_linear = &gst_ov2735_awb_cfg;	
			gp_isp_cfg_tables->contrast_param_linear = g_ov2735_contrast_param_linear;
			gp_isp_cfg_tables->ltm_table_param_linear = g_ov2735_ltm_table_linear;
			gp_isp_cfg_tables->gamma_table_param_linear = g_gamma_table_hisi;
			gp_isp_cfg_tables->gb_param_linear = g_ov2735_gb_param_linear;
			gp_isp_cfg_tables->sharpeness_param_linear = g_ov2735_sharpeness_param_linear;
			gp_isp_cfg_tables->nr3d_param_linear = g_ov2735_nr3d_param_linear;
			gp_isp_cfg_tables->nr2d_param_linear = g_ov2735_nr2d_param_linear;
			gp_isp_cfg_tables->ynr_param_linear = g_ov2735_ynr_param_linear;
			gp_isp_cfg_tables->lc_param_linear = &g_ov2735_lc_param_linear;
			gp_isp_cfg_tables->vpss_param_linear = &g_ov2735_vpss_param_linear;
			/**ae*/
			gp_isp_inner_ctrl->aec_ctrl.y_ref_map_linear = &luma_ref_ov2735_linear;
			gp_isp_inner_ctrl->aec_ctrl.y_ref_map_linear_hlc = &luma_ref_ov2735_linear_hlc;
			
			if(LENS_PB401F == inner_param->lens_type)
			{
				snprintf(gp_isp_cfg_tables->awb_line_config, CFG_NAME_LEN, "%s", AWB_CONFIG_OV2735_PB401F_LINERA);
				snprintf(gp_isp_cfg_tables->awb_wdr_config, CFG_NAME_LEN, "%s", AWB_CONFIG_OV2735_PB401F_WDR);
				snprintf(gp_isp_cfg_tables->ccm_line_config, CFG_NAME_LEN, "%s", CCM_CONFIG_OV2735_LINEAR);
				snprintf(gp_isp_cfg_tables->ccm_wdr_config, CFG_NAME_LEN, "%s", CCM_CONFIG_OV2735_WDR);
			}
			else
			{
				snprintf(gp_isp_cfg_tables->awb_line_config, CFG_NAME_LEN, "%s", AWB_CONFIG_OV2735_LINERA);
				snprintf(gp_isp_cfg_tables->awb_wdr_config, CFG_NAME_LEN, "%s", AWB_CONFIG_OV2735_WDR);
				snprintf(gp_isp_cfg_tables->ccm_line_config, CFG_NAME_LEN, "%s", CCM_CONFIG_OV2735_LINEAR);
				snprintf(gp_isp_cfg_tables->ccm_wdr_config, CFG_NAME_LEN, "%s", CCM_CONFIG_OV2735_WDR);
			}
			break;
			
		case SOI_K02:
			gp_isp_cfg_tables->ae_param_linear = &gst_k02_ae_cfg;
			gp_isp_cfg_tables->awb_param_linear = &gst_k02_awb_cfg;	
			gp_isp_cfg_tables->contrast_param_linear = g_k02_contrast_param_linear;
			gp_isp_cfg_tables->ltm_table_param_linear = g_k02_ltm_table_linear;
			gp_isp_cfg_tables->gamma_table_param_linear = g_gamma_table_ti;
			gp_isp_cfg_tables->gb_param_linear = g_k02_gb_param_linear;
			gp_isp_cfg_tables->sharpeness_param_linear = g_k02_sharpeness_param_linear;
			gp_isp_cfg_tables->nr3d_param_linear = g_k02_nr3d_param_linear;
			gp_isp_cfg_tables->nr2d_param_linear = g_k02_nr2d_param_linear;
			gp_isp_cfg_tables->ynr_param_linear = g_k02_ynr_param_linear;
			gp_isp_cfg_tables->lc_param_linear = &g_k02_lc_param_linear;
			gp_isp_cfg_tables->vpss_param_linear = &g_k02_vpss_param_linear;
			/**ae*/
			gp_isp_inner_ctrl->aec_ctrl.y_ref_map_linear = &luma_ref_k02_linear;
			gp_isp_inner_ctrl->aec_ctrl.y_ref_map_linear_hlc = &luma_ref_k02_linear_hlc;

			snprintf(gp_isp_cfg_tables->awb_line_config, CFG_NAME_LEN, "%s", AWB_CONFIG_K02_LINERA);
			snprintf(gp_isp_cfg_tables->awb_wdr_config, CFG_NAME_LEN, "%s", AWB_CONFIG_K02_WDR);
			snprintf(gp_isp_cfg_tables->ccm_line_config, CFG_NAME_LEN, "%s", CCM_CONFIG_K02_LINEAR);
			snprintf(gp_isp_cfg_tables->ccm_wdr_config, CFG_NAME_LEN, "%s", CCM_CONFIG_K02_WDR);
			break;
		
		case OMNI_SP2309:
			gp_isp_cfg_tables->ae_param_linear = &gst_sp2309_ae_cfg;
			gp_isp_cfg_tables->awb_param_linear = &gst_sp2309_awb_cfg;	
			gp_isp_cfg_tables->contrast_param_linear = g_sp2309_contrast_param_linear;
			gp_isp_cfg_tables->ltm_table_param_linear = g_sp2309_ltm_table_linear;
			gp_isp_cfg_tables->gamma_table_param_linear = g_gamma_table_hisi;
			gp_isp_cfg_tables->gb_param_linear = g_sp2309_gb_param_linear;
			gp_isp_cfg_tables->sharpeness_param_linear = g_sp2309_sharpeness_param_linear;
			gp_isp_cfg_tables->nr3d_param_linear = g_sp2309_nr3d_param_linear;
			gp_isp_cfg_tables->nr2d_param_linear = g_sp2309_nr2d_param_linear;
			gp_isp_cfg_tables->ynr_param_linear = g_sp2309_ynr_param_linear;
			gp_isp_cfg_tables->lc_param_linear = &g_sp2309_lc_param_linear;
			gp_isp_cfg_tables->vpss_param_linear = &g_sp2309_vpss_param_linear;
			/**ae*/
			gp_isp_inner_ctrl->aec_ctrl.y_ref_map_linear = &luma_ref_sp2309_linear;
			gp_isp_inner_ctrl->aec_ctrl.y_ref_map_linear_hlc = &luma_ref_sp2309_linear_hlc;
			
			snprintf(gp_isp_cfg_tables->awb_line_config, CFG_NAME_LEN, "%s", AWB_CONFIG_SP2309_LINERA);
			snprintf(gp_isp_cfg_tables->awb_wdr_config, CFG_NAME_LEN, "%s", AWB_CONFIG_SP2309_WDR);
			snprintf(gp_isp_cfg_tables->ccm_line_config, CFG_NAME_LEN, "%s", CCM_CONFIG_SP2309_LINEAR);
			snprintf(gp_isp_cfg_tables->ccm_wdr_config, CFG_NAME_LEN, "%s", CCM_CONFIG_SP2309_WDR);
			break;
			
		default:
			break;
	}
	return ISP_LIB_S_OK;
}
#endif

/************************************************************************************************
 * 功 能： 加载配置文件
 * 参 数：
 *
 *         inner_param  - 内部参数
 *
 * 返回值：返回状态码
 *************************************************************************************************/
HRESULT isp_load_config_files(ISP_INNER_PARAM *inner_param)
{
	S32 ret = ISP_LIB_S_OK;
	//load gbce config
	//ret = isp_switch_gbce_table(inner_param, CURVE_HISI);	// 初始化加载hisi曲线
	//CHECK_RET(ret < 0, ISP_LIB_S_FAIL);
	
	//load awb config
	ret = isp_load_awb_table(inner_param);
	CHECK_RET(ret < 0, ISP_LIB_S_FAIL);

	// load ccm config
	ret = isp_load_ccm_table(inner_param);
	CHECK_RET(ret < 0, ISP_LIB_S_FAIL);
	
	// remove awb、ccm config
	//ret = isp_remove_cfg_file(inner_param);
	//CHECK_RET(ret < 0, ISP_LIB_S_FAIL);
	return ISP_LIB_S_OK;
}

#if 0
/* @fn      isp_sensor_register_cb
  * @brief sensor操作回调函数
  * @param   [in] inner_param,isp内部参数.
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_sensor_register_cb(ISP_INNER_PARAM * inner_param)
{
	S32 ret = 0;
	struct isp_sensor_if sensor_func;
	memset(&sensor_func, 0, sizeof(struct isp_sensor_if));
	sensor_func.init = gp_isp_inner_ctrl->funcs.pfn_init;
	sensor_func.get_gain = gp_isp_inner_ctrl->funcs.pfn_get_gain;
	sensor_func.get_intt = gp_isp_inner_ctrl->funcs.pfn_get_intt;
	sensor_func.set_gain = gp_isp_inner_ctrl->funcs.pfn_set_gain;
	sensor_func.set_intt = gp_isp_inner_ctrl->funcs.pfn_set_intt;
	sensor_func.set_sns_fmt = gp_isp_inner_ctrl->funcs.pfn_set_sns_fmt;
	sensor_func.set_sns_reg = sensor_write_reg;/**API_ISP_SetSensorReg 使用*/
	sensor_func.set_exposure_ratio = gp_isp_inner_ctrl->funcs.pfn_set_exposure_ratio;
	sensor_func.get_exposure_ratio = gp_isp_inner_ctrl->funcs.pfn_get_exposure_ratio;
	sensor_func.get_sensor_attribute = gp_isp_inner_ctrl->funcs.pfn_get_sensor_attribute;
	
	ret = API_ISP_SensorRegCb((U32)gp_isp_inner_ctrl->sensor_id,&sensor_func);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	return ISP_LIB_S_OK;
}

/* @fn      isp_set_default_param
  * @brief 获取isp默认参数.
  * @param   [in] inner_param,isp内部参数配置
  * @param   [in] gst_isp_def_param,isp默认参数
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_set_default_param(ISP_INNER_PARAM *inner_param)
{
	S32 ret = 0;
	ISP_DEFAULT_PARAM st_isp_def_param;
	///* 1. sensor reg init */
	switch(gp_isp_inner_ctrl->sensor_id)
	{
		case PANASONIC_MN34425:
			ret = get_mn34425_default_param(&st_isp_def_param, HIK_FALSE);
			break;
		case OMNI_OV2735:
			ret = get_ov2735_default_param(&st_isp_def_param, HIK_FALSE);
			break;
		case SOI_K02:
			ret = get_k02_default_param(&st_isp_def_param, HIK_FALSE);
			break;
		case OMNI_SP2309:
			ret = get_sp2309_default_param(&st_isp_def_param, HIK_FALSE);
			break;
		default:
			ret = ISP_LIB_S_FAIL;
			break;
	}
	CHECK_RET(ret != ISP_LIB_S_OK, ret);

	ret = API_ISP_SetIspParam(&st_isp_def_param);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	
	ret = isp_set_wdr_param(inner_param);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);

	ret = isp_set_anti_purple(inner_param);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);

	ret = isp_set_lc_param(inner_param);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);

	//apc算子恢复默认状态
	g_apc_selection = 1;
		
	return ISP_LIB_S_OK;
}


//FH_UINT16 Sensor_Read(FH_UINT16 addr)
//{
	//return 0;
//}


/* @fn      isp_sensor_init_cfg
 * @brief 配置不同sensor的capture mode、wdr、图像宽高等初始值。
 * @param   [in] inner_param,isp内部结构体
 * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_sensor_init_cfg(ISP_INNER_PARAM *inner_param)
{
	/**内部线程退出标志开关*/
	inner_param->isp_server_engin.exit_flag = 0;
	inner_param->isp_inner_task.exit_flag = 0;
	inner_param->isp_image_debug.exit_flag = 1;
	inner_param->isp_debug.exit_flag = 0;

	/**isp内部自动曝光状态*/
	inner_param->aaa_param.ae_param.auto_enable = 1;
	inner_param->img_enhance.drc_mode = 0;
	inner_param->aaa_param.awb_param.mode = HIK_WB_AWB1_MODE;
	
	//ltm related params change or not
	g_isp_params_change = PARAMS_NO_CHANGED;

	//默认开启blc手动校正机制，暂不使能
	gp_isp_inner_ctrl->manual_blc_en = 1;

	//if(LENS_PB401F == inner_param->lens_type)
	//{
	//	memcpy(&g_isp_ae_weight_table, &g_isp_ae_weight_table_fisheye, 8*sizeof(isp_ae_weight_table_t));
	//}
	
	/**定义sensor输出的最大宽高，DSP和ISP分配buff时用*/
	switch(gp_isp_inner_ctrl->sensor_id)
	{	
		#if 0
		case PANASONIC_MN34425:
			gp_isp_inner_ctrl->max_image_size.height = 1080;
			gp_isp_inner_ctrl->max_image_size.width = 1920;
			gp_isp_inner_ctrl->isp_capture_mode = VI_CAPTURE_MODE_1080P25;
			inner_param->aaa_param.ae_param.anti_flicker_mode = HIK_ANTI_FLICKER_50HZ;
			gp_isp_inner_ctrl->frame_rate = 25;
			gp_isp_inner_ctrl->wdr_mode = WDR_MODE_NONE;
			inner_param->img_enhance.auto_local_exp_mode = WDR_MODE_NONE;

			gp_isp_inner_ctrl->bus_attr.addr_len = 2;
			gp_isp_inner_ctrl->bus_attr.data_len = 1;
			gp_isp_inner_ctrl->bus_attr.device_addr = MN34425_I2C_ADDR;
			gp_isp_inner_ctrl->bus_attr.trans_type = SENSOR_TRANS_I2C;

			gp_isp_inner_ctrl->funcs.pfn_init = sensor_init_mn34425;
			gp_isp_inner_ctrl->funcs.pfn_set_gain = sensor_set_gain_mn34425;
			gp_isp_inner_ctrl->funcs.pfn_get_gain = sensor_get_gain_mn34425;
			gp_isp_inner_ctrl->funcs.pfn_set_intt = sensor_set_intt_mn34425;
			gp_isp_inner_ctrl->funcs.pfn_get_intt = sensor_get_intt_mn34425;
			gp_isp_inner_ctrl->funcs.pfn_set_sns_fmt = sensor_set_fmt_mn34425;
			gp_isp_inner_ctrl->funcs.pfn_capture_mode_cfg = cfg_capuremode_mn34425;
			gp_isp_inner_ctrl->funcs.pfn_set_exposure_ratio = sensor_set_exposure_ratio_mn34425;
			gp_isp_inner_ctrl->funcs.pfn_get_exposure_ratio = sensor_get_exposure_ratio_mn34425;
			gp_isp_inner_ctrl->funcs.pfn_get_sensor_attribute = sensor_get_attribute_mn34425;
			
			break;
		#endif
		case OMNI_OV2735:
			gp_isp_inner_ctrl->max_image_size.height = 1080;
			gp_isp_inner_ctrl->max_image_size.width = 1920;
			gp_isp_inner_ctrl->isp_capture_mode = VI_CAPTURE_MODE_1080P25;
			inner_param->aaa_param.ae_param.anti_flicker_mode = HIK_ANTI_FLICKER_50HZ;
			gp_isp_inner_ctrl->frame_rate = 25;
			gp_isp_inner_ctrl->wdr_mode = WDR_MODE_NONE;
			inner_param->img_enhance.auto_local_exp_mode = WDR_MODE_NONE;
			
			gp_isp_inner_ctrl->bus_attr.addr_len = 1;
			gp_isp_inner_ctrl->bus_attr.data_len = 1;
			gp_isp_inner_ctrl->bus_attr.device_addr = OV2735_I2C_ADDR;
			gp_isp_inner_ctrl->bus_attr.trans_type = SENSOR_TRANS_I2C;

			gp_isp_inner_ctrl->funcs.pfn_init = sensor_init_ov2735;
			gp_isp_inner_ctrl->funcs.pfn_set_gain = sensor_set_gain_ov2735;
			gp_isp_inner_ctrl->funcs.pfn_get_gain = sensor_get_gain_ov2735;
			gp_isp_inner_ctrl->funcs.pfn_set_intt = sensor_set_intt_ov2735;
			gp_isp_inner_ctrl->funcs.pfn_get_intt = sensor_get_intt_ov2735;
			gp_isp_inner_ctrl->funcs.pfn_set_sns_fmt = sensor_set_fmt_ov2735;
			gp_isp_inner_ctrl->funcs.pfn_capture_mode_cfg = cfg_capuremode_ov2735;
			gp_isp_inner_ctrl->funcs.pfn_set_exposure_ratio = sensor_set_exposure_ratio_ov2735;
			gp_isp_inner_ctrl->funcs.pfn_get_exposure_ratio = sensor_get_exposure_ratio_ov2735;
			gp_isp_inner_ctrl->funcs.pfn_get_sensor_attribute = sensor_get_attribute_ov2735;
			
			break;
		#if 0
		case SOI_K02:
			gp_isp_inner_ctrl->max_image_size.height = 1520;
			gp_isp_inner_ctrl->max_image_size.width = 2048;
			gp_isp_inner_ctrl->isp_capture_mode = VI_CAPTURE_MODE_2048X1536P25;
			inner_param->aaa_param.ae_param.anti_flicker_mode = HIK_ANTI_FLICKER_50HZ;
			gp_isp_inner_ctrl->frame_rate = 25;
			gp_isp_inner_ctrl->wdr_mode = WDR_MODE_NONE;
			inner_param->img_enhance.auto_local_exp_mode = WDR_MODE_NONE;

			gp_isp_inner_ctrl->bus_attr.addr_len = 1;
			gp_isp_inner_ctrl->bus_attr.data_len = 1;
			gp_isp_inner_ctrl->bus_attr.device_addr = K02_I2C_ADDR;
			gp_isp_inner_ctrl->bus_attr.trans_type = SENSOR_TRANS_I2C;

			gp_isp_inner_ctrl->funcs.pfn_init = sensor_init_k02;
			gp_isp_inner_ctrl->funcs.pfn_set_gain = sensor_set_gain_k02;
			gp_isp_inner_ctrl->funcs.pfn_get_gain = sensor_get_gain_k02;
			gp_isp_inner_ctrl->funcs.pfn_set_intt = sensor_set_intt_k02;
			gp_isp_inner_ctrl->funcs.pfn_get_intt = sensor_get_intt_k02;
			gp_isp_inner_ctrl->funcs.pfn_set_sns_fmt = sensor_set_fmt_k02;
			gp_isp_inner_ctrl->funcs.pfn_capture_mode_cfg = cfg_capuremode_k02;
			gp_isp_inner_ctrl->funcs.pfn_set_exposure_ratio = sensor_set_exposure_ratio_k02;
			gp_isp_inner_ctrl->funcs.pfn_get_exposure_ratio = sensor_get_exposure_ratio_k02;
			gp_isp_inner_ctrl->funcs.pfn_get_sensor_attribute = sensor_get_attribute_k02;
			
			break;
		
		case OMNI_SP2309:
			gp_isp_inner_ctrl->max_image_size.height = 1080;
			gp_isp_inner_ctrl->max_image_size.width = 1920;
			gp_isp_inner_ctrl->isp_capture_mode = VI_CAPTURE_MODE_1080P25;
			inner_param->aaa_param.ae_param.anti_flicker_mode = HIK_ANTI_FLICKER_50HZ;
			gp_isp_inner_ctrl->frame_rate = 25;
			gp_isp_inner_ctrl->wdr_mode = WDR_MODE_NONE;
			inner_param->img_enhance.auto_local_exp_mode = WDR_MODE_NONE;
			
			gp_isp_inner_ctrl->bus_attr.addr_len = 1;
			gp_isp_inner_ctrl->bus_attr.data_len = 1;
			gp_isp_inner_ctrl->bus_attr.device_addr = SP2309_I2C_ADDR;
			gp_isp_inner_ctrl->bus_attr.trans_type = SENSOR_TRANS_I2C;

			gp_isp_inner_ctrl->funcs.pfn_init = sensor_init_sp2309;
			gp_isp_inner_ctrl->funcs.pfn_set_gain = sensor_set_gain_sp2309;
			gp_isp_inner_ctrl->funcs.pfn_get_gain = sensor_get_gain_sp2309;
			gp_isp_inner_ctrl->funcs.pfn_set_intt = sensor_set_intt_sp2309;
			gp_isp_inner_ctrl->funcs.pfn_get_intt = sensor_get_intt_sp2309;
			gp_isp_inner_ctrl->funcs.pfn_set_sns_fmt = sensor_set_fmt_sp2309;
			gp_isp_inner_ctrl->funcs.pfn_capture_mode_cfg = cfg_capuremode_sp2309;
			gp_isp_inner_ctrl->funcs.pfn_set_exposure_ratio = sensor_set_exposure_ratio_sp2309;
			gp_isp_inner_ctrl->funcs.pfn_get_exposure_ratio = sensor_get_exposure_ratio_sp2309;
			gp_isp_inner_ctrl->funcs.pfn_get_sensor_attribute = sensor_get_attribute_sp2309;
			
			break;
		#endif
		default:
			PRINT_INFO(DBG_KEY, g_dbg_level,"not support such sensor 0x%x!\n", gp_isp_inner_ctrl->sensor_id);
			break;
	}
	return ISP_LIB_S_OK;
}


/* @fn      isp_load_isp_bin
 * @brief 加载isp参数，配置功能模块使能和ae模式
 * @param   [in] void
 * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_load_isp_bin(void)
{
	FILE *param_file;
	char *filename  = "/config/isp.hex";
	char *isp_param_buff;
	U32 param_addr = 0;
	U32 param_size = 0;
	
	API_ISP_GetBinAddr(&param_addr,&param_size);
	isp_param_buff = (char*)malloc(param_size);
	CHECK_RET(isp_param_buff == NULL, -1);
	param_file = fopen(filename,"rb");
	if(param_file == NULL)
	{
		printf("open file failed!\n");
		free(isp_param_buff);
		return ISP_LIB_S_FAIL;
	}
	
	if (param_size != fread(isp_param_buff,1,param_size,param_file))
	{
		printf("open file failed!\n");
		free(isp_param_buff);
		fclose(param_file);
		return ISP_LIB_S_FAIL;
	}
	
	//PRINT_INFO(DBG_KEY,g_dbg_level, "load param %s\n", filename);
	API_ISP_LoadIspParam(isp_param_buff);
	free(isp_param_buff);
	fclose(param_file);
	return ISP_LIB_S_OK;
}
#endif

#if 0
/* @fn      isp_create_server_thread
 * @brief 创建ISP SDK线程
 * @param   [in] inner_param,isp内部结构体
 * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_create_server_thread(ISP_INNER_PARAM *inner_param)
{
	#if 0
	S32 ret_api;
	U32 stack_size = 128*1024; //100k

	if(inner_param->isp_server_engin.id == 0)
	{
		inner_param->isp_server_engin.exit_flag = 0;
		/**create thread*/
		g_thread_isp = rt_thread_create("isp_server_run", isp_server_run, inner_param,
                                    stack_size, RT_SDK_PRIORITY, 10);
		if(g_thread_isp != 0)
		{
			rt_thread_startup(g_thread_isp);
			PRINT_INFO(DBG_KEY,g_dbg_level, "isp_create_server_thread ok\n");
		}
		else
		{
			PRINT_INFO(DBG_KEY,g_dbg_level, " ERROR: Can't create thread <isp_server> !\n");
			return ISP_LIB_S_FAIL;
		}
	}
	#endif
	return ISP_LIB_S_OK;
}
#endif
void isp_create_server_thread(ISP_INNER_PARAM *inner_param)
{
	#if 01
	int ret = -1;
	pthread_t tid_3A;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//detach 系统关注资源释放状态
	ret = pthread_create(&tid_3A, &attr, isp_server_run, NULL);
	if (ret != 0) 
	{
		PRINT_INFO(DBG_KEY,g_dbg_level, "isp_create_server_thread failed\n");
		return;
	}
	pthread_join(tid_3A, NULL);
	return;
	#endif
}

#if 0
/* @fn      isp_create_inner_thread
 * @brief 创建ISP内部线程
 * @param   [in] inner_param,isp内部结构体
 * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_create_inner_thread(ISP_INNER_PARAM *inner_param)
{
	S32 ret_api;
	U32 stack_size = 128*1024; //100k

	if(inner_param->isp_inner_task.id == 0)
	{
		inner_param->isp_inner_task.exit_flag = 0;
		/**create thread*/
		ret_api = pthreadSpawn(&inner_param->isp_inner_task.id, 70, stack_size, 
			(void* (*)(void*))isp_inner_loop, 1, &inner_param->isp_inner_task.exit_flag);
		if(ret_api != ISP_LIB_S_OK)
		{
			PRINT_INFO(DBG_KEY,g_dbg_level, " ERROR: Can't create thread <isp_inner> !\n");
			return ISP_LIB_S_FAIL;
		}
		PRINT_INFO(DBG_KEY,g_dbg_level, "isp_create_inner_thread ok\n");
	}
	return ISP_LIB_S_OK;
}

/* @fn      isp_sensor_clk
  * @brief isp初始参数配置。
  * @param   [in] void
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_sensor_clk(void)
{
	S32 ret;
	U08 cisClk = CIS_CLK_36M;
	
	switch(gp_isp_inner_ctrl->sensor_id)
	{
		case PANASONIC_MN34425:
			cisClk = CIS_CLK_24M;
			break;
		case OMNI_OV2735:
			cisClk = CIS_CLK_24M;
			break;
		case SOI_K02:
			cisClk = CIS_CLK_24M;
			break;
		case OMNI_SP2309:
			cisClk = CIS_CLK_24M;
			break;
		default:
			PRINT_INFO(DBG_KEY,g_dbg_level, " error: unsupport sensor id - %d\n",gp_isp_inner_ctrl->sensor_id);
			break;
	}
	
	PRINT_INFO(DBG_KEY,g_dbg_level, " isp_sensor_clk %d\n",cisClk);
	
	ret = API_ISP_SetCisClk(cisClk);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	
	return ISP_LIB_S_OK;
}
#endif


/* @fn      isp_get_io_handle
 * @brief 获取io句柄
 * @param   [in] inner_param,isp内部结构体
 * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_get_io_handle(VOID)
{
	/* HIKIO fd open */
	if (isp_hikio_fd < 0)
	{
		isp_hikio_fd = open(DEVICE_HIKIO, NULL, O_RDWR);
		if (isp_hikio_fd < 0)
		{
			PRINT_INFO(DBG_KEY,g_dbg_level, "打开设备%s失败\n", DEVICE_HIKIO);
			return ISP_LIB_S_FAIL;
		}
	}
	return ISP_LIB_S_OK;
}

IMPISPWeight g_jz_ae_weight[1]=
{
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,3,3,3,3,3,3,3,1,1,1,1},
		{1,1,1,3,5,5,5,5,5,5,5,3,1,1,1},
		{1,1,1,3,5,5,5,5,5,5,5,3,1,1,1},
		{1,1,1,3,5,5,5,5,5,5,5,3,1,1,1},
		{1,1,1,3,5,5,5,5,5,5,5,3,1,1,1},
		{1,1,1,3,5,5,5,5,5,5,5,3,1,1,1},
		{1,1,1,3,5,5,5,5,5,5,5,3,1,1,1},
		{1,1,1,3,5,5,5,5,5,5,5,3,1,1,1},
		{1,1,1,1,3,3,3,3,3,3,3,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

HRESULT JZ_ISP_Set_AEWeight()
{
	int ret = -1;
	//IMPISPWeight ae_weight;
	ret = IMP_ISP_Tuning_SetAeWeight(g_jz_ae_weight);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
}

/* @fn      isp_start_isp
 * @brief 初始化ISP相关配置参数，启动ISP
 * @param   [in] inner_param,isp内部结构体
 * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_start_isp(ISP_INNER_PARAM *inner_param)
{
	s32 ret;
	SENSOR_CFG_S sensor_cfg = {0};
	//ISP_VI_ATTR_S stViAttr;
	//MIRROR_CFG_S mirror_cfg = {0};
#if 0
	//isp 打开需要依赖的动态库文件
	ret = isp_open_gpio_dynamic_lib();
	CHECK_RET(ret != ISP_LIB_S_OK, ret);

	//isp 将声明的函数指向动态库里的具体实现函数
	isp_set_gpio_dynamic_func();
	//初始化hikio表，表索引值依据设备确定。若dsp初始化完成，则isp可不再初始化。
	ret = isp_init_dynamic_lib_hikio(inner_param->res_type);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
#endif
	/**ISP 初始化配置表*/ 
	//ret = isp_config_sensor_params(inner_param);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);

	/**加载配置文件*/ 
	ret = isp_load_config_files(inner_param);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);

	/**coolview图像调试工具使能，默认不开启*/
	//ret = isp_debug_tool_en(inner_param,ISP_MOD_OPEN);
	//CHECK_RET(ret != ISP_LIB_S_OK, ret);
	
	/**(1)ISP memory 分配*/
	//ret = API_ISP_MemInit(gp_isp_inner_ctrl->max_image_size.width, gp_isp_inner_ctrl->max_image_size.height);
	//CHECK_RET(ret != ISP_LIB_S_OK, ret);

	/**sensor回调函数注册*/
	//ret = isp_sensor_register_cb(inner_param);
	//CHECK_RET(ret != ISP_LIB_S_OK, ret);
	
	/**(2)SDK isp 初始化*/
	//ret = API_ISP_SensorInit();
	//CHECK_RET(ret != ISP_LIB_S_OK, ret);

	#if 0
	/**设置vi参数，包括图像宽高、bayer格式等*/
	sensor_cfg.capture_mode = gp_isp_inner_ctrl->isp_capture_mode;
	sensor_cfg.wdr_mode = gp_isp_inner_ctrl->wdr_mode;
	sensor_cfg.sensor_id =  gp_isp_inner_ctrl->sensor_id;
	ret = isp_get_viu_attr(sensor_cfg, &stViAttr);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	
	ret = API_ISP_SetViAttr(&stViAttr);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	
	/**(3)ISP模块初始化*/
	ret = API_ISP_Init();
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	//设置API_ISP_SetViAttr后，必须配置API_ISP_MirrorEnable，只要配置bayer pattern，API_ISP_MirrorEnable必须调用。
	mirror_cfg.bEN = gp_isp_inner_ctrl->mirror_en;
	mirror_cfg.mirror_bayer = g_mirror_offset2fmt[stViAttr.enBayerType][OFFSET_X];
	mirror_cfg.normal_bayer = stViAttr.enBayerType;
	ret = API_ISP_MirrorEnable(&mirror_cfg);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);

	/**设置isp初始化参数*/
	//isp_load_isp_bin();
	ret = isp_set_default_param(inner_param);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	
	/**配置3A参数*/
	ret = API_ISP_AESendCmd(SET_AE_DEFAULT, gp_isp_cfg_tables->ae_param_linear);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	hik_ae_init(inner_param);
	ret = API_ISP_AWBSendCmd(SET_AWB_DEFAULT, gp_isp_cfg_tables->awb_param_linear);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	hik_awb_init();
	ret = API_ISP_AFAlgEn(FH_TRUE);//AF统计信息enable
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	#else	
	hik_awb_init();
	#endif

	//GBCE init
	//isp_init_gbce(inner_param);
	//gbce 在线性模式下均开启，放初始化之后
	inner_param->img_enhance.gbce_enable = 0;
	
	/**创建SDK内部ISP线程*/
	isp_create_server_thread(inner_param);


	ret = isp_refresh_awb_param(inner_param, inner_param->img_enhance.auto_local_exp_mode);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	
	/**isp驱动存在问题，如果再创建一个线程，会导致stat信息偶尔丢失。*/
	/**创建与agc联动线程*/
	//ret = isp_create_inner_thread(inner_param);
	//CHECK_RET(ret != ISP_LIB_S_OK, ret);

	printf("ISP_Create thread OK\n");
	
	return ISP_LIB_S_OK;
}

#if 0
/* @fn      isp_refresh_fh_param
 * @brief 刷新fh的isp模块参数，在API_ISP_SoftReset后使用
 * @param   [in] inner_param,isp内部结构体
 * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_refresh_fh_param(ISP_INNER_PARAM *inner_param)
{
	HRESULT ret;
	ISP_HLR_CFG isp_hlr_cfg = {0};

	/**设置isp初始化参数*/
	//isp_load_isp_bin();
	ret = isp_set_default_param(inner_param);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);

	/**配置3A参数*/
	ret = API_ISP_AESendCmd(SET_AE_DEFAULT, gp_isp_cfg_tables->ae_param_linear);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	/**关闭内置的AE算法*/
	API_ISP_AEAlgEn(FH_FALSE);
	
	ret = API_ISP_AWBSendCmd(SET_AWB_DEFAULT, gp_isp_cfg_tables->awb_param_linear);
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	/**关闭内置的AWB、CCM算法*/
	API_ISP_AWBAlgEn(FH_FALSE);
		
	/**设置高光恢复阈值为最大，修复回补黑电平亮度时画面成黑白*/
	isp_hlr_cfg.bHlrEn = FH_TRUE;
	isp_hlr_cfg.s08Level = 0x3F;
	API_ISP_SetHlrCfg(&isp_hlr_cfg);
	return ISP_LIB_S_OK;
}
#endif
#if 0
/* @fn      isp_reload_sensor_param
 * @brief 重新刷新sensor前端参数
 * @param   [in] inner_param,isp内部结构体
 * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_reload_sensor_param(ISP_INNER_PARAM *inner_param)
{
	//wdr
	if(inner_param->img_enhance.auto_local_exp_mode)
	{	
		ISP_SetKeyParam(inner_param,ISP_WDR_MODE_I,&inner_param->img_enhance.auto_local_exp_mode);		
		ISP_SetKeyParam(inner_param,ISP_HIGH_LIGHT_CONTROL_EN_I,&inner_param->img_enhance.high_light_control_enable);
	}
	else
	{
		ISP_SetKeyParam(inner_param,ISP_BACK_LIGHT_MODE_I,&inner_param->img_enhance.backlight_comp_mode);
	}

	if(inner_param->img_enhance.drc_mode)
	{
		ISP_SetKeyParam(inner_param,ISP_WDR_LEVEL_I,&inner_param->img_enhance.drc_level);
	}

	ISP_SetKeyParam(inner_param,ISP_AE_SHUT_SLOWEST_I,&inner_param->aaa_param.ae_param.shutter_time_max);
	ISP_SetKeyParam(inner_param,ISP_AE_SHUT_FASTEST_I,&inner_param->aaa_param.ae_param.shutter_time_min);
	ISP_SetKeyParam(inner_param,ISP_AE_GAIN_MAX_LEVEL_I,&inner_param->aaa_param.ae_param.gain_max);
	ISP_SetKeyParam(inner_param,ISP_AWB_MODE_I,&inner_param->aaa_param.awb_param.mode);
	
	ISP_SetKeyParam(inner_param,ISP_AE_BRIGHTNESS_LEVEL_I,&inner_param->img_quality.brightness);
	ISP_SetKeyParam(inner_param,ISP_CONTRAST_LEVEL_I,&inner_param->img_quality.contrast);
	ISP_SetKeyParam(inner_param,ISP_SATURATION_LEVEL_I,&inner_param->img_quality.saturation);
	ISP_SetKeyParam(inner_param,ISP_SHARPEN_LEVEL_I,&inner_param->img_quality.sharpness);

	ISP_SetKeyParam(inner_param,ISP_SPATIAL_NR_LEVEL_I,&inner_param->img_enhance.spatial_denoise_strength);
	ISP_SetKeyParam(inner_param,ISP_TEMPORAL_NR_LEVEL_I,&inner_param->img_enhance.temporal_denoise_strength);
	return ISP_LIB_S_OK;
}
#endif

/* @fn      isp_set_sensor_reset
 * @brief Sensor复位函数
 * @param   [in] inner_param,isp内部结构体
 * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
HRESULT isp_set_sensor_reset(ISP_INNER_PARAM *inner_param,S32 param_val)
{
	if( param_val )// need to reset sensor
	{
		switch(gp_isp_inner_ctrl->sensor_id)
		{
			//case PANASONIC_MN34425:
			//	sensor_init_mn34425();
			//	break;
			//case OMNI_OV2735:
			//	sensor_init_ov2735();
			//	break;
			//case SOI_K02:
			//	sensor_init_k02();
			//	break;
			//case OMNI_SP2309:
			//	sensor_init_sp2309();
			//	break;
			default:
				PRINT_INFO(DBG_KEY,g_dbg_level, " error: unsupport sensor id - %d\n",gp_isp_inner_ctrl->sensor_id);
				break;
		}
		PRINT_INFO(DBG_KEY,g_dbg_level, " Reset the sensor \n");	
		//isp_reload_sensor_param(inner_param);
	}
	else//no need to reset sensor
	{
		//do nothing;
	}
	return ISP_LIB_S_OK;
}

#if 0
/* @fn      Sensor_Read
  * @brief 读sensor寄存器
  * @param   [in] addr,sesor寄存器地址。
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */

U16 Sensor_Read(U16 addr)
{
	S32 ret = 0;
	U16 data = 0;

	switch(gp_isp_inner_ctrl->sensor_id)
	{
		case PANASONIC_MN34425:
			ret = sensor_read_reg(addr,&data);
			break;
		case OMNI_OV2735:
			ret = sensor_read_reg(addr,&data);
			break;
		case SOI_K02:
			ret = sensor_read_reg(addr,&data);
			break;
		case OMNI_SP2309:
			ret = sensor_read_reg(addr,&data);
			break;
		default:
			PRINT_INFO(DBG_KEY,g_dbg_level, " error: unsupport sensor id - %d\n",gp_isp_inner_ctrl->sensor_id);
			break;
	}
	CHECK_RET(ret != ISP_LIB_S_OK, ret);
	return data;
}

/* @fn      Sensor_Write
  * @brief 写sensor寄存器地址
  * @param   [in] addr,寄存器地址
  * @param   [in] data,寄存器的值
  * @return  ISP_LIB_S_OK/ISP_LIB_S_FAIL.
 */
VOID Sensor_Write(FH_UINT16 addr, FH_UINT16 data)
{
	S32 ret = 0;
	
	PRINT_INFO(DBG_AE_REG,g_dbg_level, " Sensor_Write addr 0x%x, data 0x%x\n",addr,data);

	switch(gp_isp_inner_ctrl->sensor_id)
	{
		case PANASONIC_MN34425:
			ret = sensor_write_reg(addr,data);
			break;
		case OMNI_OV2735:
			ret = sensor_write_reg(addr,data);
			break;
		case SOI_K02:
			ret = k02_reg_to_buf(addr,data);
			break;
		case OMNI_SP2309:
			ret = sensor_write_reg(addr,data);
			break;
		default:
			PRINT_INFO(DBG_KEY,g_dbg_level, " error: unsupport sensor id - %d\n",gp_isp_inner_ctrl->sensor_id);
			break;
	}
	return;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

