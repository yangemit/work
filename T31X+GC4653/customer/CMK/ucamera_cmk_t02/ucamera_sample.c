/*
 * sample-Encoder-jpeg.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <semaphore.h>
#include <string.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <errno.h>
#include <imp/su_base.h>
#include "imp-common.h"
#define TAG "Sample-UCamera"
#include "ucam/usbcamera.h"
#include "anticopy/AntiCopy_Verify.h"
#include <linux/types.h>
#include <linux/watchdog.h>
//#include "motortest.h"
#include "motor_app.h"
#include "conf.h"
#include "mtd_flash.h"

#define VENDOR_ID  0x0416 //0x32a8
#define PRODUCT_ID 0x5030//0xa80b
#define DEVICE_BCD 0x0104

#define LABEL_LEN 64

#define UCAMERA_IOCTL_MAGIC  			'U'
#define UCAMERA_IOCTL_VIDEO_CFG			_IO(UCAMERA_IOCTL_MAGIC, 1)
#define UCAMERA_IOCTL_DRIVER_INIT		_IO(UCAMERA_IOCTL_MAGIC, 2)
#define UCAMERA_IOCTL_DRIVER_DEINIT		_IO(UCAMERA_IOCTL_MAGIC, 3)
#define UCAMERA_IOCTL_AUDIO_ENABLE		_IO(UCAMERA_IOCTL_MAGIC, 4)
#define UCAMERA_IOCTL_AUDIO_CFG			_IO(UCAMERA_IOCTL_MAGIC, 5)
#define UCAMERA_IOCTL_ADB_ENABLE		_IO(UCAMERA_IOCTL_MAGIC, 6)
#define UCAMERA_IOCTL_PRODUCT_CFG		_IO(UCAMERA_IOCTL_MAGIC, 7)
#define UCAMERA_IOCTL_HID_ENABLE		_IO(UCAMERA_IOCTL_MAGIC, 8)
#define UCAMERA_IOCTL_STILL_IMG_CAP		_IO(UCAMERA_IOCTL_MAGIC, 9)
#define UCAMERA_IOCTL_UMS_ENABLE		_IO(UCAMERA_IOCTL_MAGIC, 10)

#define AF_ON_OFF		0
#define MOTOR_MOVE_SLEEP_TIME   (80 * 1000)
//#define MOTOR_MOVE              _IOW('M', 1, int)
static int dac_pos = 100;
static int dac_pos_old = 100;
static int focus_trigger_value = 30;
unsigned char g_firmware_update;
static char vendor_label[LABEL_LEN] = "C1E Camera";
static char product_label[LABEL_LEN] = "C1E Camera";
static char serial_label[LABEL_LEN] = "123456";
 int Ucam_Stream_On = 0;
extern struct chn_conf chn[];

FILE *uvc_attr_fd = NULL;
pthread_mutex_t uvc_attr_mutex;
sem_t ucam_ready_sem;
static int imp_inited = 0;
char  fw_version_string[20]="V8-JT31T02-LA1V010";
const char  hw_version_string[32]="CMK-C1E-OT1778-PG1-V4.0";
const char  iq_version_string[20]="2021-11-09";
//extern stepMotorCtrl gMotor;

static int first_on;
/*burn in test*/
unsigned char g_burnin_test_mode= 0;
/*uid & license*/
//char g_flag_license_actived=1;
SUDevID g_dev_uid;
//#define LIC_ADDR_OFFSET 0xA0000
#define LIC_OTP_OFFSET 0x10

#if 1
#define UVC_EU_CMD_GET_VERSION 0x11
#define UVC_EU_CMD_START_UPGRADE 0x13
#define UVC_EU_CMD_SET_BITRATE 0x14 
#define UVC_EU_CMD_SET_BITRATE_CVT 0x01 
#define UVC_EU_CMD_GET_UID 0x02 
#define UVC_EU_CMD_SET_LIC 0x03 
#define UVC_EU_CMD_GET_SLIDER_INFO 0x17
#define UVC_EU_CMD_SET_SLIDER   0x1C

static unsigned int do_license_active(unsigned char *buf, unsigned int bufLength)
{
	return 16;
}

#define SET_MOTOR_CLOSE_SLIDER  0X02
#define SET_MOTOR_OPEN_SLIDER  0X01
static void do_slider_action(unsigned char action)
{
    if((action!=SET_MOTOR_OPEN_SLIDER)&&(action!=SET_MOTOR_CLOSE_SLIDER))
    	{
           printf("do slider action err:wrong para %x",action);
			return;
		}	
	while(motor_get_status()!=eMOTOR_ACTION_IDLE)
	{
	      usleep(5*1000);
	}
   if(action==SET_MOTOR_OPEN_SLIDER)
   	{
   		sample_ucamera_led_ctl(g_led, 0);
	 	motor_open_door();
   	}
   if(action==SET_MOTOR_CLOSE_SLIDER)
   	{
   		sample_ucamera_led_ctl(g_led, 1);
	 	motor_close_door();
   	}
}

#define MOTOR_STATUS_RUNING 0X02
#define MOTOR_STATUS_OPENED 0X01
#define MOTOR_STATUS_CLOSED 0X03

static unsigned char xu_get_slider_status(void)
{
     eMOTOR_ACTION action;
	 eMOTOR_POS pos;
	 action=motor_get_status();
     if(action!=eMOTOR_ACTION_IDLE)
		 	return MOTOR_STATUS_RUNING;
	pos=motor_get_pos();
	if(pos==eMOTOR_POS_LEFT)
		   return MOTOR_STATUS_OPENED;
	else
			return MOTOR_STATUS_CLOSED;

}
//xu命令数据长度GET_LEN信息，可以根据不同xu命令返回不同的长度
int xu_req_set_GetLen(unsigned char cs, void *res)
{
	int i;
	//printf("xu cs=%x\n",cs);
	unsigned char *pOutData=(unsigned char *)res;
	switch(cs)
		{
      case UVC_EU_CMD_GET_VERSION://do get version
			*pOutData++=23; //buffer length 24 byte
			*pOutData=0;
			break;
	case UVC_EU_CMD_SET_BITRATE_CVT:
		*pOutData++=4; //buffer length 4 byte
		*pOutData=0;
		break;
	case UVC_EU_CMD_GET_SLIDER_INFO:
				*pOutData++=5; //buffer length 2 byte
		*pOutData=0;
		break;
	case UVC_EU_CMD_SET_SLIDER:
				*pOutData++=5; //buffer length 2 byte
		*pOutData=0;
		break;	
	case UVC_EU_CMD_START_UPGRADE:
		*pOutData++=2; //buffer length 2 byte
		*pOutData=0;
		break;	
		default:
			*pOutData++=24; //buffer length 24 byte
			*pOutData=0;
			break;
	}
	return 0;
}
int xu_req_process_usr(unsigned char cs, void *data, int slen, void *res, int *rlen)
{
	int i;
//	printf("xu cs=%x,data len=%d,data:\n",cs,slen);
	unsigned char *pInData=(unsigned char*)data;
	unsigned char *pOutData=(unsigned char *)res;
	int bitrate=0;
	/*
	for(i=0;i<slen;i++)
	{
         printf("%d,",pInData[i]);
	}
    printf("\n");
    */
	switch(cs)
		{
		case UVC_EU_CMD_SET_BITRATE_CVT:
				bitrate=(pInData[3]<<24)|(pInData[2]<<16)|(pInData[1]<<8)|(pInData[0]<<0);
				printf("set bitrate=%d\n",bitrate);
			//	bitrate=bitrate/1000;

				if(IMP_Encoder_SetChnBitRate(0,bitrate,bitrate)==0)
				{
					 printf("set bitrate success\n");
				}	
			break;
      case UVC_EU_CMD_GET_VERSION://do get version
				printf("do get version\n");
				*pOutData++=0x03;			//type
				*pOutData++=0x00; //length
				*pOutData++=20;
				memcpy(pOutData, fw_version_string, sizeof(fw_version_string));
				*rlen=23;
				break;
	 case UVC_EU_CMD_GET_SLIDER_INFO:
	 			//printf("do get slider info\n");
				*pOutData++=0x01;			//type :motor
				*pOutData++=0x00; //length
				*pOutData++=0x02;
				*pOutData++=3; //滑盖时3s
				*pOutData=xu_get_slider_status(); //滑盖状态
				*rlen=5;
	 	        break;
		 case UVC_EU_CMD_SET_SLIDER:
	 			//printf("do Set slider info\n");
				if(pInData[0]!=0x01)//type :01 motor
				     break;
				do_slider_action(pInData[3]);
				
	 	break;
	 case UVC_EU_CMD_GET_UID://do get UID
				printf("do get uid( length=%d):%s\n", strlen(g_dev_uid.chr),g_dev_uid.chr);
				memcpy(pOutData, g_dev_uid.chr, strlen(g_dev_uid.chr)+1);
				*rlen=strlen(g_dev_uid.chr)+1;
				break;
	case UVC_EU_CMD_SET_LIC:
		printf("do active license\n");
		do_license_active(pInData,16);	
		break;
    case UVC_EU_CMD_START_UPGRADE://switch to upgrade
				printf("start upgrade\n");
				write_conf_value("enable","1","/system/config/update.config");
				//reboot
				run_cmd("sync");

				run_cmd("reboot");
			//	run_cmd("echo "b" > /proc/sysrq-trigger");
				break;
	case UVC_EU_CMD_SET_BITRATE:			
				if(pInData[0]==0xff)
				{
                     bitrate=(pInData[3]<<24)|(pInData[4]<<16)|(pInData[5]<<8)|(pInData[6]<<0);
                     printf("set bitrate=%d\n",bitrate);
					//bitrate=bitrate/1000;
					 
					  if(IMP_Encoder_SetChnBitRate(0,bitrate,bitrate)==0)
					  	{
                               printf("set bitrate success\n");
						}
						else{

						}
				}
				break;
		default:
			break;
	}
	return 0;
}
#endif


struct uvc_pu_string {
	char id;
	const char *s;
};

struct uvc_pu_string pu_string[] = {
	{0, "null"},
	{UVC_BACKLIGHT_COMPENSATION_CONTROL, "backlight"},
	{UVC_BRIGHTNESS_CONTROL, "brightness"},
	{UVC_CONTRAST_CONTROL, "contrast"},
	{UVC_GAIN_CONTROL, "gain"},
	{UVC_POWER_LINE_FREQUENCY_CONTROL, "powerline_freq"},
	{UVC_HUE_CONTROL, "hue"},
	{UVC_SATURATION_CONTROL, "saturation"},
	{UVC_SHARPNESS_CONTROL, "sharpness"},
	{UVC_GAMMA_CONTROL, "gamma"},
	{UVC_WHITE_BALANCE_TEMPERATURE_CONTROL, "white_balance"},
	{UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL, "white_balance_auto"}
};

static struct Ucamera_Video_CT_Control auto_exposure_mode = {
	.type = UVC_AUTO_EXPOSURE_MODE_CONTROL,
	.data[UVC_MIN] = 1,
	.data[UVC_MAX] = 4,
	.data[UVC_DEF] = 2,
	.data[UVC_CUR] = 2,
};

