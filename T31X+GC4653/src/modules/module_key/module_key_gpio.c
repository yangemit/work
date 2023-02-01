#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#include <imp/imp_osd.h>
#include <imp/imp_isp.h>
#include <imp-common.h>

#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include <semaphore.h>

#define TAG_KEY "KEY_GPIO"

/* 静态初始化创建互斥锁 */
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* 图标长显标志 */
volatile int osd_long_flag = 0;
volatile int osd_type = 0;

/* 黑图显示osd 图片标志 */
volatile int flag_osd_black = 0;

/* 信号对象 */
sem_t sem_osd;
sem_t sem_led;
sem_t sem_osd_long;

/*
	算法模式
	1：单人模式
	2：多人模式
	3：常规模式
*/
#define KEY_MODE_NUM     3

/*
	选择的I2C 控制器
*/
#define KEY_BOARD_DRV_NAME  "/dev/i2c-1"
/*
	I2C 设备的地址
*/
#define I2C_ADDR  0x34

/*
	png 图像调整后的长宽
*/
int x_out = 0;
int y_out = 0;
/*
	png - argb 数据储存
*/
char * buff_ARGB = NULL;
/*
	png 图像路径 结构体
*/
typedef struct _osd_sum {
	char const *png_frame[17];
}osd_sum;

osd_sum osd_sum_png = {
	.png_frame[0] = "/system/etc/png/1.png",
	.png_frame[1] = "/system/etc/png/2.png",
	.png_frame[2] = "/system/etc/png/3.png",
	.png_frame[3] = "/system/etc/png/4.png",
	.png_frame[4] = "/system/etc/png/5.png",
	.png_frame[5] = "/system/etc/png/6.png",
	.png_frame[6] = "/system/etc/png/7.png",
	.png_frame[7] = "/system/etc/png/8.png",
	.png_frame[8] = "/system/etc/png/9.png",
	.png_frame[9] = "/system/etc/png/10.png",
	.png_frame[10] = "/system/etc/png/11.png",
	.png_frame[11] = "/system/etc/png/12.png",
	.png_frame[12] = "/system/etc/png/13.png",
	.png_frame[13] = "/system/etc/png/14.png",
	.png_frame[14] = "/system/etc/png/15.png",
	.png_frame[15] = "/system/etc/png/16.png",
	.png_frame[16] = "/system/etc/png/17.png",
};

/*
	sensor 的宽高
*/
int g_VideoWidth = 2560;
int g_VideoHeight = 1440;
/*
	同步pantilt 改变裁剪的原点位置进行同步标志
*/
int first_set_flag = 0;
/*
	osd 显示控制开关
	0：关
	1：显示
*/
volatile int osd_switsh = 0;
volatile int osd_long_switsh = 0;

/*
	zoom 与 pantilt 数据交互结构体
*/
typedef struct _crop_obj {
	int left;
	int top;
	int w;
	int h;
} crop_obj_t;

crop_obj_t zoom_crop;

/*
	集合变量
*/
typedef struct _key_info {
	volatile int frame_on_off ;
	int audio_on_off ;

	/* 亮度 */
	int bright_value ;
	/* 清晰度 */
	int sharp_value ;
	/* 缩放 */
	int zoom_value ;
	/* 左右 */
	int pan_value ;
	/* 上下 */
	int tilt_value ;
	/* pantilt 使能b标志*/
	int pantilt_flag;
	/* osd图片标号 */
	volatile int png_num;
} key_info;
/*
	zoom 与 pt 改变的左上角坐标
*/
static float  pan_factor;
static float  tilt_factor;

int key_type = 0; //关流时，键值复位(thread_process.c)

enum{
//按键模式
	KEY_MODE_SINGLE,
	KEY_MODE_MANY,
	KEY_MODE_NORMAL,
//gpio控制
//	GPIO_PB00 = 32,
//	GPIO_PB01,
//	GPIO_PB02,
	GPIO_PB17 = 49,
	//GPIO_PB18,
	GPIO_PB27 = 59,
//开关状态
	ENABLE = 0,
	DISABLE,
};

//################ 算法控制 ###########################

/*
	算法模式
	单人模式: 1
	多人模式: 2
*/
extern void set_face_zoom_mode(int mode);
/*
	算法开关
	ENABLE :开
	DISABLE :关
*/
extern void set_face_zoom_switch(int on_off);
//##################### END AI #####################


//##################### OSD #####################
/*
	获取图像格式
*/
extern int get_frame_fmt(void);
/*
	获取OSD 句柄
*/
extern IMPRgnHandle *prHander_interface(void);
/*
	pc 输出的 宽
*/
extern int get_osd_width(void);
/*
	pc 输出的 高
*/
extern int get_osd_height(void);
/*
	pc 出流开关
	1: 开流
	0：关流
*/
extern int get_frame_switch(void);
/*
	osd yuyv 显示黑图
	1 :正常出图
	0 :显示黑图
*/
extern void set_yuyv_switch(int on_off);
/*
	音频开关
	ENABLE :开
	DISABLE :关
*/
extern void set_audio_switch(int on_off);
//##################### END OSD #####################

//##################### gpio control #######################

//######sys/class/gpio/  apply

//--------------------------------->gpio 输入配置
/*static int key_in_init(int gpio_in)//, int value)*/
/*{*/
	/*char direction_path[64] = {0};*/
	/*FILE *p = NULL;*/

	/*[>[> 申请GPIO	<]<]*/
	/*p = fopen("/sys/class/gpio/export","w");*/
	/*if (!p)*/
		/*return -1;*/
	/*fprintf(p,"%d",gpio_in);*/
	/*fclose(p);*/

	/*sprintf(direction_path, "/sys/class/gpio/gpio%d/direction", gpio_in);*/


	/*p = fopen(direction_path, "w");*/
	/*if (!p)*/
		/*return -1;*/
	/*[>[> 设置输入方式<]<]*/
	/*fprintf(p, "in");*/
	/*fclose(p);*/

	/*return 0;*/
