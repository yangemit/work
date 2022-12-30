#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/module.h>
#include <linux/mfd/jz_tcu.h>
#include "motorctrl.h"
#define DRIVER_VERSION "2020.08.15"
#define DRIVER_AUTHOR "Nick"
#define DRIVER_DESC "PWM StepMotor Driver"
#define DRIVER_LICENSE "GPL"
 
#define	DEVICE_NAME	"motor"
#define	MOTOR_MAJOR	173
 
 
#define	DEF_PULSE	500

#define GPIO(port_group,port_offset) port_group*32+port_offset


#define PIN_LEFTSTOP    GPIO(1,26)  //stop detect
#define PIN_RIGHTSTOP    GPIO(1,25)
#define MOTOR_DRV_SLEEP_PORT    GPIO(1,29) //
#define motor_output_enable  gpio_direction_output(MOTOR_DRV_SLEEP_PORT,1)
#define motor_output_disable  gpio_direction_output(MOTOR_DRV_SLEEP_PORT,0)

#define SYS_WRITEL(Addr, Value) ((*(volatile unsigned int *)(Addr)) = (Value))
#define	THOUSAND_US	1000000
 
/* debug macros */
#define DEBUG	1
 
#ifdef DEBUG
#define DPRINTK(fmt,args...) printk(fmt,##args)
#else
#define DPRINTK(fmt,args...)
#endif
 
typedef struct _stepMotorInfo
{
	uint openCnt;
	uint pulse;
	uint in_leftstop;
	uint in_rightstop;
	spinlock_t lock;
	char motor_action;
	char motor_position;
	unsigned int stepcnt;
	struct semaphore sem;
	struct class *class;
	struct cdev cdev;
}stepMotorInfo,*pstepMotorInfo;

#define NUM_SHORT_STEP   8
const uint SIN[NUM_SHORT_STEP+1]={
0,
98,
191, 
278,//722,
354,//783, 
416,//833,
462,//870,
490,//892,
500//900,
  };
const uint COS[NUM_SHORT_STEP+1]={
500,
490,
462,
416,
354,
278,
191,
98,
0};


struct pwm_device_t {
	int duty;
	int period;
	int polarity;
	struct pwm_device *pwm_device;
};
struct pwm_lookup jz_pwm_lookup[] = {
	PWM_LOOKUP("tcu_chn0.0", 1, "pwm-jz", "pwm-jz.0"),
	PWM_LOOKUP("tcu_chn1.1", 1, "pwm-jz", "pwm-jz.1"),
	PWM_LOOKUP("tcu_chn2.2", 1, "pwm-jz", "pwm-jz.2"),
	PWM_LOOKUP("tcu_chn3.3", 1, "pwm-jz", "pwm-jz.3"),
};

#define PWM_NUM  4

struct pwm_device_t pwm_cntrl[PWM_NUM];

stepMotorInfo motor;
 
static int motor_major = MOTOR_MAJOR;
 
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);
MODULE_VERSION(DRIVER_VERSION);


#define K   10
#define PWM_FREQ   20000
#define MOTOR_PWM_A1_CH   0
#define MOTOR_PWM_A2_CH   1
#define MOTOR_PWM_B1_CH   2
#define MOTOR_PWM_B2_CH   3
//static uint short_step_index=0;
#define MOTOR_PWM_PERIOD		50000//ns