static struct Ucamera_Video_CT_Control exposure_time = {
	.type = UVC_EXPOSURE_TIME_CONTROL,
	.data[UVC_MIN] = 1,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_CT_Control focus = {
	.type = UVC_FOCUS_CONTROL,
	.data[UVC_MIN] = 100,
	.data[UVC_MAX] = 500,
	.data[UVC_DEF] = 100,
	.data[UVC_CUR] = 100,
};

static struct Ucamera_Video_CT_Control focus_auto = {
    .type = UVC_FOCUS_AUTO_CONTROL,
    .data[UVC_MAX] = 1,
    .data[UVC_MIN] = 0,
    .data[UVC_DEF] = 1,
    .data[UVC_CUR] = 1,
};

static struct Ucamera_Video_PU_Control backlight_compens = {
	.type = UVC_BACKLIGHT_COMPENSATION_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 4,
	.data[UVC_DEF] = 0,
	.data[UVC_CUR] = 0,
};

static struct Ucamera_Video_CT_Control *Ct_ctrl[] = {
    &exposure_time,
    &auto_exposure_mode,
    &focus,
    &focus_auto,
    NULL,
};

static struct Ucamera_Video_PU_Control brightness = {
	.type = UVC_BRIGHTNESS_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_PU_Control contrast = {
	.type = UVC_CONTRAST_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_PU_Control saturation = {
	.type = UVC_SATURATION_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_PU_Control sharpness = {
	.type = UVC_SHARPNESS_CONTROL,
	.data[UVC_MIN] = 1,
	.data[UVC_MAX] = 200,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_PU_Control hue = {
	.type = UVC_HUE_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_PU_Control gamma = {
	.type = UVC_GAMMA_CONTROL,
	.data[UVC_MIN] = 1,
	.data[UVC_MAX] = 255,
	.data[UVC_DEF] = 128,
	.data[UVC_CUR] = 128,
};

static struct Ucamera_Video_PU_Control whitebalance = {
	.type = UVC_WHITE_BALANCE_TEMPERATURE_CONTROL,
	.data[UVC_MIN] = 300,
	.data[UVC_MAX] = 600,
	.data[UVC_DEF] = 417,
};

static struct Ucamera_Video_PU_Control powerlinefreq = {
	.type = UVC_POWER_LINE_FREQUENCY_CONTROL,
	.data[UVC_MIN] = 0,
	.data[UVC_MAX] = 2,
	.data[UVC_DEF] = 0,

};

static struct Ucamera_Video_PU_Control *Pu_ctrl[] = {
	&backlight_compens,
	&brightness,
	&contrast,
	&hue,
	&saturation,
	&sharpness,
	&gamma,
	&whitebalance,
	&powerlinefreq,
	NULL,
};

static int strToInt(char const* str) {
	int val;
	if (sscanf(str, "%d", &val) == 1) return val;
	return -1;
}

int htoi(char s[])
{
	int n = 0;
	int i = 0;
	while (s[i] != '\0' && s[i] != '\n') {
		if (s[i] == '0') {
			if (s[i+1] == 'x' || s[i+1] == 'X')
				i+=2;
		}
		if (s[i] >= '0' && s[i] <= '9') {
			n = n * 16 + (s[i] - '0');
		} else if (s[i] >= 'a' && s[i] <= 'f') {
			n = n * 16 + (s[i] - 'a') + 10;
		} else if (s[i] >= 'A' && s[i] <= 'F') {
			n = n * 16 + (s[i] - 'A') + 10;
		} else
			return -1;
		++i;
	}

	return n;
}
int sample_video_focus_set(int value)
{
    focus.data[UVC_CUR] = value;
    return 0;
}

int sample_video_focus_get(int value)
{
    return focus.data[UVC_CUR];
}

int sample_video_focus_auto_set(int value)
{
    focus_auto.data[UVC_CUR] = value;
    return 0;
}

int sample_video_focus_auto_get(int value)
{
    return focus_auto.data[UVC_CUR];
}

int sample_video_exposure_time_set(int value)
{
    int ret;
    ret = IMP_ISP_Tuning_SetAeComp(value);
    if(ret)
        Ucamera_LOG("ERROR:set exposure time Invalid leave:%d\n",value);

    exposure_time.data[UVC_CUR] = value;
    return 0;
}

int sample_video_exposure_time_get(void)
{
    int ret,value = 0;
    if(!Ucam_Stream_On)
        return exposure_time.data[UVC_CUR];

    ret = IMP_ISP_Tuning_GetAeComp(&value);
    if(ret)
        Ucamera_LOG("ERROR:get exposure time Invalid leave:%d\n",value);
    return value;
}

int sample_video_auto_exposure_mode_set(int value)
{
	auto_exposure_mode.data[UVC_CUR] = value;
	return 0;
}

int sample_video_auto_exposure_mode_get(void)
{
	return auto_exposure_mode.data[UVC_CUR];
}


int sample_video_backlight_compens_set(int value)
{
	int ret;
	if (value < 0 || value > 10) {
		Ucamera_LOG("set BackLight Invalid leave:%d\n", value);
		return 0;
	}
	ret = IMP_ISP_Tuning_SetHiLightDepress(value);
	if (ret)
		Ucamera_LOG("ERROR: set BackLight Invalid leave:%d\n", value);
	backlight_compens.data[UVC_CUR] = value;
	ucamera_uvc_pu_attr_save(UVC_BACKLIGHT_COMPENSATION_CONTROL, value);
	return 0;
}

int sample_video_backlight_compens_get(void)
{
	int ret, value = 0;

	if (!Ucam_Stream_On)
		return backlight_compens.data[UVC_CUR];

	ret = IMP_ISP_Tuning_GetHiLightDepress(&value);
	if (ret)
		Ucamera_LOG("ERROR: get BackLight error:%d\n", ret);
	return value;
}

int run_cmd(char *cmd)
{   
    FILE *fp=NULL;   
    fp = popen(cmd, "r");   
    char ps_buf[128]={0};
    if (fp)  
    {      
        while(fgets(ps_buf, 128, fp) != NULL)   
        { 
         printf("%s\n",ps_buf);
        }       
        pclose(fp);  
    }   
    return 0;
}
static char doactive_entry_step=0;
static char doactive_processing=0;
static char license_data_cnt=0;
static char license_data[16];
static char uid_ret_cnt=0;
static char ver_ret_cnt=0;


int sample_video_brightness_set(int value)
{
	int ret;
	unsigned char bright = 0;

	bright = value & 0xff;

	if(doactive_processing==0)
	{
				//printf("sample_video_brightness_set=%d\n",bright);
				printf("doactive_entry_step=%d\n",doactive_entry_step);
				if(bright==brightness.data[UVC_MIN])
				{
					doactive_entry_step=1;
				}
				if(bright==brightness.data[UVC_MAX])
				{
			            if(doactive_entry_step==1)
			            	{
								 doactive_entry_step++;
			            	}
							else{
								doactive_entry_step=0;
							}
				}
				if(bright==brightness.data[UVC_DEF])
				{
			            if(doactive_entry_step==2)
			            	{
								 doactive_entry_step++;
			            	}
							else{
								doactive_entry_step=0;
							}
				}
				if(bright==0x5A)
					{
							if(doactive_entry_step==3)
							{
								doactive_entry_step++;
							}
							else{
								doactive_entry_step=0;
							}
				}
				if(bright==0xA5)
				{
							if(doactive_entry_step==4)
							{
									doactive_entry_step++;
									doactive_processing=1;
									license_data_cnt=0;
									uid_ret_cnt=0;
									memset(license_data,0,16);
									printf("start do active\n");
							}
							else{
									doactive_entry_step=0;
							}
					}
	}

#if 0
	ret = IMP_ISP_Tuning_SetBrightness(bright);
	if (ret)
		Ucamera_LOG("ERROR: set BrightNess failed :%d\n", ret);
	brightness.data[UVC_CUR] = bright;
	ucamera_uvc_pu_attr_save(UVC_BRIGHTNESS_CONTROL, bright);
#endif
	return 0;
}

int sample_video_brightness_get(void)
{
	int ret;
	unsigned char bright = 0;	
	if(doactive_processing==0){

	if (!Ucam_Stream_On)
				return brightness.data[UVC_CUR];

			ret = IMP_ISP_Tuning_GetBrightness(&bright);
			if (ret) {
				Ucamera_LOG("get BrightNess failed:%d\n", ret);
				bright = 128;
			}	
			return bright;
		}else{
//return activemod flag
        return 1;
		}

}

int sample_video_contrast_set(int value)
{
	int ret;
	unsigned char tmp = 0;

	tmp = value & 0xff;
	#if 0
	ret = IMP_ISP_Tuning_SetContrast(tmp);
	if (ret)
		Ucamera_LOG("set Contrast failed:%d\n", ret);

	contrast.data[UVC_CUR] = tmp;
	ucamera_uvc_pu_attr_save(UVC_CONTRAST_CONTROL, tmp);
	#endif
   if(doactive_processing==1)
	{
			license_data[license_data_cnt]=value&0xff;
			//printf("recevie license[%d]=%x\n",license_data_cnt,	license_data[license_data_cnt]);
			license_data_cnt++;

			if(license_data_cnt==16)
			{
                do_license_active(license_data,16);
				 printf("active success\n");
				 doactive_processing=0;
			}
	}
	return 0;
}

int sample_video_contrast_get(void)
{
	int ret;
	unsigned char cost = 0;
 if(doactive_processing==0)
   	{
			if (!Ucam_Stream_On)
			return contrast.data[UVC_CUR];

			ret = IMP_ISP_Tuning_GetContrast(&cost);
			if (ret) {
			Ucamera_LOG("get Contrast failed:%d\n", ret);
				cost = 128;
			}
   	}
	 else  //return uid
	 	{
			cost= g_dev_uid.chr[uid_ret_cnt];
			printf("uid[%d]=%x\n",uid_ret_cnt,cost);
			uid_ret_cnt++;
			if(uid_ret_cnt>=24)
				uid_ret_cnt=0;
			return cost;
	 }
	
	return cost;
}

int sample_video_saturation_set(int value)
{

	int ret;
	unsigned char tmp = 0;
	tmp = value & 0xff;
#if 0
	ret = IMP_ISP_Tuning_SetSaturation(tmp);
	if (ret)
		Ucamera_LOG("set Saturation failed:%d\n", ret);
	saturation.data[UVC_CUR] = tmp;
	ucamera_uvc_pu_attr_save(UVC_SATURATION_CONTROL, tmp);
	#endif
	return 0;
}

int sample_video_saturation_get(void)
{
	int ret;
	unsigned char tmp = 0;
   if(doactive_processing==0)
   	{
		if (!Ucam_Stream_On)
			return saturation.data[UVC_CUR];

		ret = IMP_ISP_Tuning_GetSaturation(&tmp);
		if (ret) {
			Ucamera_LOG("get Saturation failed:%d\n", ret);
			tmp = 128;

		}
   	}
	 else  //fw versiton
	 	{
	 	    if(ver_ret_cnt==0)
	 	    {
					tmp=strlen(fw_version_string);
	 	    }
		   else
			{
			        tmp= fw_version_string[ver_ret_cnt-1];
			}
			printf("version[%d]=%x\n",ver_ret_cnt,tmp);
			ver_ret_cnt++;
			if(ver_ret_cnt>=(strlen(fw_version_string)+1))
				  ver_ret_cnt=0;
	 }		
	 return tmp;

}

int sample_video_sharpness_set(int value)
{
	int ret;
	unsigned char tmp = 0;

	tmp = value & 0xff;
	#if 0
	ret = IMP_ISP_Tuning_SetSharpness(tmp);
	if (ret)
		Ucamera_LOG("set Sharpness failed:%d\n", ret);
	sharpness.data[UVC_CUR] = tmp;
	ucamera_uvc_pu_attr_save(UVC_SHARPNESS_CONTROL, tmp);
	#endif
	return 0;
}

int sample_video_sharpness_get(void)
{
	int ret;
	unsigned char tmp = 0;

	if (!Ucam_Stream_On)
		return sharpness.data[UVC_CUR];

	ret = IMP_ISP_Tuning_GetSharpness(&tmp);
	if (ret) {
		Ucamera_LOG("get Sharpness failed:%d\n", ret);
		tmp = 128;
	}
	return tmp;
}

int sample_video_hue_set(int value)
{
	int ret;
	unsigned char hue_value = value & 0xff;
	ret = IMP_ISP_Tuning_SetBcshHue(hue_value);
	if (ret)
		Ucamera_LOG("set Hue failed:%d\n", ret);
	hue.data[UVC_CUR] = hue_value;
	ucamera_uvc_pu_attr_save(UVC_HUE_CONTROL, hue_value);
	return 0;
}

int sample_video_hue_get(void)
{
	int ret;
	unsigned char hue_value = 0;

	if (!Ucam_Stream_On)
		return hue.data[UVC_CUR];
	ret = IMP_ISP_Tuning_GetBcshHue(&hue_value);
	if (ret) {
		Ucamera_LOG("set Hue failed:%d\n", ret);
		hue_value = 128;
	}
	return hue_value;
}

int sample_video_t_compens_set(int value)
{
	int ret;
	if (value < 0 || value > 10) {
		Ucamera_LOG("set BackLight Invalid leave:%d\n", value);
		return 0;
	}
	ret = IMP_ISP_Tuning_SetHiLightDepress(value);
	if (ret)
		Ucamera_LOG("ERROR: set BackLight Invalid leave:%d\n", value);
	backlight_compens.data[UVC_CUR] = value;
	ucamera_uvc_pu_attr_save(UVC_BACKLIGHT_COMPENSATION_CONTROL, value);
	return 0;
}

int sample_video_gamma_set(int value)
{
	int ret;
	unsigned char tmp = 0;

	tmp = value & 0xff;
	ret = IMP_ISP_Tuning_SetContrast(tmp);
	if (ret)
		Ucamera_LOG("set Contrast failed:%d\n", ret);

	gamma.data[UVC_CUR] = tmp;
	ucamera_uvc_pu_attr_save(UVC_GAMMA_CONTROL, tmp);
	return 0;
}

int sample_video_gamma_get(void)
{
	int ret;
	unsigned char cost = 0;

	if (!Ucam_Stream_On)
		return gamma.data[UVC_CUR];

	ret = IMP_ISP_Tuning_GetContrast(&cost);
	if (ret) {
		Ucamera_LOG("get Contrast failed:%d\n", ret);
		cost = 128;
	}
	return cost;
}
int load_umass_driver(void)
{
        int fd;
        int ret;
	if((fd=open("/dev/ucamera",O_RDWR))==-1)
	{
		printf("open input file error!\n");
		return -1;
	}

	printf("open /dev/ucamera successfully\n");

	ret = ioctl(fd, UCAMERA_IOCTL_UMS_ENABLE, 0);

	if (ret == -1) {

close(fd);
		printf("ioctl error\n");

		return -1;
	}
	close(fd);

}
#if 0
int sample_video_gamma_set(int value)
{
	int i;
	IMPISPGamma gamma = {0};

	IMP_ISP_Tuning_GetGamma(&gamma);
	gamma.gamma[0] = value & 0xffff;
	IMP_ISP_Tuning_SetGamma(&gamma);
	Ucamera_LOG("Sample set Gamma value:\n");
	for (i = 0; i < 129; i++)
		Ucamera_LOG("%d ", gamma.gamma[i]);
	Ucamera_LOG("\n");
	return 0;
}

int sample_video_gamma_get(void)
{
	int i;
	IMPISPGamma gamma = {0};

	IMP_ISP_Tuning_GetGamma(&gamma);
	Ucamera_LOG("Sample get Gamma value:\n");
	for (i = 0; i < 129; i++)
		Ucamera_LOG("%d ", gamma.gamma[i]);
	Ucamera_LOG("\n");
	return gamma.gamma[0];
	return 0;
}
#endif

/*
* value楂?6浣嶄唬琛ㄦā寮忥紙0:鑷姩/ 1:鎵嬪姩
* value 搴?6浣嶄负鐧藉钩琛″疄闄呭�?*/
int sample_video_whitebalance_set(int value)
{
	int ret;
	unsigned short gain, mode = 0;
	IMPISPWB wb = {0};

	mode = value >> 16;
	gain = value & 0xffff;

	if (mode == ISP_CORE_WB_MODE_AUTO)
		wb.mode = mode;
	else{
		wb.mode = ISP_CORE_WB_MODE_MANUAL;
		wb.bgain = gain;
		wb.rgain = 160000/gain;
	}
	ret = IMP_ISP_Tuning_SetWB(&wb);
	if (ret)
		Ucamera_LOG("set WhiteBalance failed:%d\n", ret);

	whitebalance.data[UVC_CUR] = wb.bgain;
	ucamera_uvc_pu_attr_save(UVC_WHITE_BALANCE_TEMPERATURE_CONTROL, wb.bgain);
	return 0;
}

int sample_video_whitebalance_get(void)
{
	int ret, tmp = 0;
	IMPISPWB wb = {0};

	ret = IMP_ISP_Tuning_GetWB(&wb);
	if (ret)
		Ucamera_LOG("get WhiteBalance failed:%d\n", ret);
	if (wb.mode == ISP_CORE_WB_MODE_AUTO)
		wb.bgain = 417;
	tmp = (wb.mode << 16) | wb.bgain;
	return tmp;
}

int sample_video_powerlinefreq_set(int value)
{
	int ret, fps_num;
	IMPISPAntiflickerAttr attr;

	attr = value;
	if (attr < IMPISP_ANTIFLICKER_DISABLE || attr >= IMPISP_ANTIFLICKER_BUTT) {
		Ucamera_LOG("[ERROR]Sample set PowerLine Freq Invalid level:%d\n", value);
		return 0;
	}
	Ucamera_LOG("set PowerLine Freq :%d\n", value);

	ret = IMP_ISP_Tuning_SetAntiFlickerAttr(attr);
	if (ret)
		Ucamera_LOG("set PowerLine Freq failed:%d\n", ret);


//	attr = IMPISP_ANTIFLICKER_60HZ;
	
	if (attr == IMPISP_ANTIFLICKER_50HZ)
		fps_num = SENSOR_FRAME_RATE_NUM_25;
	else if(attr == IMPISP_ANTIFLICKER_60HZ)
		fps_num = SENSOR_FRAME_RATE_NUM_30;
	else
		fps_num = g_Fps_Num;

	ret = IMP_ISP_Tuning_SetSensorFPS(fps_num, 1);
	if (ret < 0) {
		Ucamera_LOG("failed to set sensor fps\n");
		return -1;
	}

	powerlinefreq.data[UVC_CUR] = attr;
	ucamera_uvc_pu_attr_save(UVC_POWER_LINE_FREQUENCY_CONTROL, attr);
	return 0;
}

int sample_video_powerlinefreq_get(void)
{
	int ret;
	IMPISPAntiflickerAttr attr;

	if (!Ucam_Stream_On)
		return powerlinefreq.data[UVC_CUR];

	ret = IMP_ISP_Tuning_GetAntiFlickerAttr(&attr);
	if (ret)
		Ucamera_LOG("get PowerLine Freq faild:%d\n", ret);
	return attr;
}


int uvc_pu_attr_setcur(int type, int value)
{
	int ret = 0;
	struct Ucamera_Video_PU_Control *pu_attr = NULL;

	switch (type) {
	case UVC_BACKLIGHT_COMPENSATION_CONTROL:
		pu_attr = &backlight_compens;
		break;
	case UVC_BRIGHTNESS_CONTROL:
		pu_attr = &brightness;
		break;
	case UVC_CONTRAST_CONTROL:
		pu_attr = &contrast;
		break;
	case UVC_GAIN_CONTROL:
		break;
	case UVC_POWER_LINE_FREQUENCY_CONTROL:
		pu_attr = &powerlinefreq;
		break;
	case UVC_HUE_CONTROL:
		pu_attr = &hue;
		break;
	case UVC_SATURATION_CONTROL:
		pu_attr = &saturation;
		break;
	case UVC_SHARPNESS_CONTROL:
		pu_attr = &sharpness;
		break;
	case UVC_GAMMA_CONTROL:
		pu_attr = &gamma;
		break;
	case UVC_WHITE_BALANCE_TEMPERATURE_CONTROL:
		pu_attr = &whitebalance;
		break;
	case UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
		break;
	default:
		Ucamera_LOG("Unkown uvc pu type:%d\n", type);
		ret = -1;
		break;
	}
	if (pu_attr)
		pu_attr->data[UVC_CUR] = value;

	return ret;
}


char *uvc_pu_type_to_string(int type)
{
	return pu_string[type].s;
}

int uvc_pu_string_to_type(char *string)
{
	int i, index;
	index = sizeof(pu_string)/sizeof(struct uvc_pu_string);

	for (i = 0; i < index; i++) {
		if (strcmp(string, pu_string[i].s) == 0)
			return pu_string[i].id;
	}
	return 0;
}

int ucamera_uvc_pu_attr_load(void)
{
	char key[64] = {0};
	char value[16] = {0};
	char line_str[128] = {0};
	int type;


	if ((uvc_attr_fd = fopen("/system/config/uvc.attr", "r+")) == NULL) {
		Ucamera_LOG("%s open config file failed!\n", __func__);
		return -1;
	}
	if (pthread_mutex_init(&uvc_attr_mutex, NULL) != 0){
		Ucamera_LOG("%s init mutex failed\n", __func__);
		return -1;
	}
	while (!feof(uvc_attr_fd)) {
		if (fscanf(uvc_attr_fd, "%[^\n]", line_str) < 0)
			break;

		if (sscanf(line_str, "%[^:]:%[^\n]", key, value) != 2) {
			Ucamera_LOG("warning: skip config %s\n", line_str);
			fseek(uvc_attr_fd , 1L, SEEK_CUR);
			continue;
		}
		/* Ucamera_LOG("%s : %d\n", key, strToInt(value)); */
		if ((type = uvc_pu_string_to_type(key))) {
			uvc_pu_attr_setcur(type, strToInt(value));
		}
		fseek(uvc_attr_fd , 1L, SEEK_CUR);
	}

	return 0;
}

int ucamera_uvc_pu_attr_save(int type, int value)
{
	char key[64] = {0};
	char data[16] = {0};
	char line_str[128] = {0};
	char *attr_string = NULL;


	if (uvc_attr_fd == NULL) {
		Ucamera_LOG("%s can not open uvc config file!\n", __func__);
		return -1;
	}
	attr_string = uvc_pu_type_to_string(type);
	if (attr_string == NULL)
		return -1;

	pthread_mutex_lock(&uvc_attr_mutex);
	fseek(uvc_attr_fd, 0L, SEEK_SET);

	while (!feof(uvc_attr_fd)) {
		if (fscanf(uvc_attr_fd, "%[^\n]", line_str) < 0)
			break;

		if (sscanf(line_str, "%[^:]:%[^\n]", key, data) != 2) {
			Ucamera_LOG("warning: Invalid param:%s\n", line_str);
			fseek(uvc_attr_fd , 1L, SEEK_CUR);
			continue;
		}
		if (strcmp(key, attr_string) == 0) {
			fseek(uvc_attr_fd, -strlen(line_str), SEEK_CUR);
			fprintf(uvc_attr_fd, "%s:%04d", key, value);
		}
		fseek(uvc_attr_fd , 1L, SEEK_CUR);
	}

	pthread_mutex_unlock(&uvc_attr_mutex);
	return 0;
}

int imp_system_init(void)
{
	int ret;
	/* Step.1 System init */
	ret = sample_system_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
		return -1;
	}
	return 0;
}

int imp_isp_tuning_init(void)
{
	int ret, fps_num;

	/* enable turning, to debug graphics */
	ret = IMP_ISP_EnableTuning();
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
		return -1;
	}

	IMP_ISP_Tuning_SetContrast(contrast.data[UVC_CUR]);
	IMP_ISP_Tuning_SetSharpness(sharpness.data[UVC_CUR]);
	IMP_ISP_Tuning_SetSaturation(saturation.data[UVC_CUR]);
	IMP_ISP_Tuning_SetBrightness(brightness.data[UVC_CUR]);
    IMP_ISP_Tuning_SetBcshHue(hue.data[UVC_CUR]);
	IMP_ISP_Tuning_SetHiLightDepress(backlight_compens.data[UVC_CUR]);
	IMP_ISP_Tuning_SetAeComp(exposure_time.data[UVC_CUR]);

	IMPISPAntiflickerAttr attr = powerlinefreq.data[UVC_CUR];
	//IMPISPAntiflickerAttr attr=IMPISP_ANTIFLICKER_DISABLE;
	if (attr < IMPISP_ANTIFLICKER_DISABLE || attr >= IMPISP_ANTIFLICKER_BUTT) {
		Ucamera_LOG("%s unvaild Antiflicker param:%d\n", __func__, attr);
		attr = IMPISP_ANTIFLICKER_DISABLE;
	}
	IMP_ISP_Tuning_SetAntiFlickerAttr(attr);

	if (attr == IMPISP_ANTIFLICKER_50HZ)
		fps_num = SENSOR_FRAME_RATE_NUM_25;
	else if(attr == IMPISP_ANTIFLICKER_60HZ)
		fps_num = SENSOR_FRAME_RATE_NUM_30;
	else
		fps_num = g_Fps_Num;

	ret = IMP_ISP_Tuning_SetSensorFPS(fps_num, 1);
	if (ret < 0) {
		Ucamera_LOG("failed to set sensor fps\n");
		return -1;
	}
	Ucamera_LOG("set Antiflicker level:%d fps:%d\n", attr, fps_num);
	ret = IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
	if (ret < 0){
		IMP_LOG_ERR(TAG, "failed to set running mode\n");
		return -1;
	}

	if ((g_HV_Flip > IMPISP_FLIP_NORMAL_MODE) && (g_HV_Flip < IMPISP_FLIP_MODE_BUTT)) {
		IMPISPHVFLIP testhv = g_HV_Flip;
		IMP_ISP_Tuning_SetHVFLIP(testhv);
		if (ret < 0){
			IMP_LOG_ERR(TAG, "failed to set HV Filp mode\n");
			return -1;
		}
		IMP_LOG_ERR(TAG, "Set HV filp mode:%d\n", g_HV_Flip);

		usleep(100*1000);
	}

	return 0;

}

int imp_sdk_init(int format, int width, int height)
{
	int i, ret, tmp = 0;
	IMPFSChnAttr *imp_chn_attr_tmp;
		struct timespec ts;
	//	clock_gettime(CLOCK_MONOTONIC, &ts);
		//Ucamera_LOG("[%d.%d]++imp_sdk_init++++\n",ts.tv_sec,ts.tv_nsec);

	imp_chn_attr_tmp = &chn[0].fs_chn_attr;
	imp_chn_attr_tmp->outFrmRateNum = SENSOR_FRAME_RATE_NUM_25;
	imp_chn_attr_tmp->picWidth = width;
	imp_chn_attr_tmp->picHeight = height;
	imp_chn_attr_tmp->scaler.outwidth = width;
	imp_chn_attr_tmp->scaler.outheight = height;

	if (width == g_VideoWidth && height == g_VideoHeight) {
		imp_chn_attr_tmp->scaler.enable = 0;
	} else
		imp_chn_attr_tmp->scaler.enable = 1;
	if (width > 2560 && height > 1440)
		imp_chn_attr_tmp->nrVBs = 1;
	else
		imp_chn_attr_tmp->nrVBs = 2;

	imp_chn_attr_tmp->crop.enable = 0;

	if ((g_VideoWidth*height) != (g_VideoHeight*width)) {
		imp_chn_attr_tmp->crop.enable = 1;
		tmp =  g_VideoWidth*height/g_VideoHeight;
		if (tmp > width) {
			if (tmp%16)
				tmp -= tmp%16;
			imp_chn_attr_tmp->scaler.outwidth = tmp;
			imp_chn_attr_tmp->scaler.outheight = height;
			imp_chn_attr_tmp->crop.left = (tmp - width)/2;
			imp_chn_attr_tmp->crop.top = 0;
			imp_chn_attr_tmp->crop.width = width;
			imp_chn_attr_tmp->crop.height = height;
		} else {
			tmp = g_VideoHeight*width/g_VideoWidth;
			imp_chn_attr_tmp->scaler.outwidth = width;
			imp_chn_attr_tmp->scaler.outheight = tmp;
			imp_chn_attr_tmp->crop.left = 0;
			imp_chn_attr_tmp->crop.top = (tmp-height)/2;
			imp_chn_attr_tmp->crop.width = width;
			imp_chn_attr_tmp->crop.height = height;
		}
		Ucamera_LOG("IMP: Crop enable w:%d h:%d left:%d top:%d\n",
							imp_chn_attr_tmp->crop.width,
							imp_chn_attr_tmp->crop.height,
							imp_chn_attr_tmp->crop.left,
							imp_chn_attr_tmp->crop.top);
	}
	Ucamera_LOG("IMP: Scaler enable w:%d h:%d\n",
						imp_chn_attr_tmp->scaler.outwidth,
						imp_chn_attr_tmp->scaler.outheight);

	if (!first_on) {
		ret = IMP_ISP_EnableSensor();
		if(ret < 0){
			IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
			return -1;
		}

		imp_isp_tuning_init();
	}

//	IMP_ISP_Tuning_SetAntiFlickerAttr(IMPISP_ANTIFLICKER_50HZ);

	/* Step.2 FrameSource init */
	//	clock_gettime(CLOCK_MONOTONIC, &ts);
	//	Ucamera_LOG("[%d.%d]++sample_framesource_init++++\n",ts.tv_sec,ts.tv_nsec);
	ret = sample_framesource_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

	if (format == V4L2_PIX_FMT_YUYV || format == V4L2_PIX_FMT_NV12) {
		IMPFSChnAttr fs_chn_attr[2];
		/* Step.3 Snap raw config */
		ret = IMP_FrameSource_GetChnAttr(0, &fs_chn_attr[0]);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_GetChnAttr failed\n", __func__, __LINE__);
			return -1;
		}

		fs_chn_attr[0].pixFmt =PIX_FMT_NV12;//PIX_FMT_YUYV422;//PIX_FMT_NV12;
		ret = IMP_FrameSource_SetChnAttr(0, &fs_chn_attr[0]);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_SetChnAttr failed\n", __func__, __LINE__);
			return -1;
		}

		/* Step.3 config sensor reg to output colrbar raw data*/
		/* to do */

		/* Step.4 Stream On */
		if (chn[0].enable){
			ret = IMP_FrameSource_EnableChn(chn[0].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn[0].index);
				return -1;
			}
		}

		/* Step.4 Snap raw */
		ret = IMP_FrameSource_SetFrameDepth(0, 1);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "%s(%d):IMP_FrameSource_SetFrameDepth failed\n", __func__, __LINE__);
			return -1;
		}
		return 0;
	}

	if (!first_on) {
		for (i = 0; i < FS_CHN_NUM; i++) {
			if (chn[i].enable) {
				ret = IMP_Encoder_CreateGroup(chn[i].index);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
					return -1;
				}
			}
		}
	}

	/* Step.3 Encoder init */
		//	clock_gettime(CLOCK_MONOTONIC, &ts);
	//	Ucamera_LOG("[%d.%d]++Encoder init++++\n",ts.tv_sec,ts.tv_nsec);
	switch (format) {
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_NV12:
		break;
	case V4L2_PIX_FMT_MJPEG:
		ret = sample_jpeg_init();
		break;
	case V4L2_PIX_FMT_H264:
		ret = sample_encoder_init();	
		break;
	}
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder init failed\n");
		return -1;
	}

	/* Step.4 Bind */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed\n",i);
				return -1;
			}
		}
	}
		/* Step.5 Stream On */
		//clock_gettime(CLOCK_MONOTONIC, &ts);
		//Ucamera_LOG("[%d.%d]sample_framesource_streamon\n",ts.tv_sec,ts.tv_nsec);
	ret = sample_framesource_streamon();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		return -1;
	}
		if(format != V4L2_PIX_FMT_H264)
		{
			while(motor_get_status()!=eMOTOR_ACTION_IDLE){
					usleep(10*1000);
			}  
		}

	if (format == V4L2_PIX_FMT_H264) {
		ret = IMP_Encoder_StartRecvPic(chn[0].index);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", chn[0].index);
			return -1;
		}

	}

	if (format == V4L2_PIX_FMT_MJPEG) {
		ret = IMP_Encoder_StartRecvPic(chn[0].index+3);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", chn[0].index);
		return -1;
		}
	}

	first_on = 1;

	return 0;
}