/*}*/
//读取数据
/*static int read_gpio_ctl(int gpio_in)*/
/*{*/
	/*char value_path[64] = {0};*/
	/*int value = 0;*/
	/*FILE *p = NULL;*/

	/*sprintf(value_path, "/sys/class/gpio/gpio%d/value", gpio_in);*/

	/*p = fopen(value_path, "r");*/
	/*if (!p) {*/
		/*printf("ERROR(%s): fopen %s failed \n", TAG_KEY, value_path);*/
		/*return -1;*/
	/*}*/
	/*fscanf(p,"%d", &value);*/
	/*fclose(p);*/
/*#ifdef  DBUG*/
	/*printf("[%s]gpio_value:%d\n",__func__,value);*/
/*#endif*/
	/*return value;*/
/*}*/
//->函数封装
/*int module_key_ctl_read(int gpio_in)*/
/*{	int gpio_value = 0;*/
	/*gpio_value = read_gpio_ctl(gpio_in);*/

	/*return gpio_value; */
/*}*/

/*int module_key_in_init(int gpio_in)*/
/*{*/
		/*return key_in_init(gpio_in);*/
/*}*/
//<-函数封装 END

//<----------------------------------gpio 输入配置  END

//--------------------------->gpio 输出配置
static int key_out_init(int gpio_out,int value_out)
{
	char direction_path[64] = {0};
	char value_path[64] = {0};
	FILE *p = NULL;

	//[> 申请gpio	<]
	p = fopen("/sys/class/gpio/export","w");
	if (!p)
		return -1;
	fprintf(p,"%d",gpio_out);
	fclose(p);

	sprintf(direction_path, "/sys/class/gpio/gpio%d/direction", gpio_out);
	sprintf(value_path, "/sys/class/gpio/gpio%d/value", gpio_out);

	p = fopen(direction_path, "w");
	if (!p)
		return -1;
	//[>设置为输出<]
	fprintf(p, "out");
	fclose(p);

	p = fopen(value_path, "w");
	if (!p)
		return -1;
	//[>输出值<]
	fprintf(p, "%d", value_out);
	fclose(p);

	return 0;
}
//改变gpio电平
static void key_ctl_write(int gpio_out, int value_out)
{
	char value_path[64] = {0};
	FILE *p = NULL;

	sprintf(value_path, "/sys/class/gpio/gpio%d/value", gpio_out);

	p = fopen(value_path, "w");
	if (!p) {
		printf("ERROR(%s): fopen %s failed \n", TAG_KEY, value_path);
		return;
	}
	fprintf(p,"%d", value_out);
	fclose(p);
}
//->函数封装
void module_key_ctl_write(int gpio_out, int value_out)
{

	key_ctl_write(gpio_out, value_out);
}


int module_key_out_init(int gpio_out,int value_out)
{
		return key_out_init(gpio_out,value_out);
}
//<-函数封装 END
//<-------------------------------- gpio 输出配置 END

/*
	led 灯初始化
*/
static void key_init(void)
{
	int ret  = 0;
	ret = module_key_out_init(GPIO_PB17,DISABLE); //PB17端口初始化 电源灯开关---红灯
	if(ret < 0){
			printf("ERROR(%s):%s PB03 init error \n",TAG_KEY,__func__);
			return;
		}
	ret = module_key_out_init(GPIO_PB27,DISABLE); //PB27端口初始化 图像灯开关---蓝灯
	if(ret < 0){
			printf("ERROR(%s):%s PB05 init error \n", TAG_KEY,__func__);
			return;
		}
		printf("[%s]%s success!\n",TAG_KEY,__func__);
	return;
}

/*
	osd mjpeg 显示黑图
*/
static void set_stream_switch(int on_off)
{
	if(get_frame_fmt() != 1){
	//set osd attr
	IMPOSDRgnAttr rAttrRect_type_1;
	IMPRgnHandle *han_data = prHander_interface();
	memset(&rAttrRect_type_1,0,sizeof(IMPOSDRgnAttr));
	//获取rgn属性
				if(IMP_OSD_GetRgnAttr(han_data[0],&rAttrRect_type_1) != 0){
					printf("ERROR(%s)[%d]IMP_OSD_GETRGNATTR error\n",TAG_KEY,__LINE__);
					return;
				}
				rAttrRect_type_1.rect.p1.x = get_osd_width() - 1;
				rAttrRect_type_1.rect.p1.y = get_osd_height() - 1;
				if(IMP_OSD_SetRgnAttr(han_data[0], &rAttrRect_type_1) != 0){
						printf("INFO(%s)[%d]IMP_OSD_SETRGNATTR error\n",TAG_KEY,__LINE__);
						return;
					}
	//osd show
	if (IMP_OSD_ShowRgn(han_data[0], 0, on_off) != 0) {
		printf("INFO(%s)[%d]IMP_OSD_ShowRgn error\n",TAG_KEY,__LINE__);
		return;
	}
	han_data = NULL;
	}else{
		//set_osd_ivs_switch(on_off);
		set_yuyv_switch(on_off);
	}
	return;
}

/*
	ARGB数据存储到osd_buff
*/
char *set_ARGB_buff(char const *file,int x,int y)
{
	FILE *fp;
	char *osd_buff = NULL;
	int size = 0;

	/*
		调整png 图像的合适的宽-x_out高-y_out
		第一个参数与第二个参数越大 osd图像越小
	*/
	Stretch_pic(2560, 1440, x, y, &x_out, &y_out);
	/*
		实现png 图像大小调整
		将调整好的图片放在/tmp/tmp.png
	*/
	resize_png(file, x_out, y_out);
	/*
		将png->argb,argb文件放在/tmp/ARGB
	*/
	load_png_image("/tmp/tmp.png");
	//二进制方式打开文件
	fp = fopen("/tmp/ARGB", "rb");
	if(NULL == fp)
	{
		printf("Error:Open %s file fail!\n",file);
		return NULL;
	}

	//求得文件的大小
	fseek(fp, 0, SEEK_END);
	size=ftell(fp);
	/*
		释放上一次osd开的buff
	*/
	if ( buff_ARGB != NULL ) {
		free(buff_ARGB);
		buff_ARGB = NULL;
		//printf("[INFO] osd_buff free success !!!\n");
	} else {
		//printf("[INFO] osd_buff NULL\n");
	}
	osd_buff = malloc(size);
	//printf("#####size %d ######\n",size);
	rewind(fp);

	fread(osd_buff, 1, size, fp);
	if ( ferror(fp) ) {
		printf("[INFO] read ARGB data failed \n");//每次读一个，共读size次
	}
	fclose(fp);

	return osd_buff;
}

