/* drivers/misc/dw9714.c
 *
 * Copyright (c) 2017.03  Ingenic Semiconductor Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/syscalls.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/dma-mapping.h>
#include <linux/input.h>
#include <linux/input-polldev.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/time.h>
#include <soc/gpio.h>
#include <linux/module.h>
#include "dw9714.h"

#define DEBUG
#define DW9714_ADDR			0x0c

#define MOTOR_MOVE			_IOW('M', 1, int)

/** this is set according to the DW9714 chip XSD pin**/
//#define SW9714_XSD_GPIO 		GPIO_PB(10)
#define SW9714_XSD_GPIO 		GPIO_PB(19)

#define I2C_ADAPTER_ID 0

#ifdef DEBUG
#define dbg(format, arg...) printk(KERN_INFO "[%s : %d ]" format , __func__, __LINE__, ## arg)
#else
#define dbg(format, arg...) do{ }while(0)
#endif

struct dw9714_dev {
	struct device *dev;
	struct i2c_client *client;
	struct miscdevice mdev;
	spinlock_t		lock;
};

static int dw9714_init_config(struct i2c_client *client)
{
	int ret = 0;
	int dac_value = 100;


	ret = i2c_smbus_write_byte_data(client, 0xec, 0xa3);
	if (ret) {
		printk("i2c_smbus_write_byte_data err \n");
		goto err_i2c_write;
	}

	ret = i2c_smbus_write_byte_data(client, 0xa1, 0x0d);
	if (ret) {
		printk("i2c_smbus_write_byte_data err \n");
		goto err_i2c_write;
	}

	ret = i2c_smbus_write_byte_data(client, 0xf2, 0x00);
	if (ret) {
		printk("i2c_smbus_write_byte_data err \n");
		goto err_i2c_write;
	}

	ret = i2c_smbus_write_byte_data(client, 0xdc, 0x51);
	if (ret) {
		printk("i2c_smbus_write_byte_data err \n");
		goto err_i2c_write;
	}

	ret = i2c_smbus_write_byte_data(client, (dac_value >> 4), (dac_value & 0x0f ) << 4);
	if (ret) {
		printk("i2c_smbus_write_byte_data err \n");
		goto err_i2c_write;
	}
	printk("dac_value is %d \n" , dac_value );

	return ret;

err_i2c_write:
	printk("i2c_smbus_write_byte_data err!\n");
	return ret;
}

static int dw9714_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int dw9714_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int dw9714_read(struct file *filp, char __user *buf, size_t size, loff_t *off)
{
	return 0;
}

static long dw9714_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct miscdevice *dev = filp->private_data;

	struct dw9714_dev *dw9714 = container_of(dev , struct dw9714_dev , mdev);

	int ret;
	switch (cmd)
	{
		case MOTOR_MOVE:
			ret = i2c_smbus_write_byte_data(dw9714->client, (arg >> 4), (arg & 0x0f ) << 4);
			if (ret) {
				printk("i2c_smbus_write_byte_data err \n");
				return ret;
			}
			break;
	}
	return 0;
}

static struct file_operations dw9714_fp = {
	.owner	= THIS_MODULE,
	.open	= dw9714_open,
	.release= dw9714_release,
	.read	= dw9714_read,
	.unlocked_ioctl = dw9714_ioctl,
};

static int dw9714_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;

	struct dw9714_dev *dw9714;

	dw9714 = kzalloc(sizeof(struct dw9714_dev), GFP_KERNEL);
	if (!dw9714) {
		pr_err("kzalloc dw9714 memery error\n");
		return -ENOMEM;
	}

	dw9714->dev = &client->dev;


	dw9714->client = client;

	i2c_set_clientdata(client, dw9714);

	gpio_request(SW9714_XSD_GPIO,"sw9714_xsd");
	gpio_direction_output(SW9714_XSD_GPIO,0);
	msleep(1);
	gpio_direction_output(SW9714_XSD_GPIO,1);
	msleep(1);
	if ((ret = dw9714_init_config(client))) {
		dev_err(dw9714->dev, "dw9714_init_set  error\n");
		goto err_i2c;
	}

	dw9714->mdev.minor = MISC_DYNAMIC_MINOR;
	dw9714->mdev.name = DW9714_NAME;
	dw9714->mdev.fops = &dw9714_fp;
	if ((ret = misc_register(&dw9714->mdev)) < 0) {
		dev_err(dw9714->dev, "misc register failed\n");
		goto err_misc;
	}else {
		dev_err(dw9714->dev, "misc_deregister is ok!\n");
	}

	return 0;

err_misc:
err_i2c:
	i2c_set_clientdata(client, NULL);

	return ret;
}

static int dw9714_remove(struct i2c_client *client)
{
	int ret = 0;
	struct dw9714_dev *dw9714 = i2c_get_clientdata(client);

	misc_deregister(&dw9714->mdev);
	i2c_set_clientdata(client, NULL);
	kfree(dw9714);

	return ret;
}

static const struct i2c_device_id dw9714_id[] = {
	{"dw9714", 0x0f},
	{},
};

static struct of_device_id dw9714_dt_match[] = {
	{.compatible = "silan, dw9714"},
	{},
};

static struct i2c_driver dw9714_driver = {
	.driver = {
		.name = "dw9714-input",
		.owner = THIS_MODULE,
		.of_match_table = dw9714_dt_match,
	},
	.probe = dw9714_probe,
	.remove = dw9714_remove,
	.id_table = dw9714_id,
};

struct i2c_board_info jz_i2c1_dw9714_dev[] __initdata = {
	{
		I2C_BOARD_INFO("dw9714", DW9714_ADDR),
	},
};

static struct i2c_client *device_client;

static int __init dw9714_init(void)
{
	struct i2c_adapter *adapter;
	adapter = i2c_get_adapter(I2C_ADAPTER_ID);
	if (!adapter) {
		printk("dw9714: failed to get I2C adapter %d\n", I2C_ADAPTER_ID);
	}

	device_client = i2c_new_device(adapter, jz_i2c1_dw9714_dev);
	if (device_client == NULL) {
		printk("dw9714: i2c new device fail!\n");
		return -EINVAL;
	}

	return i2c_add_driver(&dw9714_driver);
}

static void __exit dw9714_exit(void)
{
	i2c_del_driver(&dw9714_driver);
	i2c_unregister_device(device_client);
}

module_init(dw9714_init);
module_exit(dw9714_exit);

MODULE_DESCRIPTION("dw9714 driver");
MODULE_LICENSE("GPL");
