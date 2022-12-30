/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <media/rc-core.h>
#include <media/gpio-ir-recv.h>
#include <linux/gpio_keys.h>

#define GPIO_IR_DRIVER_NAME	"gpio-ir-data"
//#define GPIO_IR_DEVICE_NAME	"gpio-ir-recv"

struct gpio_rc_dev {
	struct rc_dev *rcdev;
	int gpio_nr;
	bool active_low;
};

#ifdef CONFIG_OF
/*
 * Translate OpenFirmware node properties into platform_data
 */
static int gpio_ir_recv_get_devtree_pdata(struct device *dev,
				  struct gpio_ir_recv_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	enum of_gpio_flags flags;
	int gpio;
	
	printk("[INFO] event on\n");
	gpio = of_get_gpio_flags(np, 0, &flags);
	if (gpio < 0) {
		if (gpio != -EPROBE_DEFER)
			dev_err(dev, "Failed to get gpio flags (%d)\n", gpio);
		return gpio;
	}

	pdata->gpio_nr = gpio;
	pdata->active_low = (flags & OF_GPIO_ACTIVE_LOW);
	/* probe() takes care of map_name == NULL or allowed_protos == 0 */
	pdata->map_name = of_get_property(np, "linux,rc-map-name", NULL);
	pdata->allowed_protos = 0;

	return 0;
}

static struct of_device_id gpio_ir_recv_of_match[] = {
	{ .compatible = "gpio-ir-data", }, //保持一致
	{ },
};
MODULE_DEVICE_TABLE(of, gpio_ir_recv_of_match);

#else /* !CONFIG_OF */

#define gpio_ir_recv_get_devtree_pdata(dev, pdata)	(-ENOSYS)

#endif

/* 对应gpio 的中断函数 
   获取脉冲时间
   生成有效脉冲
   处理脉冲----对应键值
*/

static irqreturn_t gpio_ir_recv_irq(int irq, void *dev_id)
{
	struct gpio_rc_dev *gpio_dev = dev_id;
	int gval;
	int rc = 0;
	enum raw_event_type type = IR_SPACE;

	/*
	      获取 gpio 有效值        
	*/
	gval = gpio_get_value_cansleep(gpio_dev->gpio_nr);

	//printk("[INFO] %d\n",gval);

	if (gval < 0)
		goto err_get_value;

	/*
	        有的协议的引导码与 NEC 的相反，需要将active_low:true
	*/
	if (gpio_dev->active_low)
		gval = !gval;

	/*
	   收到的脉冲信号
	   space：无
	   pulse：脉冲
	*/
	if (gval == 1)
		type = IR_PULSE;

	//这个函数用来计算电平的持续时间
	rc = ir_raw_event_store_edge(gpio_dev->rcdev, type);
	if (rc < 0)
		goto err_get_value;
	//用来处理这个电平表示什么含义
	ir_raw_event_handle(gpio_dev->rcdev);

err_get_value:
	return IRQ_HANDLED;
}

/* 设备初始化
   配置gpio 的管脚/有效电平/支持的解码协议
   配置input event事件
   配置中断函数{
   IRQF_TRIGGER_FALLING：下降沿 
   IRQF_TRIGGER_RISING：上升沿
   }
*/

