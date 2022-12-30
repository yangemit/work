#ifndef __VCM_CONTROL_H__
#define __VCM_CONTROL_H__
#define MOTOR_MOVE		0x3   //_IOW('M', 1, int)
#define MOTOR_READ		0x4   //_IOW('M', 1, int)

extern int g_VcmValue;
extern int g_VcmEn;
extern int af_state;
extern int af_thread_cancel;

struct whf_typedef
{
	int sensorW;			// sensor出图的宽
	int sensorH;			// sensor出图的高
	int sensorF;			// sensor出图的帧率
	int ispW;				// ISP裁剪后的宽
	int ispH;				// ISP裁剪后的高
	int cropX;				// ISP裁剪的起始X
	int cropY;				// ISP裁剪的起始Y
	int outputW;			// 裁剪加缩放后实际的输出宽
	int outputH;			// 裁剪加缩放后实际的输出高	
	int productType;		// 产品类型: 2M相机为1,4M相机为2
	int reserve;			// 保留字段
};

struct vcm_typedef
{
	int nearCoord;			// 小距离的清晰坐标
	int calibrationCoord;	// 中距离（3.4M时）的清晰坐标
	int farCoord;			// 无穷远时的清晰坐标
	float slope;			// 通过中距离计算小距离和无穷远时的斜率
	int tof1_status;		// TOF1的状态，为0时，tof1损坏，为1时，tof1正常工作
	int tof2_status;		// TOF2的状态，为0时，tof2损坏，为1时，tof2正常工作
	int garde;				// 灵敏度等级
	int reserve;			// 保留字段
};

/** 初始化接口，在调用AF_MotorCtrl 和 AF_GetCoord 之前调用*/
int AF_Init();
int AF_Uninit();

// 获取镜头的有效范围，单位um
int AF_GetMaxCoord();
int AF_GetMinCoord();

/**************************************************************************
* Name                 :  void AF_MotorCtrl(int chan, int dir, int step)
* Describe             :  移动马达
* Inputs               :  chan: 通道，无效参数；
*						  dir: 方向，>=0为正方向，<0为反方向
*						  step: 步长 (单位：um)
* Outputs              :  NONE
*************************************************************************/
void AF_MotorCtrl(int chan, int dir, int step);

/**************************************************************************
* Name                 :  int AF_GetCoord(int chan);
* Describe             :  获取当前FOCUS马达坐标
* Inputs               :  chan: 通道号，无效参数
* Outputs              :  NONE
* Return               :  坐标值 
*************************************************************************/
int AF_GetCoord(int chan);

/**************************************************************************
* Name                 :  int AF_IsStreamOn(int chan);
* Describe             :  判断相机是否处于预览状态
* Inputs               :  chan: 通道号，无效参数
* Outputs              :  NONE
* Return               :  0：关闭预览状态，非0：预览状态
*************************************************************************/
int AF_IsStreamOn(int chan);

/**************************************************************************
* Name                 :  int AF_GetResolvFPS(int chan, struct whf_typedef * whf);
* Describe             :  获取相机的分辨率和帧率
* Inputs               :  chan: 通道号，无效参数
* Inputs               :  whf: 图像相关结构体
* Outputs              :  NONE
* Return               :  NONE
*************************************************************************/
int AF_GetResolvFPS(int chan, struct whf_typedef * whf);

int start_auto_focus();
int stop_auto_focus();

int set_value_ctl_vcm(int value);
int get_value_ctl_vcm();

int search_peak();
int read_vcm_value();
void write_vcm_value(int value);
void thread_vcm_check(void *arg);
int read_vcm_en();
void write_vcm_en(int value);

long long int get_daf(int chan);
void init_af_win(int chan);

#endif
