/*
 * main.c
 *
 * Copyright (C) 2022 Ingenic Semiconductor Co.,Ltd
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <semaphore.h>
#include <linux/input.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <node_list.h>
#include <usbcamera.h>
#include <AntiCopy_Verify.h>
#include <imp-common.h>
#include <module_af_control.h>
#include <module_ucamera_control.h>
#include <module_led_control.h>
#include "global_config.h"
#include "main.h"
/*
#include "uac_control.h"
#include "uvc_control.h"
*/

#include <module_key_gpio.h>

#ifdef MODULE_FACEAE_ENABLE
#include <module_faceAE.h>
#endif

#ifdef MODULE_TRACK_ENABLE
#include <module_track_control.h>
#endif

#ifdef MODULE_FACEZOOM_ENABLE
#include <module_face_zoom.h>
#endif

#define TAG "KIVA"
/*
#define KEY_INVALID 	-1
#define KEY_LOW_VALUE    0  
#define KEY_HIGH_VALUE 	 1
#define KEY_MODE_NUM 	 3

#define KEY_BOARD_DRV_NAME "/dev/input/event1"

#define TAG_KEY "KEY"


enum{
//按键模式
	KEY_MODE_SINGLE,
	KEY_MODE_MANY,
	KEY_MODE_NORMAL,
//gpio控制	
	GPIO_PB00 = 32,
	GPIO_PB01,
	GPIO_PB02,
	GPIO_PB03,
	GPIO_PB04,
	GPIO_PB05,
//开关状态	
	ENABLE = 0,
	DISABLE,
};

static sem_t ucam_ready_sem;

int key_type = 0; //关流时，键值复位
*/


static sem_t ucam_ready_sem;

static int kiva_generate_version()
{
	FILE *fp = NULL;
	int ret = -1;
	static char Kiva_version[LABEL_LEN] = KIVA_VERSION;
	if ((fp = fopen(KIVA_VERSION_PATH, "w+")) == NULL) {
		printf("ERROR(%s): open config file failed !\n", TAG);
		return -1;
	}

	ret = fwrite(Kiva_version, 1, strlen(Kiva_version), fp);
	if(ret != strlen(Kiva_version))
	{
		printf("ERROR(%s): write  config file failed !\n", TAG);
		fclose(fp);
		return -1;
	}
	fclose(fp);

	printf("INFO(%s): kiva version = %s \n", TAG, KIVA_VERSION);
	return 0;
}

static void auto_deinit()
{
	kiva_load_config_deinit();

#ifdef MODULE_TRACK_ENABLE
	module_track_deinit();
#endif
	sample_system_exit();
	module_autofocus_deinit();
	//module_led_deinit();
	module_ucamera_deinit();
	kiva_process_deinit();
}


static void signal_handler(int signum) {
	if (signum == SIGUSR1)
		sem_post(&ucam_ready_sem);
	else if(signum == SIGUSR2) {
		printf("INFO(%s): Ucamera Exit Now. \n", TAG);
		auto_deinit();
	}
}

static void signal_init()
{
	sem_init(&ucam_ready_sem, 0, 0);
	signal(SIGUSR1, signal_handler); /*set signal handler*/
	signal(SIGUSR2, signal_handler); /*set signal handler*/
	signal(SIGTERM, signal_handler); /*set signal handler*/
	signal(SIGKILL, signal_handler); /*set signal handler*/
}

static void *ucam_impsdk_init_entry(void *res)
{
	sem_wait(&ucam_ready_sem);

	sample_system_init(&g_func_param.isp_attr);

	module_ucamera_enable_impinited();
#ifdef MODULE_FACEZOOM_ENABLE
	module_facezoom_enable_impinited(1);
#endif

	return NULL;
}

static int ucamera_system_init()
{
	signal_init();
	/* imp system init */

	int ret = 0;
	//ucam_impsdk_init_entry(NULL);
	pthread_t ucam_impsdk_init_id;
	ret = pthread_create(&ucam_impsdk_init_id, NULL, ucam_impsdk_init_entry, NULL);
	if (ret != 0) {
		Ucamera_LOG("pthread_create failed \n");
		return -1;
	}
	return 0;

}

int sample_get_ucamera_streamonoff()
{
	return module_ucamera_stream_on();
}

int module_uvc_control_get_status(void)
{
	return module_ucamera_stream_on();
}

int sample_get_ucamera_af_onoff()
{
	return module_ucamera_get_focus_auto();
}

int sample_get_ucamera_af_cur()
{
	return module_ucamera_get_focus_cur();
}