int imp_sdk_deinit(int format)
{
	int i, ret;

	if (format == V4L2_PIX_FMT_YUYV || format == V4L2_PIX_FMT_NV12) {
		/* Step.5 Stream Off */
		if (chn[0].enable){
			ret = IMP_FrameSource_DisableChn(chn[0].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn[0].index);
				return -1;
			}
		}

		/* Step.6 FrameSource exit */
		if (chn[0].enable) {
			/*Destroy channel i*/
			ret = IMP_FrameSource_DestroyChn(0);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn() error: %d\n", ret);
				return -1;
			}
		}
	} else {

		if (format == V4L2_PIX_FMT_H264) {
		#if 1
			ret = IMP_Encoder_StopRecvPic(chn[0].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic(%d) failed\n", chn[0].index);
				return -1;
			}
		#else
			sample_get_h264_stop();
		#endif
		}
		
		if (format == V4L2_PIX_FMT_MJPEG) {
			ret = IMP_Encoder_StopRecvPic(chn[0].index+3);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic(%d) failed\n", chn[0].index);
				return -1;
			}

		}

		/* Step.a Stream Off */
		ret = sample_framesource_streamoff();
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
			return -1;
		}

		/* Step.b UnBind */
		for (i = 0; i < FS_CHN_NUM; i++) {
			if (chn[i].enable) {
				ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
					return -1;
				}
			}
		}

		/* Step.c Encoder exit */
		switch (format) {
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_NV12:
			break;
		case V4L2_PIX_FMT_MJPEG:
			ret = sample_jpeg_exit();
			break;
		case V4L2_PIX_FMT_H264:
			ret = sample_encoder_exit();
			break;
		}
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "Encoder init failed\n");
			return -1;
		}
