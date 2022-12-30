/*********************************************************
 * File Name   : hid_test.h
 * Author      : Jasmine
 * Mail        : jian.dong@ingenic.com
 * Created Time: 2020-09-24 15:03
 ********************************************************/
#ifndef _HID_TEST_H__
#define _HID_TEST_H__

#include <stdio.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <semaphore.h>


#define HID_DEVICE_PATH                    "/dev/hidg0"
#define UCAMERA_DEVICE_PATH                "/dev/ucamera"
#define BUF_LEN                            1024
#define HID_DEBUG			0
#define HID_REPORT_ID                      0x01
#define FS_SUB_CHN                         0


/*functions*/
#define HID_COMMON_USAGE                   0x0000
#define HID_ALGORITHM_AUTHORIZE            0x1000
#define HID_SN_BURNING                     0x2000
#define HID_ONLINE_UPDATE                  0x3000

#define UCAMERA_IOCTL_MAGIC                'U'
#define UCAMERA_IOCTL_HID_EMER_ENABLE      _IO(UCAMERA_IOCTL_MAGIC, 8)

/* details cmd of every function */
/*Hid common usage (HCU)  */
#define HCU_GET_VERSION                    (HID_COMMON_USAGE + 0x01)

/*Hid Algorithm Authorize  */
#define HAA_START                      	   (HID_ALGORITHM_AUTHORIZE + 0x100)
#define HAA_DEVICE_INFO                    (HID_ALGORITHM_AUTHORIZE + 0x200)
#define HAA_WARRANT_INFO                   (HID_ALGORITHM_AUTHORIZE + 0x210)

/*Hid SN burning (HSB)  */
#define HSB_READ_MANUFACTURE               (HID_SN_BURNING + 0x000)
#define HSB_READ_SREIALNUMBER              (HID_SN_BURNING + 0x010)
#define HSB_READ_PRODUCT                   (HID_SN_BURNING + 0x020)
#define HSB_READ_VID                       (HID_SN_BURNING + 0x030)
#define HSB_READ_PID                       (HID_SN_BURNING + 0x040)
#define HSB_READ_BCD                       (HID_SN_BURNING + 0x050)

#define HSB_READ_MODEL                     (HID_SN_BURNING + 0xf00)
#define HSB_READ_CMEI                      (HID_SN_BURNING + 0xf10)

#define HSB_WRITE_MANUFACTURE             (HID_SN_BURNING + 0x001)
#define HSB_WRITE_SREIALNUMBER            (HID_SN_BURNING + 0x011)
#define HSB_WRITE_PRODUCT                 (HID_SN_BURNING + 0x021)
#define HSB_WRITE_VID                     (HID_SN_BURNING + 0x031)
#define HSB_WRITE_PID                     (HID_SN_BURNING + 0x041)
#define HSB_WRITE_BCD                     (HID_SN_BURNING + 0x051)

#define HSB_WRITE_MODEL                   (HID_SN_BURNING + 0xf01)
#define HSB_WRITE_CMEI                    (HID_SN_BURNING + 0xf11)

#define HSB_AUDIO_NOISE_OPEN                   (HID_SN_BURNING + 0xf30)
#define HSB_AUDIO_NOISE_CLOSE                  (HID_SN_BURNING + 0xf31)


#define MANUFACTURE 	                  0x200
#define SREIALNUMBER			  0x201
#define VID 			 	  0x203
#define PID 			 	  0x204
#define BCD			 	  0x205

#define	MODEL 				  0x2f0
#define	CMEI 			  	  0x2f1

/* Hid online update (HOU) */
#define HOU_START                         (HID_ONLINE_UPDATE + 0x0)
#define HOU_SEND_DATA                     (HID_ONLINE_UPDATE + 0x200)
#define HOU_GET_RESULT                    (HID_ONLINE_UPDATE + 0x300)
#define HOU_UPDATE_UVCAPP                 (HID_ONLINE_UPDATE + 0x110)
#define HOU_UPDATE_UVCAPP_ANTICOPY        (HID_ONLINE_UPDATE + 0x111)
#define HOU_UPDATE_UVCCONFIG              (HID_ONLINE_UPDATE + 0x112)
#define HOU_UPDATE_UVCATTR                (HID_ONLINE_UPDATE + 0x113)
#define HOU_UPDATE_SENSOR_DIRVER          (HID_ONLINE_UPDATE + 0x114)
#define HOU_UPDATE_SENSOR_SETTING         (HID_ONLINE_UPDATE + 0x115)
#define HOU_UPDATE_APP_INIT_SH            (HID_ONLINE_UPDATE + 0x116)

#define HOU_UPDATE_FAST_INIT_SH           (HID_ONLINE_UPDATE + 0x120)

#define HOU_UPDATE_HID_TEST               (HID_ONLINE_UPDATE + 0x1f0)