static void hw_pwm_init(void)
{
	int i;
	char pd_name[10];

for(i = 0; i < PWM_NUM; i++) {
		sprintf(pd_name, "pwm-jz.%d", i);
		pwm_cntrl[i].pwm_device = devm_pwm_get(NULL, pd_name);
		if (IS_ERR(pwm_cntrl[i].pwm_device)) {
			printk("devm_pwm_get error !");
			return ;
		}
		pwm_cntrl[i].duty = MOTOR_PWM_PERIOD/2;//50%
		pwm_cntrl[i].period = MOTOR_PWM_PERIOD;//20khz
		pwm_cntrl[i].polarity = 0;
		pwm_set_polarity(pwm_cntrl[i].pwm_device, PWM_POLARITY_INVERSED);
			pwm_config(pwm_cntrl[i].pwm_device, pwm_cntrl[i].duty, pwm_cntrl[i].period);
	}
}
static void hw_pwm_enable(unsigned char pwm_ch,unsigned char bEable)
{
     if(bEable)
				pwm_enable(pwm_cntrl[pwm_ch].pwm_device);
	 else
   				pwm_disable(pwm_cntrl[pwm_ch].pwm_device);

}
static void hw_pwm_set_duty(unsigned char pwm_ch,int duty,int period)
{
		pwm_cntrl[pwm_ch].duty=duty;
		pwm_cntrl[pwm_ch].period=period;
		pwm_config(pwm_cntrl[pwm_ch].pwm_device, duty, period);

}
 static void one_short_step(uint short_step_index)
{		
		uint pwm_duty1,pwm_duty2,pwm_duty3,pwm_duty4;  
			/*0~90*/
		 if((short_step_index>=0)&&(short_step_index<=NUM_SHORT_STEP))
		{
			pwm_duty1=50-K*COS[short_step_index]/100;
			pwm_duty3=50-K*SIN[short_step_index]/100;	
			pwm_duty2=100-pwm_duty1;
			pwm_duty4=100-pwm_duty3;
			
			//hipwm_set_time(MOTOR_PWM_A_CH,PWM_FREQ,(u32)pwm_duty1);
			//hipwm_set_time(MOTOR_PWM_B_CH,PWM_FREQ,(u32)pwm_duty2);
		 }
		/*90~180*/
	  	else if((short_step_index>NUM_SHORT_STEP)&&(short_step_index<=NUM_SHORT_STEP*2))
		{
			pwm_duty1=50+K*SIN[short_step_index-NUM_SHORT_STEP]/100;
			pwm_duty3=50-K*COS[short_step_index-NUM_SHORT_STEP]/100;
			//hipwm_set_time(MOTOR_PWM_A_CH,PWM_FREQ,(u32)pwm_duty1);
			//hipwm_set_time(MOTOR_PWM_B_CH,PWM_FREQ,(u32)pwm_duty2);
			pwm_duty2=100-pwm_duty1;
			pwm_duty4=100-pwm_duty3;
			
			//hipwm_set_time(MOTOR_PWM_A_CH,PWM_FREQ,(u32)pwm_duty1);
			//hipwm_set_time(MOTOR_PWM_B_CH,PWM_FREQ,(u32)pwm_duty2);
		
			}
		/*180~270*/
	 	else if((short_step_index>NUM_SHORT_STEP*2)&&(short_step_index<=NUM_SHORT_STEP*3))
		{
			pwm_duty1=50+K*COS[short_step_index-NUM_SHORT_STEP*2]/100;
			pwm_duty3=50+K*SIN[short_step_index-NUM_SHORT_STEP*2]/100;
			//hipwm_set_time(MOTOR_PWM_A_CH,PWM_FREQ,(u32)pwm_duty1);
			//hipwm_set_time(MOTOR_PWM_B_CH,PWM_FREQ,(u32)pwm_duty2);
			pwm_duty2=100-pwm_duty1;
			pwm_duty4=100-pwm_duty3;
			
			//hipwm_set_time(MOTOR_PWM_A_CH,PWM_FREQ,(u32)pwm_duty1);
			//hipwm_set_time(MOTOR_PWM_B_CH,PWM_FREQ,(u32)pwm_duty2);
		
		}
		/*270~360*/
		else if((short_step_index>NUM_SHORT_STEP*3)&&(short_step_index<=NUM_SHORT_STEP*4))
		{
			pwm_duty1=50-K*SIN[short_step_index-NUM_SHORT_STEP*3]/100;
			pwm_duty3=50+K*COS[short_step_index-NUM_SHORT_STEP*3]/100;
	        pwm_duty2=100-pwm_duty1;
			pwm_duty4=100-pwm_duty3;
			
			//hipwm_set_time(MOTOR_PWM_A_CH,PWM_FREQ,(u32)pwm_duty1);
			//hipwm_set_time(MOTOR_PWM_B_CH,PWM_FREQ,(u32)pwm_duty2);
		}
		//printf("pwm duty=%d,%d,%d,%d\n",pwm_duty1,pwm_duty2,pwm_duty3,pwm_duty4);
		hw_pwm_set_duty(0,MOTOR_PWM_PERIOD,MOTOR_PWM_PERIOD/100* pwm_duty1);
		hw_pwm_set_duty(1, MOTOR_PWM_PERIOD,MOTOR_PWM_PERIOD/100*pwm_duty2);
		hw_pwm_set_duty(2,MOTOR_PWM_PERIOD,MOTOR_PWM_PERIOD/100* pwm_duty3);
		hw_pwm_set_duty(3, MOTOR_PWM_PERIOD,MOTOR_PWM_PERIOD/100*pwm_duty4);	
 	}

 void gen_io_right_step(void)
{		
#if 0
	int ret = -1;
	motor_output_disable;
	hw_pwm_enable(0,1);
	hw_pwm_enable(1,1);
	hw_pwm_enable(2,1);
	hw_pwm_enable(3,1);
	ret = gpio_request(MOTOR_CNTRL_PIN_A,"out_ma");	
	if(ret < 0)	
	{		
	printk(KERN_NOTICE "MOTOR_CNTRL_PIN_A ERR\n");	
	return;
	}	
	ret = gpio_request(MOTOR_CNTRL_PIN_B,"out_mb");
	if(ret < 0)	{	
	printk(KERN_NOTICE "MOTOR_CNTRL_PIN_B ERR\n");	
		return;

	}		
	motor_output_enable;
	gpio_direction_output(MOTOR_CNTRL_PIN_A,1);	
	gpio_direction_output(MOTOR_CNTRL_PIN_B,1);	
	udelay(1000);
	gpio_direction_output(MOTOR_CNTRL_PIN_A,1);	
	gpio_direction_output(MOTOR_CNTRL_PIN_B,0);	
	udelay(1000);
	gpio_direction_output(MOTOR_CNTRL_PIN_A,0);	
	gpio_direction_output(MOTOR_CNTRL_PIN_B,0);	
	udelay(1000);
	gpio_direction_output(MOTOR_CNTRL_PIN_A,0);	
	gpio_direction_output(MOTOR_CNTRL_PIN_B,1);	
	udelay(1000);
		motor_output_disable;
	hw_pwm_enable(0,0);
	hw_pwm_enable(1,0);
	hw_pwm_enable(2,0);
	hw_pwm_enable(3,0);
	#endif
}

  void gen_io_left_step(void)
{		
#if 0
	int ret = -1;
	motor_output_disable;
	hw_pwm_enable(0,1);
	hw_pwm_enable(1,1);
	hw_pwm_enable(2,1);
	hw_pwm_enable(3,1);
	ret = gpio_request(MOTOR_CNTRL_PIN_A,"out_ma");	
	if(ret < 0)	
	{		
	printk(KERN_NOTICE "MOTOR_CNTRL_PIN_A ERR\n");	
	return;
	}	
	ret = gpio_request(MOTOR_CNTRL_PIN_B,"out_mb");
	if(ret < 0)	{	
			gpio_free(MOTOR_CNTRL_PIN_A);
	printk(KERN_NOTICE "MOTOR_CNTRL_PIN_B ERR\n");	
		return;

	}		
	motor_output_enable;
	gpio_direction_output(MOTOR_CNTRL_PIN_A,0);	
	gpio_direction_output(MOTOR_CNTRL_PIN_B,1);	
	udelay(1000);
	gpio_direction_output(MOTOR_CNTRL_PIN_A,0);	
	gpio_direction_output(MOTOR_CNTRL_PIN_B,0);	
	udelay(1000);
	gpio_direction_output(MOTOR_CNTRL_PIN_A,1);	
	gpio_direction_output(MOTOR_CNTRL_PIN_B,0);	
	udelay(1000);
	gpio_direction_output(MOTOR_CNTRL_PIN_A,1);	
	gpio_direction_output(MOTOR_CNTRL_PIN_B,1);	
	udelay(1000);

	gpio_free(MOTOR_CNTRL_PIN_A);
		gpio_free(MOTOR_CNTRL_PIN_B);

		motor_output_disable;
			hw_pwm_enable(0,0);
	hw_pwm_enable(1,0);
	hw_pwm_enable(2,0);
	hw_pwm_enable(3,0);
	#endif

}
static void stepMotorStop(pstepMotorInfo motor)
{
	printk(KERN_NOTICE "stepMotorStop\n");	
	hw_pwm_enable(MOTOR_PWM_A1_CH,0);
	hw_pwm_enable(MOTOR_PWM_A2_CH,0);
	hw_pwm_enable(MOTOR_PWM_B1_CH,0);
	hw_pwm_enable(MOTOR_PWM_B2_CH,0);
	motor_output_disable;
}
 