/*
 *
 *        for (i = 0; i < FS_CHN_NUM; i++) {
 *            if (chn[i].enable) {
 *                ret = IMP_Encoder_DestroyGroup(chn[i].index);
 *                if (ret < 0) {
 *                    IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
 *                    return -1;
 *                }
 *            }
 *        }
 */

		/* Step.d FrameSource exit */
		ret = sample_framesource_exit();
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
			return -1;
		}
	}

/*
 *    if (g_Power_save) {
 *        ret = IMP_ISP_DisableTuning();
 *        if(ret < 0){
 *            IMP_LOG_ERR(TAG, "IMP_ISP_DisableTuning failed\n");
 *            return -1;
 *        }
 *
 *        ret = IMP_ISP_DisableSensor();
 *        if(ret < 0){
 *            IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
 *            return -1;
 *        }
 *    }
 */

	return 0;
}

int imp_system_exit(void)
{
	int ret;
	/* Step.e System exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
	}
	return ret;
}

int imp_SensorFPS_Adapture(void)
{
	int ret, ev_cur = 0;
	int sensor_FPS = 0;
	static int  ev_last = 0;
	IMPISPEVAttr evattr = {0};

	if (Ucam_Stream_On == 0)
		return 0;

	ret = IMP_ISP_Tuning_GetEVAttr(&evattr);
	if (ret < 0) {
		Ucamera_LOG("failed to get evattr\n");
		return -1;
	}
	Ucamera_LOG("IMP get Ev:%d\n", evattr.ev);

	if (evattr.ev > 8000)
		ev_cur = 2;
	else if (evattr.ev > 4000)
		ev_cur = 1;
	else
		ev_cur = 0;

	if (ev_cur == ev_last)
		return 0;

	ev_last = ev_cur;

	switch (ev_cur) {
	case 2:
		sensor_FPS = 15;
		break;
	case 1:
		sensor_FPS = 20;
		break;
	case 0:
		sensor_FPS = g_Fps_Num;
		break;
	default:
		return 0;
	}

	ret = IMP_ISP_Tuning_SetSensorFPS(sensor_FPS, 1);
	if (ret < 0) {
		Ucamera_LOG("failed to set sensor fps\n");
		return -1;
	}

	return 0;
}


int ucamera_load_config(struct Ucamera_Cfg *ucfg )
{
	int nframes = 0;
	unsigned int i, width, height;
	char key[64] = {0};
	char value[64] = {0};
	char *line_str = NULL;
	FILE *fp = NULL;
	struct Ucamera_Video_Cfg *vcfg;
	int intervals = 10000000/30;

	vcfg = &(ucfg->vcfg);
	if ((fp = fopen("/system/config/uvc.config", "r")) == NULL) {
		Ucamera_LOG("%s open config file failed!\n", __func__);
		return -1;
	}
	line_str = malloc(256*sizeof(char));

	/* printf("\n**********Config Param**********\n"); */
	while (!feof(fp)) {
		if (fscanf(fp, "%[^\n]", line_str) < 0)
			break;
		fseek(fp , 1, SEEK_CUR);

		if (sscanf(line_str, "%[^:]:%[^\n]", key, value) != 2) {
			printf("warning: skip config %s\n", line_str);
			fseek(fp , 1, SEEK_CUR);
			continue;
		}

		char *ch = strchr(key, ' ');
		if (ch) *ch = 0;
		if (strcmp(key, "sensor_name") == 0) {
			/* printf("%s %s\n", key, value); */
			strncpy(g_Sensor_Name, value, sizeof(g_Sensor_Name));
		}else if(strcmp(key, "focus_trigger_value") == 0){
			focus_trigger_value = strToInt(value);
		}else if (strcmp(key, "i2c_addr") == 0) {
			/* printf("%s %s\n", key, value); */
			g_i2c_addr = htoi(value);
		} else if (strcmp(key, "fps_num") == 0) {
			/* printf("%s %s\n", key, value); */
			g_Fps_Num = strToInt(value);
		} else if (strcmp(key, "width") == 0) {
			/* printf("%s %s\n", key, value); */
			g_VideoWidth = strToInt(value);
		} else if (strcmp(key, "height") == 0) {
			/* printf("%s %s\n", key, value); */
			g_VideoHeight = strToInt(value);
		} else if (strcmp(key, "wdr_en") == 0) {
			/* printf("%s %s\n", key, value); */
			g_wdr = strToInt(value);
		} else if (strcmp(key, "bitrate") == 0) {
			/* printf("%s %s\n", key, value); */
			g_BitRate = strToInt(value);
		} else if (strcmp(key, "audio_en") == 0) {
			 printf("%s %s\n", key, value); 
			g_Audio = strToInt(value);
			ucfg->audio_en = g_Audio;
		} else if (strcmp(key, "gop") == 0) {
			/* printf("%s %s\n", key, value); */
			g_gop = strToInt(value);
		} else if (strcmp(key, "qp_value") == 0) {
			/* printf("%s %s\n", key, value); */
			g_QP = strToInt(value);
		} else if (strcmp(key, "adb_en") == 0) {
			/* printf("%s %s\n", key, value); */
			g_adb = strToInt(value);
			ucfg->adb_en = g_adb;
		} else if (strcmp(key, "stillcap_en") == 0) {
			/* printf("%s %s\n", key, value); */
			ucfg->stillcap = strToInt(value);
		} else if (strcmp(key, "rndis_en") == 0) {
			/* printf("%s %s\n", key, value); */
			g_rndis = strToInt(value);
		} else if (strcmp(key, "dmic_en") == 0) {
			/* printf("%s %s\n", key, value); */
			g_dmic = strToInt(value);
		} else if (strcmp(key, "speak_en") == 0) {
			/* printf("%s %s\n", key, value); */
			g_Speak = strToInt(value);
		} 
		#if 0
		else if (strcmp(key, "hid_en") == 0) {
			/* printf("%s %s\n", key, value); */
			ucfg->hid_en = strToInt(value);
		}
		#endif
		else if (strcmp(key, "h264_en") == 0) {
			/* printf("%s %s\n", key, value); */
			ucfg->h264_en = strToInt(value);
		} else if (strcmp(key, "uvc_led") == 0) {
			/* printf("%s %s\n", key, value); */
			g_led = strToInt(value);
		} else if (strcmp(key, "mic_volume") == 0) {
			/* printf("%s %s\n", key, value); */
			g_Volume = strToInt(value);
		} else if (strcmp(key, "audio_ns") == 0) {
			/* printf("%s %s\n", key, value); */
			g_Audio_Ns = strToInt(value);
		} else if (strcmp(key, "hvflip") == 0) {
			/* printf("%s %s\n", key, value); */
			g_HV_Flip = strToInt(value);
		} else if (strcmp(key, "dynamic_fps") == 0) {
			/* printf("%s %s\n", key, value); */
			g_Dynamic_Fps = strToInt(value);
		} else if (strcmp(key, "product_lab") == 0) {
			/* printf("%s %s\n", key, value); */
			strncpy(product_label, value, LABEL_LEN);
		} else if (strcmp(key, "vendor_lab") == 0) {
			/* printf("%s %s\n", key, value); */
			strncpy(vendor_label, value, LABEL_LEN);
		} else if (strcmp(key, "rcmode") == 0) {
			/* printf("%s %s\n", key, value); */
			if (strcmp(value, "vbr") == 0) {
				g_RcMode = IMP_ENC_RC_MODE_VBR;
			} else if (strcmp(value, "cbr") == 0) {
				g_RcMode = IMP_ENC_RC_MODE_CBR;
			} else if (strcmp(value, "fixqp") == 0) {
				g_RcMode = IMP_ENC_RC_MODE_FIXQP;
			} else if (strcmp(value, "cappedvbr") == 0) {
				g_RcMode = IMP_ENC_RC_MODE_CAPPED_VBR;
			} else {
				printf("Invalid RC method: %s\n", value);
			}
		} else if (strcmp(key, "nframes") == 0) {
			/* printf("%s %s\n", key, value); */
			i = 0;
			struct Ucamera_YUYV_Param *yuyvl;
			struct Ucamera_JPEG_Param *jpegl;
			struct Ucamera_H264_Param *h264l;
			nframes = strToInt(value);
			vcfg->yuyvnum = nframes;
			vcfg->jpegnum = nframes;
			vcfg->h264num = nframes;
			yuyvl = malloc(vcfg->yuyvnum*sizeof(struct Ucamera_YUYV_Param));
			jpegl = malloc(vcfg->jpegnum*sizeof(struct Ucamera_JPEG_Param));
			h264l = malloc(vcfg->h264num*sizeof(struct Ucamera_H264_Param));

			while (i < nframes) {
				if (fscanf(fp, "%[^\n]", line_str) < 0)
					break;
				sscanf(line_str, "{%d, %d}", &width, &height);

				if (width > 0 && height > 0 && (width%16 == 0)) {

					yuyvl[i].width = width;
					yuyvl[i].height = height;
					yuyvl[i].fps_num = intervals;
					jpegl[i].width = width;
					jpegl[i].height = height;
					jpegl[i].fps_num = intervals;
					h264l[i].width = width;
					h264l[i].height = height;
					h264l[i].fps_num = intervals;
				} else {
					printf("error(%s %d)Invalid width or height(%d %d)\n", __func__, __LINE__, width, height);
				}
				i++;
				fseek(fp , 1, SEEK_CUR);
			}
			vcfg->yuyvlist = yuyvl;
			vcfg->jpeglist = jpegl;
			vcfg->h264list = h264l;
		} else {
			printf("Invalid config param: %s\n", key);
		}

	}
	/* printf("******************************\n"); */
	free(line_str);
	return 0;
}
#define MOTOR_ACTION_IDLE 0
#define MOTOR_ACTION_OPEN 1
#define MOTOR_ACTION_CLOSE 2