static int module_init()
{
	int ret = -1;

	/*
	 * module_led
	 **/
/*	printf("INFO(%s): module_led_init ...\n", TAG);
	ret = module_led_init(&g_func_param.led_ctl);
	if (ret < 0) {
		printf("ERROR(%s): module_led_init failed \n", TAG);
		return -1;
	}
	printf("INFO(%s): module_led_init ...ok\n", TAG);
*/
	/*
	 * module_ucamera
	 **/
	printf("INFO(%s): module_ucamera_init ...\n", TAG);
	ret = module_ucamera_init(&g_func_param);
	if (ret < 0) {
		printf("ERROR(%s): module_ucamera_init failed \n", TAG);
		return -1;
	}
	printf("INFO(%s): module_ucamera_init ...ok\n", TAG);

	/*
	 * module_af
	 **/
	printf("INFO(%s): module_autofocus_init ...\n", TAG);
	ret = module_autofocus_init(&g_func_param.af_param);
	if (ret < 0) {
		printf("ERROR(%s): module_autofocus_init failed \n", TAG);
		return -1;
	}
	printf("INFO(%s): module_autofocus_init ...ok\n", TAG);

#ifdef MODULE_TRACK_ENABLE
	printf("INFO(%s): module track init...\n", TAG);
	ret = module_track_init(&g_func_param.motor_track_param);
	if (ret < 0) {
		printf("ERROR(%s): module_track_init failed \n", TAG);
		return -1;
	}
	printf("INFO(%s): module track init...ok\n", TAG);
#endif


#ifdef MODULE_FACEAE_ENABLE
	/*
	 * module_faceae TDDO:
	 **/
	printf("INFO(%s): module faceae init...\n", TAG);
	ret = module_faceAE_init();
	if (ret < 0) {
		printf("ERROR(%s): module_faceae_init failed \n", TAG);
		return -1;
	}
	printf("INFO(%s): module faceae init...ok\n", TAG);
#endif

	return 0;

}

//gpio 34 ########################
/*static int input_subsys_init(void)*/
/*{*/
	/*int fd = -1;*/
	/*fd = open(KEY_BOARD_DRV_NAME, O_RDWR);*/
	/*if (fd == -1) {*/
		/*printf("ERROR(%s): open (%s) error\n",TAG, KEY_BOARD_DRV_NAME);*/
		/*return -1;*/
	/*}*/

	/*return fd;*/
/*}*/

/*typedef struct _key_process{*/
	/*int lastKeyEvent;*/
	/*uint64_t oldKeyReleaseTime;*/
	/*uint64_t lastKeyReleaseTime;*/

/*}key_process_t;*/

/*static key_process_t key_process;*/

/*static uint64_t gettimestamp()*/
/*{*/
	/*struct timeval curtime;*/
	/*gettimeofday(&curtime, NULL);*/

	/*return (uint64_t)((uint64_t)curtime.tv_sec*1000000 + curtime.tv_usec);*/
/*}*/

/*static void key_mode_process(int key_type)*/
/*{*/
	/*switch(key_type){*/
			/*case KEY_MODE_SINGLE:*/
				/*[>printf("[%s]mode_success :%d \n",TAG_KEY,key_type);<]*/
				/*printf("[%s]%s: single success !\n",TAG_KEY,__func__);*/
				/*set_face_zoom_mode(1); //模式1*/
				/*set_face_zoom_switch(ENABLE);//算法开*/
				/*break;*/
			/*case KEY_MODE_MANY:*/
				/*[>printf("[%s]mode_success :%d \n",__func__,key_type);<]*/
				/*printf("[%s]%s: many success !\n",TAG_KEY,__func__);*/
				/*set_face_zoom_mode(2);//模式2*/
				/*break;*/
			/*case KEY_MODE_NORMAL:*/
				/*[>printf("[%s]mode_success :%d \n",__func__,key_type);<]*/
				/*printf("[%s]%s: normal success !\n",TAG_KEY,__func__);*/
				/*set_face_zoom_switch(DISABLE);//关闭算法*/
				/*break;*/
			/*default:*/
				/*printf("[%s]%s: set error %d!\n",TAG_KEY,__func__,key_type);*/
				/*break;*/
		/*}*/
/*}*/