/*
	osd 图像叠加
	用于按键切换，提示作用
 	on_off 1:开    on_off 0:关
	num 表示是哪一个区
	png_frame_num:第几个要显示的png图像
*/
static void set_osd_show(volatile int *osd_on_off,int num,volatile int png_frame_num)
{
	int on_off = *osd_on_off;
	if(get_frame_fmt() != 1){
		//set osd attr
		IMPOSDRgnAttr rAttrRect_type_2;
		IMPOSDGrpRgnAttr rAttrRect_type_2_grp;
		IMPRgnHandle *han_data = prHander_interface();
		memset(&rAttrRect_type_2,0,sizeof(IMPOSDRgnAttr));
		memset(&rAttrRect_type_2_grp,0,sizeof(IMPOSDRgnAttr));
		if ( png_frame_num > 0 ) {
			char const *png_frame_tmp = osd_sum_png.png_frame[png_frame_num-1];
			//printf("[INFO] png_frame_num %s \n",png_frame_tmp);
			//获取rgn属性
			/*
			   显示不同分辨率的时候
			   会出现图像变小的时候
			   osd图像确没有变动
			   显得osd变大了一样
			   解决:
			   通过 osd图像大小/pic图像大小最大  =  osd最终显示的大小/pic图像的大小
			   例如：
			   90 / 2560 = x / 800  (图像的宽)；
			   参考 Stretch_pic 函数
			   */
			buff_ARGB = set_ARGB_buff(png_frame_tmp,get_osd_width(),get_osd_height());


			if ( buff_ARGB == NULL ) {
				printf("[INFO] NULL buff\n");
				return;
			}

			if(IMP_OSD_GetRgnAttr(han_data[num],&rAttrRect_type_2) != 0){
				printf("ERROR(%s)[%d]IMP_OSD_GETRGNATTR error\n",TAG_KEY,__LINE__);
				return;
			}
			rAttrRect_type_2.type = OSD_REG_PIC;
			/*
			   右上角显示
			*/

			if ( png_frame_num == 5 ) {
				rAttrRect_type_2.rect.p0.x = get_osd_width()/2-x_out/2;
				rAttrRect_type_2.rect.p0.y = 0;
				rAttrRect_type_2.rect.p1.x = rAttrRect_type_2.rect.p0.x+x_out-1;
				rAttrRect_type_2.rect.p1.y = rAttrRect_type_2.rect.p0.y+y_out-1;
				rAttrRect_type_2.fmt = PIX_FMT_ARGB;
				rAttrRect_type_2.data.picData.pData = buff_ARGB;

			}else if (png_frame_num == 6){
				rAttrRect_type_2.rect.p0.x = get_osd_width()/2-x_out/2;
				rAttrRect_type_2.rect.p0.y = get_osd_height()-y_out;
				rAttrRect_type_2.rect.p1.x = rAttrRect_type_2.rect.p0.x+x_out-1;
				rAttrRect_type_2.rect.p1.y = rAttrRect_type_2.rect.p0.y+y_out-1;
				rAttrRect_type_2.fmt = PIX_FMT_ARGB;
				rAttrRect_type_2.data.picData.pData = buff_ARGB;

			}else if ( png_frame_num == 8 ) {
				rAttrRect_type_2.rect.p0.x = 0;
				rAttrRect_type_2.rect.p0.y = get_osd_height()/2-y_out/2;
				rAttrRect_type_2.rect.p1.x = rAttrRect_type_2.rect.p0.x+x_out-1;
				rAttrRect_type_2.rect.p1.y = rAttrRect_type_2.rect.p0.y+y_out-1;
				rAttrRect_type_2.fmt = PIX_FMT_ARGB;
				rAttrRect_type_2.data.picData.pData = buff_ARGB;

			} else if ( png_frame_num == 7 ) {
				rAttrRect_type_2.rect.p0.x = get_osd_width()-x_out;
				rAttrRect_type_2.rect.p0.y = get_osd_height()/2-y_out/2;
				rAttrRect_type_2.rect.p1.x = rAttrRect_type_2.rect.p0.x+x_out-1;
				rAttrRect_type_2.rect.p1.y = rAttrRect_type_2.rect.p0.y+y_out-1;
				rAttrRect_type_2.fmt = PIX_FMT_ARGB;
				rAttrRect_type_2.data.picData.pData = buff_ARGB;
				/*printf("[INFO] p0x=%d p0y=%d p1x=%d p1y=%d \n",*/
				/*rAttrRect_type_2.rect.p0.x,rAttrRect_type_2.rect.p0.y,*/
				/*rAttrRect_type_2.rect.p1.x,rAttrRect_type_2.rect.p1.y);*/
			} else if ( png_frame_num == 9 ) {
				rAttrRect_type_2.rect.p0.x = 0;
				rAttrRect_type_2.rect.p0.y = 0;
				rAttrRect_type_2.rect.p1.x = rAttrRect_type_2.rect.p0.x+x_out-1;
				rAttrRect_type_2.rect.p1.y = rAttrRect_type_2.rect.p0.y+y_out-1;
				rAttrRect_type_2.fmt = PIX_FMT_ARGB;
				rAttrRect_type_2.data.picData.pData = buff_ARGB;
			} else if ( png_frame_num == 10 ) {
				rAttrRect_type_2.rect.p0.x = 0;
				rAttrRect_type_2.rect.p0.y = 0;
				rAttrRect_type_2.rect.p1.x = rAttrRect_type_2.rect.p0.x+x_out-1;
				rAttrRect_type_2.rect.p1.y = rAttrRect_type_2.rect.p0.y+y_out-1;
				rAttrRect_type_2.fmt = PIX_FMT_ARGB;
				rAttrRect_type_2.data.picData.pData = buff_ARGB;
			} else if ( png_frame_num == 11 ) {
				rAttrRect_type_2.rect.p0.x = 0;
				rAttrRect_type_2.rect.p0.y = 0;
				rAttrRect_type_2.rect.p1.x = rAttrRect_type_2.rect.p0.x+x_out-1;
				rAttrRect_type_2.rect.p1.y = rAttrRect_type_2.rect.p0.y+y_out-1;
				rAttrRect_type_2.fmt = PIX_FMT_ARGB;
				rAttrRect_type_2.data.picData.pData = buff_ARGB;
			} else {
				rAttrRect_type_2.rect.p0.x = get_osd_width()-x_out;
				rAttrRect_type_2.rect.p0.y = 0;
				rAttrRect_type_2.rect.p1.x = rAttrRect_type_2.rect.p0.x+x_out-1;
				rAttrRect_type_2.rect.p1.y = rAttrRect_type_2.rect.p0.y+y_out-1;
				rAttrRect_type_2.fmt = PIX_FMT_ARGB;
				rAttrRect_type_2.data.picData.pData = buff_ARGB;
			}

			if(IMP_OSD_SetRgnAttr(han_data[num], &rAttrRect_type_2) != 0){
				printf("INFO(%s)[%d]IMP_OSD_SETRGNATTR error\n",TAG_KEY,__LINE__);
				return;
			}

			rAttrRect_type_2_grp.show = 0;
			rAttrRect_type_2_grp.gAlphaEn = 1;
			rAttrRect_type_2_grp.fgAlhpa = 0xff;
			rAttrRect_type_2_grp.layer = 3;

			if( IMP_OSD_SetGrpRgnAttr(han_data[num], 0, &rAttrRect_type_2_grp) < 0 ) {
				printf("[INFO] SET OSD GRP FAILED !!!\n");
				return;
			}

			//osd show
			if (IMP_OSD_ShowRgn(han_data[num], 0, on_off) != 0) {
				printf("INFO(%s)[%d]IMP_OSD_ShowRgn error\n",TAG_KEY,__LINE__);
				return;
			}
			han_data = NULL;
		} else {
			//osd show
			if (IMP_OSD_ShowRgn(han_data[num], 0, on_off) != 0) {
				printf("INFO(%s)[%d]IMP_OSD_ShowRgn error\n",TAG_KEY,__LINE__);
				return;
			}
			han_data = NULL;

		}
	}else{
		//set_osd_ivs_switch(on_off);
		//set_yuyv_switch(on_off);
	}
	return;
}