static struct itimerval oldtv;
struct itimerval itv;
//static char motor_enable=1;
//static char door_opened=0;
static char g_motor_action=MOTOR_ACTION_IDLE;

//static char times=0;
void set_motor_timer(char action,unsigned int time)//time :ms
{
#if 1
	   struct timespec ts;
	 	clock_gettime(CLOCK_MONOTONIC, &ts);

	   Ucamera_LOG("[%d.%d]++set_timer action=%d delay time=%=ld++++\n",ts.tv_sec,ts.tv_nsec,action,time);
           g_motor_action=action;

      // printf("set timer \n ");
       // motor_enable=0;
        itv.it_interval.tv_sec = 0;
        itv.it_interval.tv_usec = 0;  //启动后的定时器每隔990ms唤醒一次
        itv.it_value.tv_sec = time/1000;
        itv.it_value.tv_usec =(time%1000)*1000;   //定时器在100ms后启动
        setitimer(ITIMER_REAL, &itv, &oldtv); 
           //ITIMER_REAL表示每次定时器唤醒时将会触发SIGALRM信号
         if(time==0)
         	{
					if(g_motor_action==MOTOR_ACTION_OPEN)
					{
							printf("timer handle open door\n");
							if(motor_get_status()!=eMOTOR_ACTION_IDLE)
							{
								motor_stop();
							}      		
							//	sample_set_mic_mute(1);
							motor_open_door();
								sample_ucamera_led_ctl(g_led, 0);

	

					}
					else if(g_motor_action==MOTOR_ACTION_CLOSE)
					{
							printf("timer handle close door\n");
							if(motor_get_status()!=eMOTOR_ACTION_IDLE)
							{
									motor_stop();
							}   
							//	sample_set_mic_mute(1);
							motor_close_door();
								Ucam_Stream_On = 0;
									sample_ucamera_led_ctl(g_led, 1);

							//	sample_set_mic_mute(0);
					}
					else if(g_motor_action==eMOTOR_ACTION_IDLE)
					{
						//	sample_set_mic_mute(0);

					}

				 }
				 #endif
}

