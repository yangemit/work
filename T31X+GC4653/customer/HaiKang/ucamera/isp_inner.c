/***********************************************************
 * @file isp_inner.c
 * @note HangZhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 * @brief isp模块内部接口函数实现
 *
 * @Modification History:
 * @<version>   <time>      <author>      <desc>
 * @  1.0.0 - 2015/07/13 - [Ding sisi] - Create file
 *
 *
 * @warning:
 *     All rights reserved. No Part of this file may be reproduced, stored
 *     in a retrieval system, or transmitted, in any form, or by any means,
 *     electronic, mechanical, photocopying, recording, or otherwise,
 *     without the prior consent of Hikvision, Inc.
 *
 ***********************************************************/

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


/* @fn      isp_free_mem_tab
  * @brief 释放给memtab结构体分配的内存空间
  * @param   [in] MEM_TAB,待释放的结构体地址
  * @param   [in] cnt,结构体个数
  * @note 
  * @return  void.
 */
void isp_free_mem_tab(MEM_TAB *memTab, U32 cnt)
{
	U32 i;
	for (i = 0; i < cnt; i++)
	{
		if (memTab[i].base == 0 || memTab[i].size == 0)
		{
			continue;
		}

		SAFE_FREE(memTab[i].base);

		memTab[i].base = NULL;
		memTab[i].size = 0;
	}

	return;
}


/* @fn      isp_alloc_mem_tab
  * @brief 给memtab结构体分配内存函数
  * @param   [in] memtab ,待分配内存的结构体
  * @param   [in] cnt,结构体个数
  * @note 
  * @return void.
 */

HRESULT isp_alloc_mem_tab(MEM_TAB *memTab, U32 cnt)
{
	U32 i;

	for (i = 0; i < cnt; i++)
	{
		if (memTab[i].size == 0)
		{
			continue;
		}

		PRINT_INFO(DBG_KEY,g_dbg_level,"isp_alloc_mem_tab: space=%d, size=%d, align=%d \n", memTab[i].space,
		memTab[i].size, memTab[i].alignment);

		memTab[i].base = malloc(memTab[i].size);

		if (NULL == memTab[i].base)
		{
			PRINT_INFO(DBG_KEY,g_dbg_level,"isp_alloc_mem_tab FAILED, i=%d, space=%d, size=%d", i, memTab[i].space, memTab[i].size);
			return ISP_LIB_E_MEM_NULL;
		}
	}

	return ISP_LIB_S_OK;
}


#ifdef __cplusplus
#if __cplusplus
 }
#endif
#endif /* End of #ifdef __cplusplus */
 
