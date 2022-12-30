#ifndef __FACE_FACEDET_H__
#define __FACE_FACEDET_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "jz_faceDet_common.h"

typedef struct {
    int index; /**< the counter for frames */
    void* jz_facedet;
    facedet_param_input_t param;	/**< 输入数据 */
}facedet_t;

int fd_check_version(unsigned int out_version, unsigned int * in_version);

/*
 * @fn facedet_t *alloc_facedet(facedet_param_input_t *param, int (*free_data)(void *data));
 *
 * 初始化fdet算法
 *
 * @param[in] param facedet 算法的输入结构体
 * @param[in] param free_data 释放由facedet_preprocess或facedet_process输入的帧
 *
 * @return facedet 算法的句柄
 *
 * @retval NULL 失败
 * @retval 非NULL 成功
 *
 * @remark 无
 *
 * @attention 无
 */
facedet_t *alloc_facedet(facedet_param_input_t *param, int (*free_data)(void *data));


/*
 * @fn facedet_t *alloc_facedet(facedet_param_input_t *param, int (*free_data)(void *data));
 *
 * 初始化fdet算法
 *
 * @param[in] param facedet 算法的输入结构体
 * @param[in] param free_data 释放由facedet_preprocess或facedet_process输入的帧
 *
 * @return facedet 算法的句柄
 *
 * @retval NULL 失败
 * @retval 非NULL 成功
 *
 * @remark 无
 *
 * @attention 无
 */
facedet_t *alloc_facedet_json(char* param, int (*free_data)(void *data));


/*
 * @fn void free_facedet(facedet_t *facedet);
 *
 * 注销facedet算法
 *
 * @param[in] param facedet 算法的句柄
 *
 * @remark 无
 *
 * @attention 无
 */

void free_facedet(facedet_t *facedet);

/*
 * @fn int facedet_preprocess(facedet_t *facedet, FaceFrame *frame);
 *
 * 预先运行facedet算法
 *
 * @param[in] param facedet算法的句柄
 * @param[in] frame 输入帧信息
 *
 * @retval -1 失败,frame已free
 * @retval 0 成功,frame未被free,仍需要继续使用
 *
 * @remark 无
 *
 * @attention 无
 */
int facedet_preprocess(facedet_t *facedet, FaceFrame *frame);

/*
 * @fn int facedet_process(facedet_t *facedet, FaceFrame *data, void *result);
 *
 * 运行facedet算法
 *
 * @param[in] param facedet算法的句柄
 * @param[in] data 输入帧信息
 * @param[out] result facedet算法处理结果
 *
 * @retval -1 失败
 * @retval 0 成功,实际检测正常返回
 *
 * @remark 必须确保每个已用完的frame处于free状态
 *
 * @attention 无
 */
int facedet_process(facedet_t *facedet, FaceFrame *frame, facedet_param_output_t *result);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __FACE_FACEDET_H__ */