static int gpio_ir_recv_probe(struct platform_device *pdev)
{
	struct gpio_rc_dev *gpio_dev;
	struct rc_dev *rcdev;
	//获取gpio_ir_recv_platform_data结构体
	const struct gpio_ir_recv_platform_data *pdata =
					pdev->dev.platform_data;
	int rc;

	
	if (pdev->dev.of_node) {
		struct gpio_ir_recv_platform_data *dtpdata =
			devm_kzalloc(&pdev->dev, sizeof(*dtpdata), GFP_KERNEL);
		if (!dtpdata)
			return -ENOMEM;
		rc = gpio_ir_recv_get_devtree_pdata(&pdev->dev, dtpdata);
		if (rc)
			return rc;
		pdata = dtpdata;
		printk("[INFO] pdev->dev.of_node %d\n",pdev->dev.of_node);
	}

	if (!pdata)
		return -EINVAL;
	//判断管脚有效性
	if (pdata->gpio_nr < 0)
		return -EINVAL;

	gpio_dev = kzalloc(sizeof(struct gpio_rc_dev), GFP_KERNEL);
	if (!gpio_dev)
		return -ENOMEM;
	//配置input子系统
	rcdev = rc_allocate_device();
	if (!rcdev) {
		rc = -ENOMEM;
		goto err_allocate_device;
	}

	rcdev->priv = gpio_dev;
	rcdev->driver_type = RC_DRIVER_IR_RAW;
	rcdev->input_name = "gpio-ir-data";         //保持一致
	rcdev->input_phys = "gpio-ir-data/input0";  //保证一致
	rcdev->input_id.bustype = BUS_HOST;
	rcdev->input_id.vendor = 0x0001;
	rcdev->input_id.product = 0x0001;
	rcdev->input_id.version = 0x0100;
	rcdev->dev.parent = &pdev->dev;
	rcdev->driver_name = GPIO_IR_DRIVER_NAME;
	
	printk("[protos type] %d\n",rcdev->allowed_protos);
	//支持解码协议判断 allowed_protos:0 支持全部解码协议
	if (pdata->allowed_protos)
		rcdev->allowed_protos = pdata->allowed_protos;
	else
		rcdev->allowed_protos = RC_BIT_ALL;
	
	printk("[protos type] %d =? %d\n",rcdev->allowed_protos,RC_BIT_NEC);

	//按键映射默认使用 rc-empty
	rcdev->map_name = pdata->map_name ?: RC_MAP_EMPTY;

	printk("[map_name] %s \n",rcdev->map_name);

	gpio_dev->rcdev = rcdev;
	gpio_dev->gpio_nr = pdata->gpio_nr;
	gpio_dev->active_low = pdata->active_low;

	printk("[gpio info] gpio num: %d \n",pdata->gpio_nr);
	printk("[gpio info] gpio active_low: %d \n",pdata->active_low);
	
	/*
	   申请GPIO资源，配置GPIO为输入模式   
	*/
	rc = gpio_request(pdata->gpio_nr, "gpio-ir-data");
	if (rc < 0)
		goto err_gpio_request;
	rc  = gpio_direction_input(pdata->gpio_nr);
	if (rc < 0)
		goto err_gpio_direction_input;
	
        //input event 事件注册
	rc = rc_register_device(rcdev);
	if (rc < 0) {
		dev_err(&pdev->dev, "failed to register rc device\n");
		goto err_register_rc_device;
	}

	platform_set_drvdata(pdev, gpio_dev);
	
	//配置中断函数
	rc = request_any_context_irq(gpio_to_irq(pdata->gpio_nr),
				gpio_ir_recv_irq,
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
					"gpio-ir-recv-irq", gpio_dev);
	if (rc < 0)
		goto err_request_irq;

	return 0;

err_request_irq:
	platform_set_drvdata(pdev, NULL);
	rc_unregister_device(rcdev);
	rcdev = NULL;
err_register_rc_device:
err_gpio_direction_input:
	gpio_free(pdata->gpio_nr);
err_gpio_request:
	rc_free_device(rcdev);
err_allocate_device:
	kfree(gpio_dev);
	return rc;
}

static int gpio_ir_recv_remove(struct platform_device *pdev)
{
	struct gpio_rc_dev *gpio_dev = platform_get_drvdata(pdev);

	free_irq(gpio_to_irq(gpio_dev->gpio_nr), gpio_dev);
	platform_set_drvdata(pdev, NULL);
	rc_unregister_device(gpio_dev->rcdev);
	gpio_free(gpio_dev->gpio_nr);
	kfree(gpio_dev);
	return 0;
}

#ifdef CONFIG_PM
static int gpio_ir_recv_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct gpio_rc_dev *gpio_dev = platform_get_drvdata(pdev);

	if (device_may_wakeup(dev))
		enable_irq_wake(gpio_to_irq(gpio_dev->gpio_nr));
	else
		disable_irq(gpio_to_irq(gpio_dev->gpio_nr));

	return 0;
}

static int gpio_ir_recv_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct gpio_rc_dev *gpio_dev = platform_get_drvdata(pdev);

	if (device_may_wakeup(dev))
		disable_irq_wake(gpio_to_irq(gpio_dev->gpio_nr));
	else
		enable_irq(gpio_to_irq(gpio_dev->gpio_nr));

	return 0;
}

static const struct dev_pm_ops gpio_ir_recv_pm_ops = {
	.suspend        = gpio_ir_recv_suspend,
	.resume         = gpio_ir_recv_resume,
};
#endif

static struct platform_driver gpio_ir_recv_driver = {
	.probe  = gpio_ir_recv_probe,
	.remove = gpio_ir_recv_remove,
	.driver = {
		.name   = GPIO_IR_DRIVER_NAME,      //保持一致
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(gpio_ir_recv_of_match),
#ifdef CONFIG_PM
		.pm	= &gpio_ir_recv_pm_ops,
#endif
	},
};

/*
      添加 gpio_ir_recv_platform_data 资源，并注册到platform_device里面
*/

static struct gpio_ir_recv_platform_data  gpio_ir_buttons = {
	.gpio_nr         =  32,
	.active_low      =  false,
	.allowed_protos  =  RC_BIT_NEC,
	.map_name        = "rc-lirc",
};

struct platform_device Ares_button_device = {
	.name           = "gpio-ir-data",    //保证一致
	.id             = -1,
	.num_resources  = 0,
	.dev            = {
		.platform_data  = &gpio_ir_buttons,
	}
};


static int __init Ares_keys_init(void)
{
	platform_device_register(&Ares_button_device);
	//platform_device_register(&test_irp);
	return platform_driver_register(&gpio_ir_recv_driver);
}

static void __exit Ares_keys_exit(void)
{	
	platform_device_register(&Ares_button_device);
	//platform_device_register(&test_irp);
	platform_driver_unregister(&gpio_ir_recv_driver);
}

module_init(Ares_keys_init);
module_exit(Ares_keys_exit);

MODULE_DESCRIPTION("GPIO IR Receiver driver");
MODULE_LICENSE("GPL");
