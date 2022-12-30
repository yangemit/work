#ifndef _AF_SRC_H_
#define _AF_SRC_H_

/**************************************************************
 * Name		: void autoFocus(void *arg)
 * Describe	: 自动聚焦策略
 * Parameters	: None
 * Return    	: None
 * Notice		: None
 * Global    	: None.
 * Created   	: xuwenqiang6
 **************************************************************/
void autoFocus(void *arg);

/**************************************************************
 * Name		: long long int GetAFD(int chan)
 * Describe	:获取图像清晰度，获取af_metrics,可以用来表示当前帧聚焦的清晰度;
 * Parameters	: None
 * Return    	: afd_value
 * Notice		: None
 * Global    	: None.
 * Created   	: xuwenqiang6
 **************************************************************/
long long int GetAFD(int chan);

/**************************************************************
 * Name		: long long int GetAFDAlt(int chan)
 * Describe	:获取图像清晰度，获取af_metrics_alt,可以用来表示当前帧聚焦的清晰度，
 			 由于可能存在误差，可以取三次平均af_metrics，获取的是AF次统计值;
 * Parameters	: None
 * Return    	: afd_value
 * Notice		: None
 * Global    	: None.
 * Created   	: xuwenqiang6
 **************************************************************/
long long int GetAFDAlt(int chan);

/*********************************************************************************
 * Name		: long long int AFVCMGetAFD(int chan)
 * Describe	: 获取多窗口的下AFD
 * Parameters	: 	
 * Return    	: NONE
 * Notice		: NONE
 * Global    	: None.
 * Created   	: xuwenqiang6
 ********************************************************************************/
 long long int AFVCMGetAFD(int chan);

/*********************************************************************************
* TypeName	: int SearchPeakTest(int chan)
* Describe	: 搜索清晰点一次返回坐标
* Parameters	:
* Return    	: peakpos
* Notice		: None
* Global    	: None.
* Created   	: xuwenqiang6
********************************************************************************/
int SearchPeakTest(int chan);

/*********************************************************************************
* TypeName	: int SearchPeakTestT2(int chan)
* Describe	: 校验时搜索清晰点3次求平均
* Parameters	:
* Return    	: foucspos
* Notice		: None
* Global    	: None.
* Created   	: xuwenqiang6
********************************************************************************/
int SearchPeakTestT2(int chan);

/*********************************************************************************
 * Name		: int  GetSensorType(int chan)
 * Describe		: 获得产品类型
 * Parameters	: 
 * Return 		: None
 * Notice 		: None.
 * Global 		: None.
 * Created		: xuwenqiang6
 ********************************************************************************/ 
int  GetSensorType(int chan);

/*********************************************************************************
 * Name		: void InitAFVCMSetAFWin(int chan)
 * Describe		: 初始化多窗口的值；
 * Parameters	: 
 * Return 		: None
 * Notice 		: None.
 * Global 		: None.
 * Created		: xuwenqiang6
 ********************************************************************************/ 
void  InitAFVCMSetAFWin(int chan);

int SearchPeak(int chan);
int SearchPeak0(int chan);

/**
 * 调节聚焦灵敏度等级，实时生效
 */
void setSensLevel(int level);

#endif