/*
	音频控制
*/
static void audio_switch(key_info *flag)
{
	int test = flag->audio_on_off;

	if ( test ) {
		flag->audio_on_off = ENABLE;
		set_audio_switch(ENABLE);
		osd_switsh = 1;
		flag->png_num = 3;
		//printf("INFO(%s)[%d] on audio success!\n",TAG_KEY,__LINE__);
	} else {
		flag->audio_on_off = DISABLE;
		set_audio_switch(DISABLE);
		osd_switsh = 1;
		flag->png_num = 4;
		//printf("INFO(%s)[%d] on audio success!\n",TAG_KEY,__LINE__);
	}

	return;
}

/*
	亮度设置
*/

static void set_brightness_sharpness(int reform,int *value,int flag)
{
	int ret;
	unsigned char bright = 0;


	bright = *value & 0xff;

	if ( !reform ) {
		bright += 2;
		*value = bright;
		if ( *value <= 160 ) {
			if ( !flag ) {
				ret = IMP_ISP_Tuning_SetBrightness(bright);
				if (ret)
					printf("[INFO] set brightness failed \n");
				//printf("[INFO] ++ set brightness success !!!\n");
			} else {
				ret = IMP_ISP_Tuning_SetSharpness(bright);
				if (ret)
					printf("[INFO] set sharpness failed \n");
				//printf("[INFO] ++ set sharpness success !!!\n");
			}
		} else {
			*value = 160;
			printf("[INFO] ++ go beyond !!!\n");
		}
	} else {
		bright -= 2;
		*value = bright;
		if ( *value >= 50 ) {
			if ( !flag ) {
				ret = IMP_ISP_Tuning_SetBrightness(bright);
				if (ret)
					printf("[INFO] set brightness failed \n");
				//printf("[INFO] -- set brightness success !!!\n");
			} else {
				ret = IMP_ISP_Tuning_SetSharpness(bright);
				if (ret)
					printf("[INFO] set sharpness failed \n");
				//printf("[INFO] -- set sharpness success !!!\n");
			}
		} else {
			*value = 50;
			printf("[INFO] -- go beyond !!!\n");
		}
	}
}

/*
	缩放设置
*/

static int set_zoom(int *value)
{
	int ret;
	/*
		裁剪的长：W
		裁剪的宽：h
		裁剪的左上角顶点：（left，top）
	*/
	int w = 0 ,h = 0,left = 0,top = 0;
	/*
		裁剪的长宽以 g_VideoWidth/ ^value_cur/100 来变化(T31:100~200) （t40:100~400）
	*/
	float value_cur;
	int zoomwidth_cur,zoomheight_cur;
	value_cur = *value;

	zoomwidth_cur = round(g_VideoWidth/sqrt(value_cur/100));
	zoomwidth_cur = round((float)(zoomwidth_cur/4)) * 4;
	zoomheight_cur = round(g_VideoHeight/sqrt(value_cur/100));
	zoomheight_cur = round((float)(zoomheight_cur/4)) * 4;

	w = zoomwidth_cur;
	h = zoomheight_cur;

	/*
		裁剪的原点要位于 图像的左上角1/4的部分
	*/
	left = round((float)(g_VideoWidth -w) / 2);
	top = round((float)(g_VideoHeight - h) / 2);

	/*
		控制重开时(切换fps的时候)，出现图像位置发生改变
	*/
	if(!first_set_flag){
		left = round(1.0 * left * (1 + pan_factor));
		top = round(1.0 * top * (1 + tilt_factor));
	}

	IMPISPFrontCrop zoom_attr;
	IMP_ISP_Tuning_GetFrontCrop(&zoom_attr);
	zoom_attr.fcrop_enable = 1;
	zoom_attr.fcrop_left = left;
	zoom_attr.fcrop_top = top;
	zoom_attr.fcrop_width = w;
	zoom_attr.fcrop_height = h;
	ret = IMP_ISP_Tuning_SetFrontCrop(&zoom_attr);
	if (ret < 0) {
		printf("IMP Set Fcrop failed=%d\n",__LINE__);
		return -1;
	}
	//printf("[INFO] left[%d] top[%d] w[%d] h[%d]\n",left,top,w,h);

	/*
		保持这一次的图像改变
	*/
	zoom_crop.w = w;
	zoom_crop.h = h;
	zoom_crop.left = left;
	zoom_crop.top = top;


	return 0;
}