static u8 pos_end_flag=0;
static void stepMotorTurnLeft(pstepMotorInfo motor)
{
	uint short_step_index=0;
	motor->motor_action=eMOTOR_ACTION_LEFT;
	hw_pwm_enable(0,1);
	hw_pwm_enable(1,1);
	hw_pwm_enable(2,1);
	hw_pwm_enable(3,1);
		motor_output_enable;

	while(short_step_index<NUM_SHORT_STEP*4)
	{
	    if(gpio_get_value(motor->in_leftstop)==0)
				{
					pos_end_flag=1;
					break;
			  }
		one_short_step(short_step_index);
		short_step_index++;
		udelay(motor->pulse);

	}

}
 
//A->AD->D->DC->C->CB->B->BA->A
static void stepMotorTurnRight(pstepMotorInfo motor)
{
	uint short_step_index=NUM_SHORT_STEP*4;
	motor->motor_action=eMOTOR_ACTION_RIGHT;
	hw_pwm_enable(0,1);
	hw_pwm_enable(1,1);
	hw_pwm_enable(2,1);
	hw_pwm_enable(3,1);
		motor_output_enable;

	while(short_step_index>0)
	{
	   if(gpio_get_value(motor->in_rightstop)==0)
		{
				pos_end_flag=1;
				break;
	    }
		one_short_step(short_step_index);
		short_step_index--;
		udelay(motor->pulse);
	}
}
 