/*static void *key_monitor_thread(void *arg)*/
/*{*/
	/*int ret = 0;*/
	/*int fd = -1;*/
	/*struct input_event buttons_event;*/

	/*fd = input_subsys_init();*/
	/*prctl(PR_SET_NAME, "key_monitor_thread");*/
	/*printf("[%s]%s success !\n",TAG_KEY,__func__);*/
	/*while(1){*/
		/*memset(&buttons_event, 0 ,sizeof(struct input_event));*/
		/*ret = read(fd, &buttons_event, sizeof(struct input_event));*/
		/*if(ret < 0){*/
			/*printf("WARNNING(%s):read error \n", TAG);*/
			/*usleep(100*1000);*/
			/*continue;*/
		/*}*/
		/*if(buttons_event.type != EV_KEY)*/
			/*continue;*/

		/*switch(buttons_event.value){*/
			/*case KEY_LOW_VALUE:*/
				/*if(module_ucamera_stream_on()){*/
				/*key_process.lastKeyEvent = DISABLE;*/
				/*}*/
				/*break;*/
			/*case KEY_HIGH_VALUE:*/
				/*if(key_process.lastKeyEvent == DISABLE){*/
					/*key_process.lastKeyEvent = ENABLE;*/
					/*key_process.lastKeyReleaseTime = gettimestamp();*/
				/*}*/
				/*break;*/
			/*default:*/
				/*break;*/
		/*}*/
	/*}*/

	/*close(fd);*/

	/*return NULL;*/
/*}*/

/*static void *key_process_thread(void *arg)*/
/*{*/
	/*prctl(PR_SET_NAME, "key_process_thread");*/
	/*printf("[%s]%s success !\n",TAG_KEY,__func__);*/
	/*while(1){*/
		/*//按键按下时*/
		/*if(key_process.lastKeyEvent == ENABLE){*/
			/*key_process.oldKeyReleaseTime = key_process.lastKeyReleaseTime;*/
			/*sleep(1);*/
			/*if(key_process.lastKeyReleaseTime != key_process.oldKeyReleaseTime){*/
				/*key_process.oldKeyReleaseTime = key_process.lastKeyReleaseTime;*/
			/*}else{*/
			/*++key_type;*/
			/*if(key_type == KEY_MODE_NUM){*/
				/*key_type = KEY_MODE_SINGLE;*/
			/*}*/
			/*key_mode_process(key_type);*/
			/*key_process.lastKeyEvent = KEY_INVALID;*/
			/*key_process.oldKeyReleaseTime = key_process.lastKeyReleaseTime;*/
			/*}*/
		/*}else{ */
			/*usleep(100*1000);*/
		/*}*/
	/*}*/

	/*return NULL;*/
/*}*/
/*//####################################*/

/*//gpio 32 33 #########################*/

/*void key_init(void)*/
/*{*/
	/*int ret  = 0;*/
	/*ret = module_key_in_init(GPIO_PB00); //PB00端口初始化 视频开关*/
	/*if(ret < 0){*/
			/*printf("ERROR(%s):%s PB00 init error \n", TAG_KEY,__func__);*/
			/*return;*/
		/*}*/
	/*ret = module_key_in_init(GPIO_PB01); //PB01端口初始化 音频开关*/
	/*if(ret < 0){*/
			/*printf("ERROR(%s):%s PB01 init error \n", TAG_KEY,__func__);*/
			/*return;*/
		/*}*/
	/*ret = module_key_out_init(GPIO_PB03,DISABLE); //PB03端口初始化 音频灯开关*/
	/*if(ret < 0){*/
			/*printf("ERROR(%s):%s PB03 init error \n",TAG_KEY,__func__);*/
			/*return;*/
		/*}*/
	/*ret = module_key_out_init(GPIO_PB04,DISABLE); //PB04端口初始化 图像正常灯开关*/
	/*if(ret < 0){*/
			/*printf("ERROR(%s):%s PB04 init error \n", TAG_KEY,__func__);*/
			/*return;*/
		/*}*/
	/*ret = module_key_out_init(GPIO_PB05,DISABLE); //PB05端口初始化 图像灯开关*/
	/*if(ret < 0){*/
			/*printf("ERROR(%s):%s PB05 init error \n", TAG_KEY,__func__);*/
			/*return;*/
		/*}*/
		/*printf("[%s]%s success!\n",TAG_KEY,__func__);*/
	/*return;*/
/*}*/