static void key_mode_process(int key ,key_info *pantiltzoom_flag)
{
	switch(key){
			case KEY_MODE_SINGLE:
				/*printf("[%s]mode_success :%d \n",TAG_KEY,key_type);*/
				printf("INFO(%s)%s: single success !\n",TAG_KEY,__func__);
				set_face_zoom_mode(1); //模式1
				set_face_zoom_switch(ENABLE);//算法开
				osd_long_switsh = 1;
				osd_long_flag = 9;
				break;
			case KEY_MODE_MANY:
				/*printf("[%s]mode_success :%d \n",__func__,key_type);*/
				printf("INFO(%s)%s: many success !\n",TAG_KEY,__func__);
				set_face_zoom_mode(2);//模式2
				osd_long_switsh = 1;
				osd_long_flag = 10;
				break;
			case KEY_MODE_NORMAL:
				/*printf("[%s]mode_success :%d \n",__func__,key_type);*/
				printf("INFO(%s)%s: normal success !\n",TAG_KEY,__func__);
				set_face_zoom_switch(DISABLE);//关闭算法
				osd_long_switsh = 1;
				osd_long_flag = 11;
				/*
					清空pantiltzoom 数据
				*/
				pantiltzoom_flag->bright_value = 128;
				pantiltzoom_flag->sharp_value = 128;
				pantiltzoom_flag->zoom_value = 100;
				pantiltzoom_flag->pan_value = 0;
				pantiltzoom_flag->tilt_value = 0;
				pantiltzoom_flag->pantilt_flag = 0;

				memset(&zoom_crop, 0, sizeof(zoom_crop));
				set_zoom(&pantiltzoom_flag->zoom_value);

				break;
			default:
				printf("INFO(%s)%s: set error %d!\n",TAG_KEY,__func__,key_type);
				break;
		}
}
/*
	图像控制
*/
static void frame_switch(key_info *flag)
{
	int test = flag->frame_on_off;

	if ( test ) {
		//开图像
		flag->frame_on_off = ENABLE;
		set_stream_switch(ENABLE);
		osd_switsh = 1;
		module_key_ctl_write(GPIO_PB17,DISABLE);
		flag->png_num = 1;
		sem_post(&sem_osd);
		/* 开长显图标 */
		osd_long_switsh = 1;
		osd_long_flag = osd_type;
		sem_post(&sem_osd_long);
		//printf("INFO(%s)[%d] on frame success!\n",TAG_KEY,__LINE__);
	} else {
		//关图像
		set_stream_switch(DISABLE);
		osd_switsh = 1;
		flag->png_num = 2;
		sem_post(&sem_osd);

		/* 亮度 */
		flag->bright_value = 128;
		/* 清晰度 */
		flag->sharp_value = 128;
		/* 缩放 */
		flag->zoom_value = 100;
		/* 左右 */
		flag->pan_value = 0;
		/* 上下 */
		flag->tilt_value = 0;
		/* pantilt_flag */
		flag->pantilt_flag = 0;

		memset(&zoom_crop, 0, sizeof(zoom_crop));
		/*
			zoom 回复初始值
		*/
		set_zoom(&flag->zoom_value);
		/*
			bright 回复初始值
		*/
		set_brightness_sharpness(DISABLE,&flag->bright_value,ENABLE);
		/*
			sharpness 回复初始值
		*/
		set_brightness_sharpness(DISABLE,&flag->sharp_value,DISABLE);

		/*
			关闭蓝灯 开红灯
		*/
		module_key_ctl_write(GPIO_PB17,ENABLE);
		//printf("INFO(%s)[%d] off frame success!\n",TAG_KEY,__LINE__);

		/* 长显图标关 */
		osd_long_switsh = 0;
		osd_type = osd_long_flag;
		osd_long_flag = -1;
		sem_post(&sem_osd_long);
	}

	return;

}

/*
	图像移动设置
	需要再线程中定义 pan 与 tilt
	i = （ -10，10 ）；
	pan = tilt = 3600

	// 左移案例
	pan = pan + 3600
	if( (*pan / 3600) <= 10 ) {

		pantilt_set ;
	} else {
		pan = 36000
	}
*/

static int set_pantilt(int *pan,int *tilt)
{
	int ret = 0;
	int pan_set = 0;
	int tilt_set = 0;

	/*
pan_set:   左( 3600 ~ 36000 ) 右( -3600 ~ -36000 )移动
		tilt_set:  上( -3600 ~ 36000 ) 下( 3600 ~ 36000 )移动
		一次移动以3600位一个单位
	*/

	pan_set = *pan;
	tilt_set = *tilt;



	int left = 0;
	int top = 0;
	int w = zoom_crop.w;
	int h = zoom_crop.h;

	left = round(1.0 *(1.0 * (g_VideoWidth - w) / 2) * (1 + 1.0 * pan_set / 36000));
	top = round(1.0 * (1.0 * (g_VideoHeight - h) / 2) * (1 + 1.0 * tilt_set / 36000));
	/*
		移动后的左上角的坐标
	*/
	pan_factor = 1.0 * pan_set / 36000;
	tilt_factor = 1.0 * tilt_set / 36000;

	IMPISPFrontCrop zoom_attr;
	zoom_attr.fcrop_enable = 1;
	zoom_attr.fcrop_left = left;
	zoom_attr.fcrop_top = top;
	zoom_attr.fcrop_width = w;
	zoom_attr.fcrop_height = h;
	ret = IMP_ISP_Tuning_SetFrontCrop(&zoom_attr);
	if (ret < 0) {
		printf("IMP Set Fcrop failed=%d\n",__LINE__);
		return -1;
	}

	return 0;
}