void shut_motor_timer()
{
        itv.it_value.tv_sec = 0;        //将启动参数设置为0,表示定时器不启动
        itv.it_value.tv_usec = 0;
        setitimer(ITIMER_REAL, &itv, &oldtv);
}
//struct Ucamera_Video_Frame *g_pframe;

void signal_motor_handler(int m)
{
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		Ucamera_LOG("[%d.%d]++signal_handler++++\n",ts.tv_sec,ts.tv_nsec);
	  shut_motor_timer();

		if(g_motor_action==MOTOR_ACTION_OPEN)
		{
		       printf("timer handle open door\n");		

				if(motor_get_status()!=eMOTOR_ACTION_IDLE)
				{
					motor_stop();
				}      		
				motor_open_door();
				sample_ucamera_led_ctl(g_led, 0);


		}
		else if(g_motor_action==MOTOR_ACTION_CLOSE)
			{
					 printf("timer handle close door\n");

					if(motor_get_status()!=eMOTOR_ACTION_IDLE)
					{
						motor_stop();
					}   
				//	sample_set_mic_mute(1);
					motor_close_door();
					sample_ucamera_led_ctl(g_led, 1);

					Ucam_Stream_On = 0;
				//	sample_set_mic_mute(0);

		}		
}
static int uvc_event_process(int event_id, void *data)
{
	int retry_cnt = 0;
	int ret;
	struct timespec ts;
	static struct timespec ts_lst;
	long timedif;//us
	static struct Ucamera_Video_Frame lst_fram={0,0,0};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	switch (event_id) {
	case UCAMERA_EVENT_STREAMON: {	
		sample_set_mic_mute(1);

		Ucamera_LOG("[%d.%d]++UCAMERA_EVENT_STREAMON++++\n",ts.tv_sec,ts.tv_nsec);
		struct Ucamera_Video_Frame *frame = (struct Ucamera_Video_Frame *) data;
		g_burnin_test_mode=0;
		if(Ucam_Stream_On==1)
		{
		   Ucamera_LOG("current state is in stream on!!!\n");
	 		//imp_sdk_deinit(lst_fram.fcc);	
		}
		Ucam_Stream_On = 1;  

	//	if (g_led)
	//		sample_ucamera_led_ctl(g_led, 0);
imp_init_check:
		if (!imp_inited) {
			if (retry_cnt++ > 80) {
				Ucamera_LOG("[error]imp system init failed.\n");
				return 0;
			}
			Ucamera_LOG("imp sys not ready, wait and retry:%d", retry_cnt);
			usleep(100*1000);
			goto imp_init_check;
		}
		if(frame->fcc==V4L2_PIX_FMT_MJPEG)
			{
			set_motor_timer(MOTOR_ACTION_OPEN,0);
			}
		else
			{
			set_motor_timer(MOTOR_ACTION_OPEN,0);
		}
	
	if(frame->fcc==V4L2_PIX_FMT_MJPEG)
		{
		      while(motor_get_status()!=eMOTOR_ACTION_IDLE)
				usleep(10*1000);
				//printf("waite motor opened \n");
		}   
#if 0	
	  if(lst_fram.fcc)
		{
		   if((lst_fram.fcc!=frame->fcc)||(lst_fram.height!=frame->height)||(lst_fram.width!=frame->width))
		   	{
			 imp_sdk_deinit(lst_fram.fcc);	
			 ret=imp_sdk_init(frame->fcc, frame->width, frame->height);
		   	}
		}else
			{
			 ret=imp_sdk_init(frame->fcc, frame->width, frame->height);
		}  
#else
		ret=imp_sdk_init(frame->fcc, frame->width, frame->height);
#endif
		
			sample_set_mic_mute(0);
			lst_fram.fcc=frame->fcc;
			lst_fram.height=frame->height;
			lst_fram.width=frame->width;
			return ret;

	}
	case UCAMERA_EVENT_STREAMOFF: {	
		//Ucamera_LOG("[%d.%d]++stream off timestamp 1++++\n",ts_lst.tv_sec,ts_lst.tv_nsec);
		 timedif = 1000000*(ts.tv_sec-ts_lst.tv_sec)+(ts.tv_nsec-ts_lst.tv_nsec)/1000;	
		ts_lst=ts;
		Ucamera_LOG("[%d.%d]++UCAMERA_EVENT_STREAMOFF++++diff=%d\n",ts.tv_sec,ts.tv_nsec,timedif);
		struct Ucamera_Video_Frame *frame = (struct Ucamera_Video_Frame *) data;

	//	if (g_led)
	//		sample_ucamera_led_ctl(g_led, 1);
		
		//add by nick
		#if 1
		ret= imp_sdk_deinit(frame->fcc);  
		#else
		lst_fram.fcc=frame->fcc;
		lst_fram.height=frame->height;
		lst_fram.width=frame->width;
       #endif
	#if 0
		if((timedif>1500000)&&(frame->fcc==V4L2_PIX_FMT_MJPEG))
				set_motor_timer(MOTOR_ACTION_CLOSE,750);
		else if((timedif>1500000)&&(frame->fcc==V4L2_PIX_FMT_H264))
				set_motor_timer(MOTOR_ACTION_CLOSE,450);
		else
			    set_motor_timer(MOTOR_ACTION_CLOSE,1);

#else
			   set_motor_timer(MOTOR_ACTION_CLOSE,2000);

#endif 
		Ucam_Stream_On = 0;

//	clock_gettime(CLOCK_MONOTONIC, &ts);
//	timedif = 1000000*(ts.tv_sec-ts_lst.tv_sec)+(ts.tv_nsec-ts_lst.tv_nsec)/1000;	
//	Ucamera_LOG("[%d.%d]++EVENT_STREAMOFF END ++++diff=%d\n",ts.tv_sec,ts.tv_nsec,timedif);
		return ret;
	}

	default:
		Ucamera_LOG("%s(ERROR): unknown message ->[%d]!\n", TAG, event_id);
		return -1;
	};

	return 0;
}

FILE *pcm_fd = NULL;
FILE *ref_fd = NULL;
short ref_pcm[640] = {0};
int sample_audio_pcm_get(short *pcm)
{
	int len;
#if 0
	static int i = 0;
	if (pcm_fd == NULL)
		pcm_fd = fopen("/tmp/dmic.pcm", "wb");
	if (ref_fd == NULL)
		ref_fd = fopen("/tmp/ref.pcm", "wb");

#endif
	len = sample_audio_amic_pcm_get(ref_pcm);
	len = sample_audio_dmic_pcm_get(pcm);
#if 0
	if (i++ < 1200) {
		if (pcm_fd)
			fwrite(pcm, 2, len/2, pcm_fd);
		if (ref_fd)
			fwrite(ref_pcm, 2, len/2, ref_fd);
	}
#endif
	return len;
}

void signal_handler(int signum) {
	Ucamera_LOG("catch signal %d\n", signum);
	if (signum == SIGUSR1)
		sem_post(&ucam_ready_sem);
	else {
		Ucamera_LOG("Ucamera Exit Now. \n");
		sample_audio_amic_exit();
		if (g_dmic)
			sample_audio_dmic_exit();
		imp_system_exit();
		Ucamera_DeInit();
	}
}

void *ucam_impsdk_init_entry(void *res)
{	
static struct timespec ts;

	sem_wait(&ucam_ready_sem);
	clock_gettime(CLOCK_MONOTONIC, &ts);
	printf("[%d.%d]sample_system_init start		++++++++++\n",ts.tv_sec,ts.tv_nsec);
	sample_system_init();

	imp_inited = 1;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	printf("[%d.%d]sample_system_init finish		++++++++++\n",ts.tv_sec,ts.tv_nsec);
	if (g_Audio == 1) {
		if (g_dmic)
			sample_audio_dmic_init();
		sample_audio_amic_init();
		Ucamera_Audio_Start();
	}

	if (g_Speak)
		sample_audio_play_start();
	return NULL;
}

#if AF_ON_OFF

int flag_refocus = 0;