/*void set_stream_switch(int on_off)*/
/*{*/
	/*if(get_frame_fmt() != 1){*/
	/*//set osd attr*/
	/*IMPOSDRgnAttr rAttrRect_type_1;*/
	/*IMPRgnHandle *han_data = prHander_interface();*/
	/*memset(&rAttrRect_type_1,0,sizeof(IMPOSDRgnAttr));*/
	/*//获取rgn属性			*/
				/*if(IMP_OSD_GetRgnAttr(han_data[0],&rAttrRect_type_1) != 0){*/
					/*printf("[%s][%d]IMP_OSD_GETRGNATTR error\n",__func__,__LINE__);*/
					/*return;*/
				/*}*/
				/*rAttrRect_type_1.rect.p1.x = get_osd_width() - 1;*/
				/*rAttrRect_type_1.rect.p1.y = get_osd_height() - 1;*/
				/*if(IMP_OSD_SetRgnAttr(han_data[0], &rAttrRect_type_1) != 0){*/
						/*printf("[%s][%d]IMP_OSD_SETRGNATTR error\n",__func__,__LINE__);*/
						/*return;*/
					/*}	*/
	/*//osd show*/
	/*if (IMP_OSD_ShowRgn(han_data[0], 0, on_off) != 0) {*/
		/*IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn() timeStamp error\n");*/
		/*return;*/
	/*}*/
	/*han_data = NULL;*/
	/*}else{*/
		/*//set_osd_ivs_switch(on_off);*/
		/*set_yuyv_switch(on_off);*/
	/*}*/
	/*return;*/
/*}*/

/*static void *key_stir_thread(void *arg)*/
/*{*/
	/*int frame_on_off = 0;*/
	/*int audio_on_off = 0;*/
	/*prctl(PR_SET_NAME, "key_stir_thread");*/
	/*key_init();*/
	/*while(1){*/
		/*if(module_ucamera_stream_on()){*/
			/*if(!module_key_ctl_read(GPIO_PB00)){*/
				/*if(frame_on_off == ENABLE){*/
					/*frame_on_off = DISABLE;*/
					/*//关图像*/
					/*set_stream_switch(DISABLE);*/
					/*set_audio_switch(ENABLE);*/
					/*printf("[%s]%s off frame success!\n",TAG_KEY,__func__);*/
				/*}*/
					/*//图像灯*/
					/*module_key_ctl_write(GPIO_PB05,ENABLE);*/
					/*module_key_ctl_write(GPIO_PB03,DISABLE);*/
					/*module_key_ctl_write(GPIO_PB04,DISABLE);*/
					/*usleep(100*1000);*/
/*#ifdef DBUG*/
					/*printf("[%s]%s frame led on success!\n",TAG_KEY,__func__);*/
/*#endif*/
			/*}else if(!module_key_ctl_read(GPIO_PB01)){*/
				/*if(audio_on_off == ENABLE){*/
					/*audio_on_off = DISABLE;*/
					/*//关音频*/
					/*set_audio_switch(DISABLE);*/
					/*set_stream_switch(ENABLE);*/
					/*printf("[%s]%s off audio success!\n",TAG_KEY,__func__);*/
				/*}*/
					/*//音频灯*/
					/*module_key_ctl_write(GPIO_PB03,ENABLE);*/
					/*module_key_ctl_write(GPIO_PB05,DISABLE);*/
					/*module_key_ctl_write(GPIO_PB04,DISABLE);*/
					/*usleep(100*1000);*/
/*#ifdef DBUG*/
					/*printf("[%s]%s audio led on success!\n",TAG_KEY,__func__);*/
/*#endif*/
			/*}else{*/
				/*if(frame_on_off == DISABLE || audio_on_off == DISABLE){*/
					/*frame_on_off = ENABLE;*/
					/*//开图像*/
					/*set_stream_switch(ENABLE);*/
					/*printf("[%s]%s on frame success!\n",TAG_KEY,__func__);*/
							
					/*audio_on_off = ENABLE;*/
					/*//开音频*/
					/*set_audio_switch(ENABLE);*/
					/*printf("[%s]%s on audio success!\n",TAG_KEY,__func__);*/
				/*}*/
				/*//关音频灯*/
				/*module_key_ctl_write(GPIO_PB03,DISABLE);*/
				/*//关图像灯*/
				/*module_key_ctl_write(GPIO_PB05,DISABLE);*/
				/*//开正常图像灯*/
				/*module_key_ctl_write(GPIO_PB04,ENABLE);*/
				/*usleep(100*1000);*/
			/*}*/
		/*}else{*/
			/*usleep(100*1000);*/
			/*//关音频灯*/
			/*module_key_ctl_write(GPIO_PB03,DISABLE);*/
			/*//关图像灯*/
			/*module_key_ctl_write(GPIO_PB05,DISABLE);*/
			/*//关正常图像灯*/
			/*module_key_ctl_write(GPIO_PB04,DISABLE);*/
			/*//关音频*/
			/*if(audio_on_off == ENABLE){*/
			/*set_audio_switch(DISABLE);*/
			/*audio_on_off = DISABLE;*/
			/*}*/
		/*}*/
	/*}*/
	/*return NULL;*/