key_info key_info_source;
/*
   i2c 通信资源
   将要读取的buff数据
*/
volatile unsigned char rd_buf[2]={0x00};

static void  *gpio_ir_thread(void *arg)
{
	int fp;
	int ret;

	/*
	   初始化资源
	*/
	key_info_source.frame_on_off = ENABLE;
	key_info_source.audio_on_off = ENABLE;

	/* 亮度 */
	key_info_source.bright_value = 128;
	/* 清晰度 */
	key_info_source.sharp_value = 128;
	/* 缩放 */
	key_info_source.zoom_value = 100;
	/* 左右 */
	key_info_source.pan_value = 0;
	/* 上下 */
	key_info_source.tilt_value = 0;
	/*
		pantilt 使能标志
	*/
	key_info_source.pantilt_flag = 0;
	/*
		记录osd 显示的图片标号
	*/
	key_info_source.png_num = -1;

	/*
	   osd 显示时间控制变量
	*/
	struct timeval time;
	long int t1 = 0, t2 = 0;
	/*
	   协助osd_time osd显示时间为osd_time
	*/
	int flag_osd = 0;
	int osd_time = 1500;

	/*
	    连续按键测试时间
	*/
	/*struct timeval time_2;*/
	/*long int t3 = 0, t4 = 0;*/

	/*
	   出图 蓝灯亮 控制标志
	*/
	int flag_led = 0;
	/*
	   初始化LED
	   */
	key_init();
	printf("[INFO] key led init success !!!\n");
	/*
	   打开电源灯->红
	   */
	module_key_ctl_write(GPIO_PB17,ENABLE);

	fp = open(KEY_BOARD_DRV_NAME, O_RDWR);
	if (fp < 0) {
		printf("[INFO] open /dev/i2c-1 failed !!!\n");
		exit(1);
	}

	ret = ioctl(fp,I2C_SLAVE_FORCE,I2C_ADDR);
	if( ret < 0 ) {

		printf("set slave address failed !!!\n");
		exit(1);
	}

	printf("[INFO] open i2c-1 success !!!\n");

	while (1) {

		ret = read(fp, &rd_buf, 2);
		if ( ret < 0 ) {
			printf("[INFO] read i2c-1 data failed !!!\n");
			usleep( 200 * 1000 );
			continue;
		}

		/*if ( 0x55 == rd_buf[0] ) {*/
			/*usleep( 200 * 1000 );*/
			/*continue;*/
		/*}*/

		if ( get_frame_switch() ) {
			if ( 16 == rd_buf[0] ) { //KEY_1
				//printf("[INFO] frame on_off\n");
				frame_switch(&key_info_source);
				sem_post(&sem_led);
			} else if ( 10 == rd_buf[0] ) { //KEY_2
				if ( key_info_source.frame_on_off == DISABLE ) {
					continue;
				}
				audio_switch(&key_info_source);
				sem_post(&sem_osd);
				sem_post(&sem_led);
				//printf("[INFO] audio on_off\n");
			} else if ( 4 == rd_buf[0]  ) { //KEY_4
				//上
				//printf("[INFO] frame off move\n");
				if ( key_info_source.frame_on_off == DISABLE ) {
					continue;
				}
				if ( (key_type != KEY_MODE_NORMAL) )
					continue;
				if ( !key_info_source.pantilt_flag )
					continue;

				key_info_source.tilt_value += 1800;
				if ( (key_info_source.tilt_value / 1800 ) <= 20 ) {
					set_pantilt(&key_info_source.pan_value,&key_info_source.tilt_value);
				} else {
					key_info_source.tilt_value = 36000;
				}

				osd_switsh = 1;
				key_info_source.png_num = 6;
				sem_post(&sem_osd);
				sem_post(&sem_led);
			} else if ( 2 == rd_buf[0]  ) { //KEY_3
				//下
				//printf("[INFO] frame on move\n");
				if ( key_info_source.frame_on_off == DISABLE ) {
					continue;
				}
				if ( (key_type != KEY_MODE_NORMAL) )
					continue;
				if ( !key_info_source.pantilt_flag )
					continue;

				key_info_source.tilt_value -= 1800;
				if ( (key_info_source.tilt_value / 1800 ) >= -20 ) {
					set_pantilt(&key_info_source.pan_value,&key_info_source.tilt_value);
				} else {
					key_info_source.tilt_value = -36000;
				}

				osd_switsh = 1;
				key_info_source.png_num = 5;
				sem_post(&sem_osd);
				sem_post(&sem_led);
			} else if ( 5 == rd_buf[0]  ) { //KEY_5
				//左
				//printf("[INFO] frame left move\n");
				if ( key_info_source.frame_on_off == DISABLE ) {
					continue;
				}
				if ( (key_type != KEY_MODE_NORMAL) )
					continue;
				if ( !key_info_source.pantilt_flag )
					continue;

				key_info_source.pan_value += 1800;
				if ( (key_info_source.pan_value / 1800 ) <= 20 ) {
					set_pantilt(&key_info_source.pan_value,&key_info_source.tilt_value);
				} else {
					key_info_source.pan_value = 36000;
				}

				osd_switsh = 1;
				key_info_source.png_num = 8;
				sem_post(&sem_osd);
				sem_post(&sem_led);
			} else if ( 3 == rd_buf[0]  ) { //KEY_6
				//右
				//printf("[INFO] frame right move\n");
				if ( key_info_source.frame_on_off == DISABLE ) {
					continue;
				}
				if ( (key_type != KEY_MODE_NORMAL) )
					continue;
				if ( !key_info_source.pantilt_flag )
					continue;

				key_info_source.pan_value -= 1800;
				if ( (key_info_source.pan_value / 1800 ) >= -20 ) {
					set_pantilt(&key_info_source.pan_value,&key_info_source.tilt_value);
				} else {
					key_info_source.pan_value = -36000;
				}

				osd_switsh = 1;
				key_info_source.png_num = 7;
				sem_post(&sem_osd);
				sem_post(&sem_led);
			} else if ( 7 == rd_buf[0]  ) { //KEY_7
				++key_type;
				//printf("[INFO] frame mode0\n");
				if ( key_info_source.frame_on_off == DISABLE ) {
					continue;
				}
				if ( key_type == KEY_MODE_NUM ) {
					key_type = KEY_MODE_SINGLE;
				}

				key_mode_process(key_type,&key_info_source);
				sem_post(&sem_osd_long);
				sem_post(&sem_led);
			} else if ( 9 == rd_buf[0]  ) { //KEY_8
				//放大
				if ( key_info_source.frame_on_off == DISABLE ) {
					continue;
				}
				if ( key_type != KEY_MODE_NORMAL)
					continue;
				//printf("[INFO] be big\n");

				key_info_source.zoom_value += 2;
				//printf("[INFO] zoom_value %d \n",key_info_source.zoom_value);
				if ( key_info_source.zoom_value <= 200 ) {
					set_zoom(&key_info_source.zoom_value);
				} else {
					key_info_source.zoom_value = 200;
				}

				osd_switsh = 1;
				key_info_source.pantilt_flag = 1;
				key_info_source.png_num = 13;
				sem_post(&sem_osd);
				sem_post(&sem_led);
			} else if ( 8 == rd_buf[0]  ) { //KEY_9
				//缩小
				if ( key_info_source.frame_on_off == DISABLE ) {
					continue;
				}
				if ( key_type != KEY_MODE_NORMAL)
					continue;
				//printf("[INFO] no be big\n");

				key_info_source.zoom_value -= 2;
				//printf("[INFO] zoom_value %d \n",key_info_source.zoom_value);
				if(key_info_source.zoom_value >= 100) {
					set_zoom(&key_info_source.zoom_value);
				} else {
					key_info_source.zoom_value = 100;
				}

				osd_switsh = 1;
				key_info_source.pantilt_flag = 1;
				key_info_source.png_num = 12;
				sem_post(&sem_osd);
				sem_post(&sem_led);
			} else if ( 11 == rd_buf[0]  ) { //KEY_F1
				if ( key_info_source.frame_on_off == DISABLE ) {
					continue;
				}
				//亮度减
				//printf("[INFO] -- bright\n");
				set_brightness_sharpness(DISABLE,&key_info_source.bright_value,ENABLE);
				osd_switsh = 1;
				key_info_source.png_num = 14;
				sem_post(&sem_osd);
				sem_post(&sem_led);
			} else if ( 12 == rd_buf[0]  ) { //KEY_F2
				if ( key_info_source.frame_on_off == DISABLE ) {
					continue;
				}
				//亮度加
				set_brightness_sharpness(ENABLE,&key_info_source.bright_value,ENABLE);
				//printf("[INFO] ++ bright\n");
				osd_switsh = 1;
				key_info_source.png_num = 15;
				sem_post(&sem_osd);
				sem_post(&sem_led);
			} else if ( 0 == rd_buf[0]  ) { //KEY_F3
				if ( key_info_source.frame_on_off == DISABLE ) {
					continue;
				}
				//锐度减
				//printf("[INFO] -- sharpness\n");
				set_brightness_sharpness(DISABLE,&key_info_source.sharp_value,DISABLE);
				osd_switsh = 1;
				key_info_source.png_num = 16;
				sem_post(&sem_osd);
				sem_post(&sem_led);
			} else if ( 6 == rd_buf[0]  ) { //KEY_F4
				if ( key_info_source.frame_on_off == DISABLE ) {
					continue;
				}
				//锐度加
				//printf("[INFO] ++ sharpness\n");
				set_brightness_sharpness(ENABLE,&key_info_source.sharp_value,DISABLE);
				osd_switsh = 1;
				key_info_source.png_num = 17;
				sem_post(&sem_osd);
				sem_post(&sem_led);
			}
		}

		/*
			测试连续按键所要的时间
		*/
		/*gettimeofday(&time_2, NULL);*/
		/*t3 = time_2.tv_sec * 1000 + time_2.tv_usec / 1000;*/
		/*printf("[INFO] time %ld \n ",t3-t4);*/
		/*t4 = t3;*/
		/*printf("[INFO] %d \n ",rd_buf[0]);*/

		/*
		   出图 蓝灯亮
		   */
		if ( get_frame_switch() && !flag_led ) {
			flag_led = 1;
			module_key_ctl_write(GPIO_PB17,DISABLE);
			module_key_ctl_write(GPIO_PB27,ENABLE);
			osd_long_switsh = 1;
			osd_long_flag = 9;
			sem_post(&sem_osd_long);
		}

		/*
		   控制osd 显示时间为2S
		   osd_time 控制显示时间
		   osd_time/1000 = 2S
		*/
		if ( ( rd_buf[0] != 0x55 || flag_osd )  && get_frame_switch() ) {
			if( osd_switsh == 1 ) {
				gettimeofday(&time, NULL);
				t1 = time.tv_sec * 1000 + time.tv_usec / 1000;
				/* 后期时间上的优化 dbug */
				/*printf("[INFO] t1 : %ld\n",t1);*/
				/*printf("[INFO] t2 : %ld\n",t2);*/
				/*printf("[INFO] t1 - t2 : %ld\n",t1 - t2);*/
				if((t1 - t2) > osd_time && flag_osd ) {
					if ( key_info_source.png_num == 2 ) {
						key_info_source.frame_on_off = 0;
						flag_osd_black = 1;
					}
					osd_switsh = 0;
					//printf("[INFO] ----t1 - t2---- : %ld\n",t1 - t2);
					key_info_source.png_num = -1;
					flag_osd = 0;
					/* 进入osd 显示函数 */
					sem_post(&sem_osd);
				} else {
					if ( !flag_osd ) {
						t2 = time.tv_sec * 1000 + time.tv_usec / 1000;
						flag_osd = 1;
					}

				}
				if ( (t1 - t2) < (osd_time - 800) && rd_buf[0] != 0x55 ) {
					t1 = time.tv_sec * 1000 + time.tv_usec / 1000;
					t2 = time.tv_sec * 1000 + time.tv_usec / 1000;
				}

			}

		}

		/*
			关闭图像
		*/
		if ( !get_frame_switch() && flag_led ){


			printf("[INFO] frame %d  flag_led %d \n",get_frame_switch(),flag_led);
			key_info_source.frame_on_off = ENABLE;
			key_info_source.audio_on_off = ENABLE;

			/* 亮度 */
			key_info_source.bright_value = 128;
			/* 清晰度 */
			key_info_source.sharp_value = 128;
			/* 缩放 */
			key_info_source.zoom_value = 100;
			/* 左右 */
			key_info_source.pan_value = 0;
			/* 上下 */
			key_info_source.tilt_value = 0;

			/*
			   出图 蓝灯亮 控制标志
			*/
			flag_osd = 0;
			flag_led = 0;
			osd_switsh = 0;
			osd_long_flag = 0;
			key_info_source.png_num = -1;
			key_info_source.pantilt_flag = 0;
			memset(&zoom_crop, 0, sizeof(zoom_crop));

			module_key_ctl_write(GPIO_PB17,ENABLE);
			module_key_ctl_write(GPIO_PB27,DISABLE);
		}

		usleep( 50 * 1000 );

	}

	close(fp);
	return NULL;
}