static int motorSpinLeft(pstepMotorInfo motor)
{
	int ret = -1;

	printk(KERN_NOTICE "motorSpinLeft\n");	
	if(	motor->motor_position==eMOTOR_POS_LEFT)
	{
		printk(KERN_NOTICE "position has be in left\n");
		return 0;				
	}
    motor->stepcnt=0;
	motor->motor_action=eMOTOR_ACTION_LEFT;
    motor_output_enable;

	pos_end_flag=0;
   if(gpio_get_value(motor->in_leftstop)==0)
   	{
			pos_end_flag=1;

	 }
	while((pos_end_flag==0)&&(motor->stepcnt<MAX_STEP_CNT))
	{/*
		 if(motor->stepcnt<2)
			{
				if(gpio_get_value(motor->in_leftstop)==0)
				{
						pos_end_flag=1;
						break;
			    }
				 gen_io_left_step();
			}
		else*/{
			stepMotorTurnLeft(motor);
		}
		motor->stepcnt++;
	}
	printk(KERN_NOTICE "step=%d\n", motor->stepcnt);	
	if(motor->stepcnt>=MAX_STEP_CNT)
		{
			motor->motor_position=eMOTOR_POS_UNKNOW;
		}
	else{
			motor->motor_position=eMOTOR_POS_LEFT;
	}
	motor->motor_action=eMOTOR_ACTION_IDLE;
	stepMotorStop(motor);
	up(&motor->sem);	
 
	return ret;
}
 
