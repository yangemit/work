#include <linux/module.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/device.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/gpio.h>

/*#define GPIO_KEY1_PIN		GPIO_PA(6)*/
#define GPIO_KEY1_PIN		GPIO_PB(25)
#define GPIO_KEY1_IRQ		gpio_to_irq(GPIO_KEY1_PIN)
#define KEY1_MISC_NAME		"key1"

#define CMD_GET_VAL            _IOR('s', 1, int)

struct gpio_key{
	int gpio_pin;
	int pin_val;
	int irq;
	int irq_type;
	char *name;
} ;

static struct gpio_key key_irqs = {
	.gpio_pin = GPIO_KEY1_PIN,
	.name = KEY1_MISC_NAME,
};

static int key_press = 0;

static DECLARE_WAIT_QUEUE_HEAD(gpio_key_wait);

static irqreturn_t gpio_key_isr(int irq, void *dev_id)
{
	struct gpio_key *gpio_key = (struct gpio_key *)dev_id;
	int val;

	val = gpio_get_value(gpio_key->gpio_pin);

	printk("key %d %d\n", gpio_key->gpio_pin, val);
	gpio_key->pin_val = val;
	key_press = 1;

	wake_up_interruptible(&gpio_key_wait);

	return IRQ_HANDLED;
}

static ssize_t key_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	int key;

	printk("INFO(%s): key drv read !!!\n",__func__);
	wait_event_interruptible(gpio_key_wait, key_press);
	key = key_irqs.pin_val;

	copy_to_user(buf, &key, 4);
	key_press = 0;

	return 4;
}

static long key_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned char byte1,byte2;
	unsigned long val = 0;


	int ret;
	switch (cmd)
	{
		case CMD_GET_VAL:
			val = gpio_get_value(key_irqs.gpio_pin);
			copy_to_user((void __user *)arg, &val, sizeof(val));
			break;
	}
	return 0;
}

static struct file_operations key_fops = {
	.owner	 = THIS_MODULE,
	.unlocked_ioctl = key_drv_ioctl,
	.read    = key_drv_read,
};

static struct miscdevice key_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = KEY1_MISC_NAME,
	.fops = &key_fops,
};

static int hardware_init(void)
{
	int ret = 0;
	if (GPIO_KEY1_PIN != -1) {
		ret = gpio_request(GPIO_KEY1_PIN, "key1_irq");
		if (ret) {
			printk(KERN_ERR "can's request key1 irq\n");
			return ret;
		}
	}

	key_irqs.irq = GPIO_KEY1_IRQ;
	key_irqs.irq_type = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
	ret = request_irq(key_irqs.irq, gpio_key_isr, key_irqs.irq_type, key_irqs.name, &key_irqs);
	if (ret < 0) {
		printk(KERN_ERR "request_irq error!\n");
		free_irq(key_irqs.irq, &key_irqs);
		return ret;
	}

	return 0;
}

static int __init gpio_key_init(void)
{
	int ret = -1;

	ret = hardware_init();
	if (ret < 0) {
		printk("hardware_init error !\n");
		return ret ;
	}

	ret = misc_register(&key_misc);
	printk("gpio_key_init ok !!!\n");

	return ret;
}

static void __exit gpio_key_exit(void)
{
	free_irq(key_irqs.irq, &key_irqs);
	gpio_free(GPIO_KEY1_PIN);
	misc_deregister(&key_misc);
}

module_init(gpio_key_init);
module_exit(gpio_key_exit);

MODULE_LICENSE("GPL");


