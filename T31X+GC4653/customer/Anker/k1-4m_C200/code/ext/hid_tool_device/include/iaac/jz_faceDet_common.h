#ifndef __FACE_INF_FACEDET_H__
#define __FACE_INF_FACEDET_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include <stdint.h>
#include <stdbool.h>

#define NUM_OF_FACES     8
#define FD_VERSION       0x02000013
#define NUM_OF_LANDMARK  20

/*
 * 点的定义
 */
typedef struct{
    int x;    /**< 横向的坐标值 */
    int y;    /**< 纵向的坐标值 */
}FacePoint;

/*
 * 矩形框的定义
 */
typedef struct{
  FacePoint ul;    /**< 左上角点坐标 */
  FacePoint br;    /**< 右下角点坐标 */
}FaceRect;



/*
 * 矩形框的定义
 */
typedef struct{
    float x;
    float y;
    float w;
    float h;
}FaceInfo;


typedef struct{
  FaceInfo rect;
  int width;
  int height;
}DataInfo;


/*
 * 输入帧的信息
 */

typedef struct {
    unsigned char *main_data; /**< 帧的起始地址 bgra */
    DataInfo main_info;
    unsigned char *sub_data; /**< 帧的起始地址 bgra */
    DataInfo sub_info;
    unsigned char *y_data; /* Y data*/
    DataInfo y_info;
    int flag;  /*normal:0 infrared:1 */
    int64_t timeStamp; /**< 帧的时间戳 */
}FaceFrame;

typedef FaceFrame FaceFrameInfo;


/*
 * 人脸信息角度结构体
 */
typedef struct {
    float yaw;
    float pitch;
    float roll;
}face_pose;

/*
 * 人脸信息结构体
 */
typedef struct {
    FaceRect box; /**< 人脸区域坐标,用来做人脸识别 */
    FaceRect show_box; /**< 人脸区域真实坐标 */
    face_pose face_pose_res; /**< 人脸角度**/
    int   landmark[NUM_OF_LANDMARK];
    float confidence; /**< 人脸检测结果的置信度 */
    int   id;
    float blur_score;
//    float gauze_score;
}face_info;

typedef struct {
    FaceRect box;
    FaceRect show_box;
    float confidence;
}person_info;

/*
 * 人脸检测输入结构体
 */
typedef struct {
    unsigned int max_face_box; /**< 人脸检测处理过程中保留的框数量 */
    int skip_num; /**< 跳帧数目 */
    FaceFrameInfo frameInfo; /**< main 帧信息 */
}facedet_param_input_t;

/*
 * 人脸检测输出结构体
 */
typedef struct {
    int count; /**< 识别出人脸的个数 */
    face_info   face[NUM_OF_FACES]; /**< 识别出的人脸信息 */
    int64_t timeStamp; /**< 时间戳 */
}facedet_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __FACE_INF_FACEDET_H__ */