static int motorSpinRight(pstepMotorInfo motor)
{
	int ret = -1;
	// unsigned char  right_retry_cnt=0;
	/*
	ret = down_trylock(&motor->sem);
	if(ret)
	{
		printk(KERN_NOTICE "motor->sem lock failed\n");	
		return ret;
	}*/
	printk(KERN_NOTICE "motorSpinRight\n");	
	if(	motor->motor_position==eMOTOR_POS_RIGHT)
	{
		printk(KERN_NOTICE "position has be in right\n");
		return 0;				
	}
	motor->stepcnt=0;
	motor->motor_action=eMOTOR_ACTION_RIGHT;
		    			motor_output_enable;
   pos_end_flag=0;
   if(gpio_get_value(motor->in_rightstop)==0)
   	{
			pos_end_flag=1;

	 }
	while((pos_end_flag==0)&&(motor->stepcnt<MAX_STEP_CNT))
	{
	    /*
		if(motor->stepcnt<2)
		{
			if(gpio_get_value(motor->in_rightstop)==0)
			{
					pos_end_flag=1;
					break;
		    }
			 gen_io_right_step();
		}
		else*/{
		//udelay(1000);
		 stepMotorTurnRight(motor);
		}
	   motor->stepcnt++; 
		
	}
  	printk(KERN_NOTICE "step=%d\n", motor->stepcnt);	
	if(motor->stepcnt>=MAX_STEP_CNT)
	{
		motor->motor_position=eMOTOR_POS_UNKNOW;
	}
	else{
			motor->motor_position=eMOTOR_POS_RIGHT;
	}
	//motor->motor_position=eMOTOR_POS_RIGHT;
	motor->motor_action=eMOTOR_ACTION_IDLE;
	stepMotorStop(motor);
	up(&motor->sem);	
	return ret;
}

static int motor_open(struct inode *inode, struct file *filp)
{
	int ret = -1;
    //void  *reg_iocfg_core_base=0;
    
	spin_lock(&motor.lock);
 
	if(motor.openCnt)
	{
		spin_unlock(&motor.lock);
		return -EBUSY;
	}	
	filp->private_data = &motor;
	motor.openCnt++;
	motor.in_leftstop = PIN_LEFTSTOP;
	motor.in_rightstop = PIN_RIGHTSTOP;
	motor.pulse = DEF_PULSE;

		/*motor enbale gpio*/
	gpio_direction_output(MOTOR_DRV_SLEEP_PORT,0);
	gpio_request(MOTOR_DRV_SLEEP_PORT,NULL);
	motor_output_disable;

	hw_pwm_init();
	stepMotorStop(&motor);

	ret = gpio_request(motor.in_leftstop,"in_leftstop");
	if(ret < 0)
	{
		goto  err_in_leftstop;
	}
	gpio_direction_input(motor.in_leftstop);

	ret = gpio_request(motor.in_rightstop,"in_rightstop");
	if(ret < 0)
	{
		goto  err_in_rightstop;
	}
	gpio_direction_input(motor.in_rightstop);
	motor.motor_action=eMOTOR_ACTION_IDLE;
	motor.motor_position=eMOTOR_POS_UNKNOW;
	spin_unlock(&motor.lock);	
	return ret;
err_in_rightstop:
	gpio_free(motor.in_leftstop);
err_in_leftstop:
	spin_unlock(&motor.lock);
	return ret;
}
static int motor_release(struct inode *inode, struct file *filp)
{
    int i;
	pstepMotorInfo dev = filp->private_data;

	spin_lock(&dev->lock);

	gpio_free(dev->in_leftstop);
	gpio_free(dev->in_rightstop);
	gpio_free(MOTOR_DRV_SLEEP_PORT);

	dev->openCnt--;
	for(i = 0; i < PWM_NUM; i++) {
			devm_pwm_put(NULL,pwm_cntrl[i].pwm_device);
	}
	spin_unlock(&dev->lock);
 
	return 0;
}
 
