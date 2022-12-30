#ifndef __JPEG_CONTROL_H
#define __JPEG_CONTROL_H

#define JPEG_QUALITY            100
#define RGBA8888                32
#define RGB888                  24

/*
 *函数名：convert_jpeg_to_nv12
 *功能： 将 JPEG 格式文件转换为  NV12 格式
 *参数：
 *	jpeg_data：JPEG 文件数据
 *	jpeg_size：jpeg 文件大小
 *	nv12_w：   nv12 宽
 *	nv12_h：   nv12 高
 *返回值：返回 nv12 数据buf
 */

unsigned char* convert_jpeg_to_nv12(unsigned char *jpeg_data, int jpeg_size, unsigned int *nv12_w, unsigned int *nv12_h);

/*
 *函数名：resize_jpeg_file
 *功能： 将 JPEG 格式文件图片进行resize
 *参数：
 *	jpeg_data：JPEG 文件数据
 *	jpeg_size：jpeg 文件大小
 *	dest_w：   目标 宽
 *	dest_h：   目标 高
 *	dest_bits：目标 位深
 *	dest_size：目标 文件大小
 *返回值：resize 数据buf 地址
 */
unsigned char *resize_jpeg_file(unsigned char *jpeg_data, int jpeg_size, int dest_w, int dest_h, int dest_bits, unsigned long *dest_size);

#endif