static void  *gpio_osd_thread(void *arg)
{
	int ret = 0;
	while(1) {
		sem_wait(&sem_osd);
		if ( get_frame_switch() ) {
			//osd 图像显示
			/* 控制显示black 图时，协调osd 显示时间和再次进入black图 */
			if( !key_info_source.frame_on_off ) {
				if ( key_info_source.png_num == 2 ) key_info_source.frame_on_off = 1;
				if ( flag_osd_black == 1 ) {
					key_info_source.frame_on_off = 1;
					flag_osd_black = 0;
				}
				//lock
				ret = pthread_mutex_lock(&lock);
				if(0 != ret){
					perror("pthread_mutex_lock");
					pthread_exit(NULL);
				}
				set_osd_show(&osd_switsh,1,key_info_source.png_num);
				//unlock
				ret = pthread_mutex_unlock(&lock);
				if(0 != ret){
					perror("pthread_mutex_unlock");
					pthread_exit(NULL);
				}
			}
		}
	}
	return NULL;
}

static void  *gpio_osd_long_thread(void *arg)
{
	int ret_long = 0;
	while(1) {
		sem_wait(&sem_osd_long);
		if ( get_frame_switch() ) {
			//osd 图像显示
				//lock
				ret_long = pthread_mutex_lock(&lock);
				if(0 != ret_long){
					perror("pthread_mutex_lock");
					pthread_exit(NULL);
				}
				set_osd_show(&osd_long_switsh,2,osd_long_flag);
				//unlock
				ret_long = pthread_mutex_unlock(&lock);
				if(0 != ret_long){
					perror("pthread_mutex_unlock");
					pthread_exit(NULL);
				}
		}
	}
	return NULL;
}