static long motor_ioctl(struct file *filp,unsigned int cmd, unsigned long arg)
{
	int ret = -1;
 
	uint freq = 0;
 
	pstepMotorInfo dev = filp->private_data;
	printk(KERN_INFO DEVICE_NAME " 1013 ioctl!\n");
	if (_IOC_TYPE(cmd) != MOTOR_IOC_MAGIC)
		return -EINVAL;
	if (_IOC_NR(cmd) > MOTOR_IOC_MAXNR)
		return -EINVAL;
 
	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
	if (ret)
		return -EFAULT;
 
	switch(cmd)
	{
		case MOTOR_IOCGETDATA:
			freq = THOUSAND_US/dev->pulse;
			ret = __put_user(freq,(uint *)arg);
			break;
 
		case MOTOR_IOCSETDATA:
			ret = __get_user(freq,(uint *)arg);
			if(0 == ret)
				dev->pulse = THOUSAND_US/freq;
			break;
 
		case MOTOR_IOCSETLEFT:
			motorSpinLeft(dev);		
			break;
 
		case MOTOR_IOCSETRIGHT:
			motorSpinRight(dev);		
			break;
		case MOTOR_IOCGET_POS:
				ret = __put_user(dev->motor_position,(uint *)arg);
			break;

		default:
			return -EINVAL;
	}
 
	return ret;
}
 
static const struct file_operations motor_fops = {
	.owner  	=       THIS_MODULE,
	.open		=	motor_open,
	.release	=	motor_release,	
	.unlocked_ioctl =motor_ioctl
};
 
static int stepMotor_init(void)
{
 
	int ret = -1;
 
	dev_t devno = MKDEV(motor_major,0);
	printk(KERN_INFO "stepMotor modules init");
	if(motor_major)
		ret = register_chrdev_region(devno,1,"motordev");
	else
	{
		ret = alloc_chrdev_region(&devno,0,1,"motordev");
		motor_major = MAJOR(devno);
	}		
 
	if(ret < 0)
	{
		printk(KERN_EMERG "\tget dev_number fail!\n");
		return ret;
	}
	if(0 == motor_major)
		motor_major = ret;
	cdev_init(&motor.cdev,&motor_fops);
	motor.cdev.owner = THIS_MODULE;
	motor.cdev.ops = &motor_fops;
	ret = cdev_add(&motor.cdev,MKDEV(motor_major,0),1); 
	if(ret)
		printk(KERN_NOTICE "Error %d adding motor.\n",ret);		
	motor.class = class_create(THIS_MODULE,DEVICE_NAME);
	if(IS_ERR(motor.class))
	{
		cdev_del(&motor.cdev);
		unregister_chrdev_region(MKDEV(motor_major,0),1);
		return PTR_ERR(motor.class);
	}
	device_create(motor.class,NULL,MKDEV(motor_major,0),NULL,DEVICE_NAME);
	motor.openCnt = 0;
	pwm_add_table(jz_pwm_lookup, 4);

	spin_lock_init(&motor.lock);
	sema_init(&motor.sem,1);	
	printk(KERN_INFO DEVICE_NAME"\tregister ok!\n");
	return ret;
}
static void stepMotor_exit(void)
{
	cdev_del(&motor.cdev);
	device_destroy(motor.class,MKDEV(motor_major,0));
	class_destroy(motor.class);
	unregister_chrdev_region(MKDEV(motor_major,0),1);
	printk(KERN_INFO DEVICE_NAME"\tunregister!\n");
}
module_init(stepMotor_init);
module_exit(stepMotor_exit);
MODULE_LICENSE("GPL");