/*}*/
/*//####################################*/
/*static int module_key_process(void)*/
/*{*/
	/*int ret = 0;*/
	/*key_process.lastKeyEvent = KEY_INVALID;*/

	/*static pthread_t key_monitor_pid;*/
	/*pthread_attr_t key_monitor_attr;*/
	/*pthread_attr_init(&key_monitor_attr);*/
	/*pthread_attr_setdetachstate(&key_monitor_attr, PTHREAD_CREATE_DETACHED);*/
	/*pthread_attr_setschedpolicy(&key_monitor_attr, SCHED_OTHER);*/
	/*ret = pthread_create(&key_monitor_pid, &key_monitor_attr, &key_monitor_thread, NULL);*/
	/*if (ret) {*/
		/*printf("ERROR(%s): create thread for key_monitor_thread failed!\n", TAG);*/
		/*return -1;*/
	/*}*/

	/*static pthread_t key_process_pid;*/
	/*pthread_attr_t key_process_attr;*/
	/*pthread_attr_init(&key_process_attr);*/
	/*pthread_attr_setdetachstate(&key_process_attr, PTHREAD_CREATE_DETACHED);*/
	/*pthread_attr_setschedpolicy(&key_process_attr, SCHED_OTHER);*/
	/*ret = pthread_create(&key_process_pid, &key_process_attr, &key_process_thread, NULL);*/
	/*if (ret) {*/
		/*printf("ERROR(%s): create thread for key_process_thread failed!\n", TAG);*/
		/*return -1;*/
	/*}*/

	/*static pthread_t key_stir_pid;*/
	/*pthread_attr_t key_stir_attr;*/
	/*pthread_attr_init(&key_stir_attr);*/
	/*pthread_attr_setdetachstate(&key_stir_attr, PTHREAD_CREATE_DETACHED);*/
	/*pthread_attr_setschedpolicy(&key_stir_attr, SCHED_OTHER);*/
	/*ret = pthread_create(&key_stir_pid, &key_stir_attr, &key_stir_thread, NULL);*/
	/*if (ret) {*/
		/*printf("ERROR(%s): create thread for key_stir_thread failed!\n", TAG);*/
		/*return -1;*/
	/*}*/
	/*return 0;*/
/*}*/

/* ---------------------------------------------------------------------------
 * main
 */

int main(int argc, char *argv[])
{
	int ret;

	/*Step1: anticopy*/
	printf("INFO(%s): AntiCopy_Verify ...\n", TAG);
	ret = AntiCopy_Verify();
	if (ret < 0) {
		printf("ERROR(%s): AntiCopy Verified failed!\n", TAG);
		return -1;
	}
	printf("INFO(%s): AntiCopy_Verify ...ok\n", TAG);

	/*Step2: load config */
	printf("INFO(%s): kiva_load_config_init ...\n", TAG);
	ret = kiva_load_config_init();
	if (ret < 0) {
		printf("ERROR(%s): ucamera load config failed!\n", TAG);
		return -1;
	}
	printf("INFO(%s): kiva_load_config_init ...ok\n", TAG);

	/*Step3: kiva_generate_version */
	ret = kiva_generate_version();
	if (ret < 0) {
		printf("ERROR(%s): kiva generate version failed!\n", TAG);
		return -1;
	}

	/*Step4: */
	printf("INFO(%s): ucamera_system_init ...\n", TAG);
	ret = ucamera_system_init();
	if (ret < 0) {
		printf("ERROR(%s): ucamera_system_init failed \n", TAG);
		return -1;
	}
	printf("INFO(%s): ucamera_system_init ...ok\n", TAG);

	/*Step5: */
	ret = module_init();
	if (ret < 0) {
		printf("ERROR(%s): module_init failed \n", TAG);
		return -1;
	}

	/*
	 * other thread process
	 **/
	printf("INFO(%s): kiva_process_init ...\n", TAG);
	ret = kiva_process_init(&g_func_param);
	if (ret < 0) {
		auto_deinit();
	}
	printf("INFO(%s): kiva_process_init ...ok\n", TAG);
	
	module_key_process();
	while (1) {
		sleep(60);

	}
	return 0;
}