#define HOU_UPDATE_CUSTOMISE           (HID_ONLINE_UPDATE + 0x150)

/* data status */
#define HID_DATA_INVALID		  0x00
#define HID_DATA_VALID			  0x01
#define HID_DATA_END			  0x02

/* CMD status */
#define CMD_EXCUTE_OK                     0x00
#define CMD_EXCUTE_ERROR                  0x01
#define CMD_REPORTID_ERROR                0x02
#define CMD_MAGIC_ERROR                   0x03
#define CMD_CRC_ERROR                     0x04
#define CMD_UNSUPPORT_FUNCTION            0x05

#define MAGIC                             0x5a5a
#define HID_HEAD_SIZE                     sizeof(hid_cmd_head)
#define HID_RESPONSE_SIZE                 sizeof(response_cmd_head)

#define HAA_LICENSE_PATH                  "/system/config/license.txt"

#define UPDATE_BUFFER_MAX_SIZE            (1024 * 1024 * 2)
#define UPDATE_UVCCONFIG_PATH             "/media/uvc.config"
#define UPDATE_UVCCONFIG_BAK_PATH         "/system/config/uvc.config"
#define UPDATE_UVCATTR_PATH               "/media/uvc.attr"
#define KIVA_VERSION_PATH                 "/tmp/version.txt"
/* ucamera update, need anticopy */
#define UPDATE_UVCAPP_PATH                "/system/bin/ucamera"
#define UPDATE_APP_INIT_SH_PATH           "/system/init/app_init.sh"
#define UPDATE_FAST_INIT_SH_PATH          "/system/init/fast_init.sh"
#define UPDATE_HID_TEST_PATH              "/system/bin/hid_update"
#define UPDATE_HID_TEST_BAK_PATH          "/system/bin/hid_update_bak"
#define UPDATE_UVCAPP_ANTICOPY_PATH       "/system/bin/anticopy"
#define UPDATE_SENSOR_DRIVER_PATH         "/system/lib/modules/sensor_%s_t31.ko"
#define UPDATE_SENSOR_SETTINT_PATH        "/system/etc/sensor/%s-t31.bin"

typedef struct hid_cmd_head_t {
	uint8_t  report_id;			/* accord to driver */
	uint16_t magic;				/* fixed value: 0x5a5a */
	uint16_t cmd;				/*  */
	uint8_t  data_status;			/* 0: no data; 1: have data; 2:the last data packet */
	uint16_t data_len;			/* data length of the packet*/
	uint32_t pack_num;			/* if have data, the data packet number. */
	uint16_t crc;
}__attribute__((packed)) hid_cmd_head;

typedef struct response_cmd_head_t {
	uint8_t  report_id;
	uint16_t magic;
	uint16_t cmd;
	uint8_t  data_status;		/* the device may transfer data to PC: 0: have no data; 1: have data */
	uint16_t data_len;
	uint32_t cmd_status;		/* the cmd exucte result */
	uint16_t crc;
}__attribute__((packed)) response_cmd_head;

typedef struct HOU_Object_Function_t {
	char *recv_buffer;
	int buffer_size;
	int recv_off;
	int last_pack_num;
	char *file_path;
	int data_trans_complete;
} HOU_Object_Function;

typedef struct HSB_Object_Function_t {
	char *file_path;
	int hsb_size_len;
} HSB_Object_Function;

typedef struct hid_context_t {
	int hid_fd;
	char read_buf[BUF_LEN];
	uint8_t resp_buf[BUF_LEN];
	HOU_Object_Function hou_object;
} hid_context;

struct uvc_hsb_string {
	int id;
	const char *s;
};

struct algorithm_info {
	int     cid;
	int     fid;		    /*<< 功能编号FID，由君正提供，ADK2.0起该成员未使用 */
	char    sn[32];            /*<< 激活序列号SN，最大32个字节，唯一不重复,可以由君正提供,*/
};

#define HID_ERR(format,arg...)         		\
	printf("%s:%s: " format "\n", "[HID_UPDATE]", "[ERROR:]", ##arg)

#define HID_WARNING(format,arg...)         	\
	printf("%s:%s: " format "\n", "[HID_UPDATE]", "[WARNING:]", ##arg)

#define HID_LOG(format,arg...)         		\
	printf("%s: " format "\n", "[HID_UPDATE]",  ##arg)

uint16_t soft_crc16(uint8_t *buf, int32_t len, uint16_t crc);

int cmd_response(uint16_t cmd, uint32_t cmd_status, uint8_t data_status,
				 char *data_buf, uint16_t data_len);

int sn_burning_cmd(char *read_buf);
int sample_audio_ns_switch(int ns_switch);

int process_update_cmd(char *read_buf, hid_context *gcontext);

int algorithm_authorize_cmd(char *read_buf);

#endif