static void  *gpio_led_thread(void *arg)
{
	while(1) {
		sem_wait(&sem_led);
		if( get_frame_switch() ) {
			//LED 闪烁
			if( !key_info_source.frame_on_off ) {
				module_key_ctl_write(GPIO_PB27,ENABLE);
				usleep( 60 * 1000 );
				module_key_ctl_write(GPIO_PB27,DISABLE);
				usleep( 60 * 1000 );
				module_key_ctl_write(GPIO_PB27,ENABLE);
			} else {
				module_key_ctl_write(GPIO_PB27,DISABLE);
			}

		}
	}

	return NULL;
}

int module_key_process(void)
{
	int ret = 0;

	/* 初始化信号 */
	ret = sem_init(&sem_osd, 0, 0);
	if (ret == -1) {
		printf("sem_init_osd failed \n");
		return -1;
	}

	ret = sem_init(&sem_led, 0, 0);
	if (ret == -1) {
		printf("sem_init_led failed \n");
		return -1;
	}

	ret = sem_init(&sem_osd_long, 0, 0);
	if (ret == -1) {
		printf("sem_init_led failed \n");
		return -1;
	}

	static pthread_t gpio_ir_pid;
	pthread_attr_t gpio_ir_attr;
	pthread_attr_init(&gpio_ir_attr);
	pthread_attr_setdetachstate(&gpio_ir_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&gpio_ir_attr, SCHED_OTHER);
	ret = pthread_create(&gpio_ir_pid, &gpio_ir_attr, &gpio_ir_thread, NULL);
	if (ret) {
		printf("ERROR(%s): create thread for gpio_ir_thread failed!\n", TAG_KEY);
		return -1;
	}

	static pthread_t gpio_led_pid;
	pthread_attr_t gpio_led_attr;
	pthread_attr_init(&gpio_led_attr);
	pthread_attr_setdetachstate(&gpio_led_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&gpio_led_attr, SCHED_OTHER);
	ret = pthread_create(&gpio_led_pid, &gpio_led_attr, &gpio_led_thread, NULL);
	if (ret) {
		printf("ERROR(%s): create thread for gpio_led_thread failed!\n", TAG_KEY);
		return -1;
	}

	static pthread_t gpio_osd_pid;
	pthread_attr_t gpio_osd_attr;
	pthread_attr_init(&gpio_osd_attr);
	pthread_attr_setdetachstate(&gpio_osd_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&gpio_osd_attr, SCHED_OTHER);
	ret = pthread_create(&gpio_osd_pid, &gpio_osd_attr, &gpio_osd_thread, NULL);
	if (ret) {
		printf("ERROR(%s): create thread for gpio_osd_thread failed!\n", TAG_KEY);
		return -1;
	}

	static pthread_t gpio_osd_long_pid;
	pthread_attr_t gpio_osd_long_attr;
	pthread_attr_init(&gpio_osd_long_attr);
	pthread_attr_setdetachstate(&gpio_osd_long_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&gpio_osd_long_attr, SCHED_OTHER);
	ret = pthread_create(&gpio_osd_long_pid, &gpio_osd_long_attr, &gpio_osd_long_thread, NULL);
	if (ret) {
		printf("ERROR(%s): create thread for gpio_osd_thread failed!\n", TAG_KEY);
		return -1;
	}

	return 0;
}