static void motor_reset(int dac_pos , int fd , int tar_pos)
{
	int ret;
	while(dac_pos != tar_pos)
	{
		if(dac_pos > tar_pos){
			while(dac_pos != tar_pos){
				dac_pos -= 25;
				ret = ioctl(fd , MOTOR_MOVE , dac_pos);
				if(ret < 0){
					Ucamera_LOG("ioctl err! \n");
				}
				usleep(10 * 1000);
			}
		}
		else{
			while(dac_pos != tar_pos){
				dac_pos += 25;
				ret = ioctl(fd , MOTOR_MOVE , dac_pos);
				if(ret < 0 ){
					Ucamera_LOG("ioctl err! \n");
				}
				usleep(10 * 1000);
			}
		}
	}
	return;
}
/*	module function: Autofocus algorithm	*/
static int autoFocus(int fd , IMPISPAFHist af_hist )
{
	int i,j;
	int dac_value = 0;
	int dac_clarity = 0;
	int temp;
	int max_value = 0;
	int min_value = 0;
	int max_i = 0;
	int min_i = 0;
	int refocus_rate = 0;
	int ret;
	static int dac_pos = 100;
	int dac_clarity_buf[32];
	int metrices_buf[32] = {0};

	motor_reset(dac_pos, fd, 400);
        usleep(MOTOR_MOVE_SLEEP_TIME);
	dac_pos = 400;

	dac_clarity = 0;
	ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
	if( ret < 0 ){
		Ucamera_LOG("IMP_ISP_Tuning_GetAFMetrices err! \n");
		return -1;
	}
	dac_clarity = af_hist.af_stat.af_metrics_alt; 
	dac_clarity_buf[8] = dac_clarity; 
	printf("dac_clarity_buf[8] is %d \n" ,  dac_clarity_buf[8] );

	for(i = 7; i >= 2; i--){
		dac_value = 50 * i;
		ret = ioctl(fd, MOTOR_MOVE, dac_value);
		if(ret < 0){
			Ucamera_LOG("ioctl err! \n");
			return -1;
		}
		usleep(MOTOR_MOVE_SLEEP_TIME);
		dac_pos = dac_value;
		dac_clarity = 0;
		ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
		if( ret < 0 ){
			Ucamera_LOG("IMP_ISP_Tuning_GetAFMetrices err! \n");
			return -1;
		}
		dac_clarity = af_hist.af_stat.af_metrics_alt;
		dac_clarity_buf[i] = dac_clarity;
		printf("dac_clarity_buf[%d] is %d \n" , i ,  dac_clarity_buf[i]);
	}

	max_value = dac_clarity_buf[2];
	max_i = 2;
	for(i = 3; i <= 8; i++)
	{
		if( dac_clarity_buf[i] > max_value )
		{
			max_value = dac_clarity_buf[i];
			max_i = i ;
		}
	}

	min_value = dac_clarity_buf[2];
	min_i = 2;
	for(i = 3; i <= 8; i++)
	{
		if( dac_clarity_buf[i] < min_value )
		{
			min_value = dac_clarity_buf[i];
			min_i = i ;
		}
	}

	if(max_value != 0){
		refocus_rate = 100 * (max_value - min_value) / max_value;
		printf("refocus_rate is %d\n", refocus_rate);
		if(refocus_rate < 20){
			flag_refocus = 1;
		}
	}


	dac_value = max_i * 50;
	motor_reset(dac_pos , fd , dac_value);
	ret = ioctl(fd, MOTOR_MOVE, dac_value);
	if(ret < 0){
		Ucamera_LOG("ioctl err! \n");
		return -1;
	}
	printf("motor_postion is %d\n",dac_value);
	dac_pos = dac_value;

	return max_value;
}
static void *get_video_clarity(void *args)
{
	int clarity_cur;
	int clarity_now;
	int clarity_diff;
	int ret;
	int i,j;
	int fall_rate;
	int num_refocus = 0;
	IMPISPAFHist af_hist;
	IMPISPWeight af_weight;

	struct timeval ts_s, ts_d;



	unsigned char set_weight[15][15] = {
		{0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
		{0,1,1,1,1,1,1,0,1,1,1,1,1,1,1},
		{0,1,1,1,1,1,1,0,1,1,1,1,1,1,1},
		{0,1,1,8,8,1,1,0,1,1,1,1,1,1,1},
		{0,1,1,8,8,1,1,0,1,1,1,1,1,1,1},
		{0,1,1,1,1,1,1,0,1,1,1,1,1,1,1},
		{0,1,1,1,1,1,1,0,1,1,1,1,1,1,1},
		{0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

	};


/*
	unsigned char set_weight[15][15] = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},     
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,1,1,1,1,1,1,1,0,0,0,0},
		{0,0,0,0,1,3,3,3,3,3,1,0,0,0,0},
		{0,0,0,0,1,3,8,8,8,3,1,0,0,0,0},
		{0,0,0,0,1,3,8,8,8,3,1,0,0,0,0},
		{0,0,0,0,1,3,8,8,8,3,1,0,0,0,0},
		{0,0,0,0,1,3,3,3,3,3,1,0,0,0,0},
		{0,0,0,0,1,1,1,1,1,1,1,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	};
*/

	int fd = open("/dev/dw9714" , 0);
	if(fd == NULL){
		Ucamera_LOG("open err! \n");
		return NULL;
	}

	while(Ucam_Stream_On == 0){
		usleep(1000 * 1000);
	}

	while(IMP_ISP_Tuning_GetAfHist(&af_hist) < 0){
		usleep(1000 * 1000);
	}

	Ucamera_LOG("af_enable is %u \n" , af_hist.af_enable);
	Ucamera_LOG("af_metrics_shift is %u \n" , af_hist.af_metrics_shift);
	Ucamera_LOG("af_delta is %u \n" , af_hist.af_delta);
	Ucamera_LOG("af_theta is %u \n" , af_hist.af_theta);
	Ucamera_LOG("af_hilight_th is %u \n" , af_hist.af_hilight_th);
	Ucamera_LOG("af_alpha_alt is %u \n" , af_hist.af_alpha_alt);
	Ucamera_LOG("af_hstart is %u \n" , af_hist.af_hstart);
	Ucamera_LOG("af_vstart is %u \n" , af_hist.af_vstart);
	Ucamera_LOG("af_stat_nodeh is %u \n" , af_hist.af_stat_nodeh);
	Ucamera_LOG("af_stat_nodev is %u \n\n\n" , af_hist.af_stat_nodev);

	for(i = 0 ; i < 15 ; i++){
		for(j = 0 ; j < 15 ; j++){
			af_weight.weight[i][j] = set_weight[i][j];
		}
	}

	ret = IMP_ISP_Tuning_SetAfWeight(&af_weight);
	if(ret < 0){
		Ucamera_LOG("IMP_ISP_Tuning_SetAfWeight err! \n");
	}


	ret = IMP_ISP_Tuning_GetAfWeight(&af_weight);
	if(ret < 0){
		Ucamera_LOG("IMP_ISP_Tuning_GetAfHist err! \n");
		return NULL;
	}
	printf("af_weight is : \n");
	for(i = 0 ; i < 15 ; i++ ){
		for( j = 0; j < 15 ; j++){
			printf("%u " , af_weight.weight[i][j]);
		}
		printf("\n");
	}

	clarity_cur=autoFocus(fd , af_hist );
	if(clarity_cur < 0){
		Ucamera_LOG("autoFocus err! \n");
	}

	dac_pos_old = dac_pos;
	int focus_trigger_value_now = focus_trigger_value;


	while(1)
	{
		while(Ucam_Stream_On == 0)
			usleep(1000 * 1000);

		if(focus_auto.data[UVC_CUR] == 1){
			ret = IMP_ISP_Tuning_GetAFMetrices(&af_hist.af_stat.af_metrics);
			if(ret < 0){
				Ucamera_LOG("IMP_ISP_Tuning_GetAFMetrices err! \n");
			}
			clarity_now = af_hist.af_stat.af_metrics;
			clarity_diff = abs(clarity_cur - clarity_now);
			fall_rate = (100 * clarity_diff) / clarity_cur;
			if( fall_rate > focus_trigger_value_now)
			{
				usleep(200*1000);
				ret = IMP_ISP_Tuning_GetAFMetrices(&af_hist.af_stat.af_metrics);
				if(ret < 0){
					Ucamera_LOG("IMP_ISP_Tuning_GetAFMetrices err! \n");
				}
				clarity_now = af_hist.af_stat.af_metrics;
				clarity_diff = abs(clarity_cur - clarity_now);
				fall_rate = (100 * clarity_diff) / clarity_cur;
				if(fall_rate > focus_trigger_value_now ){
					printf("clarity_cur = %d, clarity_now = %d, fall_rate = %d, focus_trigger_value_now = %d\n", clarity_cur, clarity_now, fall_rate, focus_trigger_value_now);
					dac_pos_old = dac_pos;
					gettimeofday(&ts_s , NULL);
					clarity_cur = autoFocus(fd , af_hist );
					if(clarity_cur <= 0){
						Ucamera_LOG("autoFocus err! \n");
					}
					gettimeofday(&ts_d, NULL);
					printf("####AF time = %llums ####\n", ((((unsigned long long )ts_d.tv_sec * 1000000) + ts_d.tv_usec) - ((( unsigned long long)ts_s.tv_sec * 1000000) + ts_s.tv_usec)) / 1000);
					while(flag_refocus == 1) {
						flag_refocus = 0;
						dac_pos_old = dac_pos;
						clarity_cur = autoFocus(fd , af_hist );
						num_refocus++;
						if(num_refocus >= 2){
							flag_refocus = 0;
						}
					}
					num_refocus = 0;
					usleep(300*1000);
					ret = IMP_ISP_Tuning_GetAFMetrices(&af_hist.af_stat.af_metrics);
					if(ret < 0){
						Ucamera_LOG("IMP_ISP_Tuning_GetAFMetrices err! \n");
					}
					clarity_cur = af_hist.af_stat.af_metrics;
					focus_trigger_value_now = (focus_trigger_value - focus_trigger_value * abs(dac_pos - dac_pos_old) / 400);
				}
			}
			usleep(300*1000);
		}
		else{
			if(dac_pos != focus.data[UVC_CUR]){
				dac_pos = focus.data[UVC_CUR];
				ret = ioctl(fd, MOTOR_MOVE, dac_pos);
				if(ret < 0){
					Ucamera_LOG("ioctl err! \n");
					return NULL;
				}
			}
		}
	}
}
#endif
#if 0
static void *motor_cnt(void *args)
{
	int fd;
	int flag;
	printf("111111111111111111111111\n");
	while(Ucam_Stream_On == 0){
		usleep(1000 * 1000);
	}
	fd = open("/dev/motor", 0);
	if(fd == NULL){
		Ucamera_LOG("open /dev/motor err! \n");
		return NULL;
	}
	flag = Ucam_Stream_On + 1 ;
	while(1){
		if(flag != Ucam_Stream_On){
			if(Ucam_Stream_On == 1){
				jb_motors_steps.x = -1800;
				jb_motors_steps.y = 0;
				ioctl(fd, 3, (unsigned long)&jb_motors_steps);
				flag = Ucam_Stream_On;
				printf("22222222222222222222\n");
			}
			else{
				jb_motors_steps.x = 1800;
				jb_motors_steps.y = 0;
				ioctl(fd, 3, (unsigned long)&jb_motors_steps);
				flag = Ucam_Stream_On;
				printf("333333333333333333333\n");
			}
		}
		usleep(1000 * 1000);
	}


	
}
#endif
int wtd_fd;
int flash_fd;
static int wdt_keep_alive(void)
{
	int ret = -1;
    int dummy;
    ret = ioctl(wtd_fd, WDIOC_KEEPALIVE, &dummy);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;
}

static int wdt_enable()
{
	int ret = -1;
	int flags = 0;
	flags = WDIOS_ENABLECARD;
	ret = ioctl(wtd_fd, WDIOC_SETOPTIONS, &flags);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;

}

static int wdt_disable()
{
	int ret = -1;
	int flags = 0;
	flags = WDIOS_DISABLECARD;
	ret = ioctl(wtd_fd, WDIOC_SETOPTIONS, &flags);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;

}

static int wdt_set_timeout(int to)
{
	int ret = -1;
    int timeout = to;
    ret = ioctl(wtd_fd, WDIOC_SETTIMEOUT, &timeout);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;

}

static int wdt_get_timeout()
{
	int ret = -1;
    int timeout = 0;
    ret = ioctl(wtd_fd, WDIOC_GETTIMEOUT, &timeout);
	if (0 != ret) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return timeout;

}
pthread_t burnin_test_pid;
static unsigned int tick_count=0;
unsigned int  motor_test_timer=0;
unsigned int  led_test_timer=0;
unsigned int burn_test_failed_flag=0;
#define BURNIN_TEST_TIME  36000
#define BURNIN_TEST_FINISH_FLAG 0x12345678
#define FLASH_ADDR_FINISH_FLAG 0x400
void TimerSet(unsigned int  *STimer, unsigned int  TimeLength)
{
	*STimer = tick_count + TimeLength;

	if(*STimer == 0)	*STimer = 1; //not set timer to 0 for timer is running
}
unsigned int  TimerHasExpired (unsigned int  *STimer)
{
	if(*STimer == 0)
		return 1;
	else if((tick_count - *STimer) <= 0x7fffffff)
	{
		*STimer = 0;	//set timer to stop
		return 1;
	}
	else
		return 0;
}

	char failed_before=0;

void motor_test(unsigned int  tick)
{
	static unsigned int motor_test_ount=0;
	static unsigned char motor_test_state=1;
	static unsigned char open_try_cnt=0;
	static unsigned char close_try_cnt=0;
   char savecount[4]={0};
  if(TimerHasExpired(&motor_test_timer)==0)
		   return;
	

	switch(motor_test_state)
		{
		case 0:
			motor_open_door();	
			motor_test_ount++;
			if(motor_door_opened()==0)
			{
			   TimerSet(&motor_test_timer,10);	
				open_try_cnt++;
				motor_test_state=1;

				if(open_try_cnt>=3)
					{
					printf("test failed\n");
				    burn_test_failed_flag=1;
					failed_before++;
			
					if(failed_before<3)
						{	
						       snprintf(savecount, sizeof(savecount), "%d", failed_before);
								write_conf_value("failed",savecount,"/system/config/motor_test");		//reboot
								run_cmd("sync");
								run_cmd("reboot");
						}
			
				}
			}
			else{	
				open_try_cnt=0;
				if(motor_test_ount<=3)
             {
			   TimerSet(&motor_test_timer,20);	
					}
           else{
				 TimerSet(&motor_test_timer,100);	
           	}
	 	
			motor_test_state=1;
				}
			
			
			break;
		case 1:
			motor_close_door();
			if(motor_door_closed()==0)
			{
				TimerSet(&motor_test_timer,10);	
				close_try_cnt++;
				motor_test_state=0;
				if(close_try_cnt>=3)
					{
					 printf("test failed\n");
				     burn_test_failed_flag=1;
					failed_before++;
			
					if(failed_before<3)
						{	
						       snprintf(savecount, sizeof(savecount), "%d", failed_before);
								write_conf_value("failed",savecount,"/system/config/motor_test");		//reboot
								run_cmd("sync");
								run_cmd("reboot");
						}
				}
			}
			else
				{
			close_try_cnt=0;
			motor_test_state=0;
       	if(motor_test_ount<=3)
       		{
			   TimerSet(&motor_test_timer,20);	
       		}
           else
           	{
				 TimerSet(&motor_test_timer,5);
           	}
				}
		
		break;
		default:
			break;
	}
}
void led_test(unsigned int  tick,unsigned int delay)
	{
	static unsigned char led_test_state=0;

  if(TimerHasExpired(&led_test_timer)==0)
		   return;
	switch(led_test_state)
		{
		case 0:
			sample_ucamera_led_ctl(g_led, 0);
			TimerSet(&led_test_timer,delay);
			led_test_state=1;
			break;
		case 1:
			sample_ucamera_led_ctl(g_led, 1);
			TimerSet(&led_test_timer,delay);
			led_test_state=0;
			break;
		default:
			break;
	}
	
}


void *burnin_TestThread(void *p)
{
	unsigned int finish_flag;
	int	fd;
	char failed_rec[8]="";
	failed_before=0;
	if(read_conf_value("failed",failed_rec,"/system/config/motor_test")==0)
		{
			printf("motor_test failed=%s\n",failed_rec);
			failed_before=strToInt(failed_rec);	
	}
		if(read_conf_value("time",failed_rec,"/system/config/motor_test")==0)
		{
			printf("time=%s\n",failed_rec);
			tick_count=strToInt(failed_rec);	
	}


	sample_ucamera_led_ctl(g_led, 0);
	motor_open_door();	
	return NULL;
}
/* ---------------------------------------------------------------------------
 * main
 */


int main(int argc, char *argv[])
{
	int ret;
	int i;
	unsigned int burnin_test_flag;
		unsigned int iboot_flag;

	unsigned int md5_lic_key[4];
	unsigned int keys[4];
	//unsigned int otp_test[4];
    //int otp_fd;

    char update_enable[4]="";
	struct Ucamera_Cfg ucfg = {0};
	struct Ucamera_Video_CB_Func v_func;
	struct Ucamera_Audio_CB_Func a_func;
/*
	if (AntiCopy_Verify()) {
		Ucamera_LOG("AntiCopy Verified failed!!!\n");
		return 0;
	}
*/	

	static struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	printf("[%d.%d]++++++++++uvc main++++++++++\n",ts.tv_sec,ts.tv_nsec);


//add by nick
	if(read_conf_value("enable",update_enable,"/system/config/update.config")==0)
   {
          printf("update enable=%s\n",update_enable);
          if(strcmp(update_enable, "1") == 0)/* update mode*/
          	{
				g_firmware_update=1;
				load_umass_driver();
				/*disable update mode after reboot*/
				write_conf_value("enable","0","/system/config/update.config");
          	}					
	}
	else
	{
		printf("can't load update.config\n");
	}


	if (ucamera_load_config(&ucfg)) {
		Ucamera_LOG("ucamera load config failed!\n");
		return 0;
	}

	if (ucamera_uvc_pu_attr_load()) {
		Ucamera_LOG("[ERROR] load uvc PU attr failed.\n");
		return 0;
	}

	if (g_Speak)
		ucfg.pcfg.Pid = PRODUCT_ID + 1;
	else
		ucfg.pcfg.Pid = PRODUCT_ID;
	
	ucfg.pcfg.Vid = VENDOR_ID;

/*update mode */
	if(g_firmware_update==1)
	{
			ucfg.pcfg.Vid = 0x1B20;
			ucfg.pcfg.Pid =  0x0300;
	}
	ucfg.pcfg.version = DEVICE_BCD;
	strcpy(ucfg.pcfg.manufacturer, vendor_label);
	strcpy(ucfg.pcfg.product, product_label);
	strcpy(ucfg.pcfg.serial, serial_label);
	ucfg.acfg.mic_volume = 0x0600;
	ucfg.acfg.spk_volume = 0x0;
	if (g_Speak)
		ucfg.acfg.speak_enable = 1;
	else
		ucfg.acfg.speak_enable = 0;

	Ucamera_Config(&ucfg);
	flash_fd= open("/dev/mtdblock0",O_RDWR);
   mtd_flash_read(flash_fd,0x0c,(unsigned char*)&iboot_flag,4);
	printf("iboot flag=0x%x\n",iboot_flag);	
	if(iboot_flag==0x4e5c)
	{
		run_cmd("/system/init/update_iboot.sh");
		printf("update iboot\n");
		//sleep(1);  
	}
	close(flash_fd);
	Ucamera_Init(UVC_BUF_NUM, UVC_BUF_SIZE);
/*update mode */
	while(g_firmware_update==1)
		{
		   /*do nothing ,just waiting*/
         sleep(5);
		  printf("update mode\n");
	}

	v_func.get_YuvFrame = sample_get_yuv_snap;
	v_func.get_JpegFrame = sample_get_jpeg_snap;
	v_func.get_H264Frame = sample_get_h264_snap;
	Ucamera_Video_Regesit_CB(&v_func);

	backlight_compens.set = sample_video_backlight_compens_set;
	backlight_compens.get = sample_video_backlight_compens_get;
	brightness.set = sample_video_brightness_set;
	brightness.get = sample_video_brightness_get;
	contrast.set = sample_video_contrast_set;
	contrast.get = sample_video_contrast_get;
	saturation.set = sample_video_saturation_set;
	saturation.get = sample_video_saturation_get;
	sharpness.set = sample_video_sharpness_set;
	sharpness.get = sample_video_sharpness_get;
	hue.set = sample_video_hue_set;
	hue.get = sample_video_hue_get;
	whitebalance.set = sample_video_whitebalance_set;
	whitebalance.get = sample_video_whitebalance_get;
	powerlinefreq.set = sample_video_powerlinefreq_set;
	powerlinefreq.get = sample_video_powerlinefreq_get;
	gamma.set = sample_video_gamma_set;
	gamma.get = sample_video_gamma_get;
	focus.set = sample_video_focus_set;
	focus.get = sample_video_focus_get;
	focus_auto.set = sample_video_focus_auto_set;
	focus_auto.get = sample_video_focus_auto_get;
	exposure_time.set = sample_video_exposure_time_set;
	exposure_time.get = sample_video_exposure_time_get;
	auto_exposure_mode.set = sample_video_auto_exposure_mode_set;
	auto_exposure_mode.get = sample_video_auto_exposure_mode_get;


	Ucamera_Video_Regesit_Process_Unit_CB(Pu_ctrl);
    Ucamera_Video_Regesit_Camera_Terminal_CB(Ct_ctrl);

	UCamera_Registe_Event_Process_CB(uvc_event_process);
	UCamera_Video_Start();

	if (g_Audio == 1) {
		if (g_dmic)
			a_func.get_AudioPcm = sample_audio_dmic_pcm_get;
		else
			a_func.get_AudioPcm = sample_audio_amic_pcm_get;
		a_func.set_Mic_Volume = sample_set_mic_volume;
		a_func.set_Spk_Volume = sample_set_spk_volume;
		a_func.set_Mic_Mute = sample_set_mic_mute;
		a_func.set_Spk_Mute = sample_set_spk_mute;
		Ucamera_Audio_Regesit_CB(&a_func);
	}

	if (g_led)
		sample_ucamera_led_init(g_led);

	sem_init(&ucam_ready_sem, 0, 0);
	signal(SIGKILL, signal_handler); /*set signal handler*/
	signal(SIGUSR1, signal_handler); /*set signal handler*/
	signal(SIGTERM, signal_handler); /*set signal handler*/

	pthread_t ucam_impsdk_init_id;
	ret = pthread_create(&ucam_impsdk_init_id, NULL, ucam_impsdk_init_entry, NULL);
	if (ret != 0) {
		Ucamera_LOG("pthread_create failed \n");
		return -1;
	}

#if	AF_ON_OFF
	pthread_t tid_AF;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	ret = pthread_create(&tid_AF, &attr, get_video_clarity, NULL);
	if (ret != 0) {
		 Ucamera_LOG("pthread_create err \n");
		 return -1;
	}
#endif
	//add by nick 
	signal(SIGALRM, signal_motor_handler);

	if(open_motor_dev()>0)  
	{     
			motor_set_speed(8000);			
	}	
	else 
	{     
			printf("open motor driver failed\n");	
	}
			
	wtd_fd = open("/dev/watchdog", O_WRONLY);
	if (-1 == wtd_fd) {
		printf("err(%s,%d): %s\n", __func__, __LINE__, strerror(errno));
		exit(-1);
	}
	
	wdt_enable();
	wdt_set_timeout(15);
	printf("firmware version:%s  date :2021-0611 \n ",fw_version_string);
	printf("iq version:%s \n",iq_version_string);
	printf("hw version:%s \n",hw_version_string);

	usleep(1000*1000*2);     
if((Ucam_Stream_On==0)&&(g_burnin_test_mode==0))	
	{		
			motor_close_door();		
			//motor_open_door();		
	}	
	if(g_burnin_test_mode==1)
	{
		pthread_create(&burnin_test_pid, 0, burnin_TestThread, NULL);
	}
	while (1) {
		if (g_Dynamic_Fps)
			{
			imp_SensorFPS_Adapture();	
			}
			usleep(1000*1000*2);     
			wdt_keep_alive();
	}	
	wdt_disable();
    close(wtd_fd);

	return 0;
}
