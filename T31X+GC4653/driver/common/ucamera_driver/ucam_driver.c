/*
 *	ucam_driver.c -- USB camera gadget driver
 *
 *	Copyright (C) 2009-2010
 *	    Laurent Pinchart (laurent.pinchart@ideasonboard.com)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/usb/composite.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/mtd/direct_norflash.h>

#define INCLUDE_HID_INTERFACE
#define INCLUDE_ADB_INTERFACE

#ifdef INCLUDE_HID_INTERFACE
#include "f_hid.c"
#endif

#ifdef INCLUDE_ADB_INTERFACE
#include "f_adb.c"
#endif

#include "ucam.h"

USB_GADGET_COMPOSITE_OPTIONS();
/* --------------------------------------------------------------------------
 * Device descriptor
 */

static unsigned int UCAMERA_VENDOR_ID   =  0x0525;	/* Linux Foundation */
static unsigned int UCAMERA_PRODUCT_ID  =  0xa4ac;	/* Ucam A/V gadget */
static unsigned int UCAMERA_DEVICE_BCD  =  0x0100;	/* 0.10 */


#define STRING_DESCRIPTION_IDX		USB_GADGET_FIRST_AVAIL_IDX

static char ucamera_vendor_label[64] = "HID USB Camera";
static char ucamera_product_label[64] = "HID USB Camera";
static char ucamera_serial_label[64] = "HID Ucamera001";
static char ucamera_config_label[] = "ucamera";
char video_device_name[64] = "HID Web Camera";
char audio_device_name[64] = "Mic ucamera";

/* string IDs are assigned dynamically */

static int ucamera_inited = 0;
static int emergency = 0;
static unsigned int audio_start = 0;
static unsigned int adb_start = 0;
static unsigned int hid_start = 1;

static struct usb_string ucamera_strings[] = {
	[USB_GADGET_PRODUCT_IDX].s = ucamera_product_label,
	[USB_GADGET_MANUFACTURER_IDX].s = ucamera_vendor_label,
	[USB_GADGET_SERIAL_IDX].s = ucamera_serial_label,
	{  }
};

static struct usb_gadget_strings ucamera_stringtab = {
	.language = 0x0409,	/* en-us */
	.strings = ucamera_strings,
};

static struct usb_gadget_strings *ucamera_device_strings[] = {
	&ucamera_stringtab,
	NULL,
};

static struct usb_device_descriptor ucamera_device_descriptor = {
	.bLength		= USB_DT_DEVICE_SIZE,
	.bDescriptorType	= USB_DT_DEVICE,
	.bcdUSB			= cpu_to_le16(0x0200),
	.bDeviceClass		= USB_CLASS_MISC,
	.bDeviceSubClass	= 0x02,
	.bDeviceProtocol	= 0x01,
	.bMaxPacketSize0	= 0, /* dynamic */
	/* .idVendor		= cpu_to_le16(UCAMERA_VENDOR_ID), */
	/* .idProduct		= cpu_to_le16(UCAMERA_PRODUCT_ID), */
	/* .bcdDevice		= cpu_to_le16(UCAMERA_DEVICE_BCD), */
	.iManufacturer		= 0, /* dynamic */
	.iProduct		= 0, /* dynamic */
	.iSerialNumber		= 0, /* dynamic */
	.bNumConfigurations	= 0, /* dynamic */
};


#ifdef INCLUDE_HID_INTERFACE
/* hid descriptor for a keyboard */
static struct hidg_func_descriptor keyboard_hid_data = {
	.subclass		= 0, /* No subclass */
	.protocol		= 1, /* Keyboard */
	.report_length		= 8,
	.report_desc_length	= 63,
	.report_desc		= {
		0x05, 0x01,	/* USAGE_PAGE (Generic Desktop)	          */
		0x09, 0x06,	/* USAGE (Keyboard)                       */
		0xa1, 0x01,	/* COLLECTION (Application)               */
		0x05, 0x07,	/*   USAGE_PAGE (Keyboard)                */
		0x19, 0xe0,	/*   USAGE_MINIMUM (Keyboard LeftControl) */
		0x29, 0xe7,	/*   USAGE_MAXIMUM (Keyboard Right GUI)   */
		0x15, 0x00,	/*   LOGICAL_MINIMUM (0)                  */
		0x25, 0x01,	/*   LOGICAL_MAXIMUM (1)                  */
		0x75, 0x01,	/*   REPORT_SIZE (1)                      */
		0x95, 0x08,	/*   REPORT_COUNT (8)                     */
		0x81, 0x02,	/*   INPUT (Data,Var,Abs)                 */
		0x95, 0x01,	/*   REPORT_COUNT (1)                     */
		0x75, 0x08,	/*   REPORT_SIZE (8)                      */
		0x81, 0x03,	/*   INPUT (Cnst,Var,Abs)                 */
		0x95, 0x05,	/*   REPORT_COUNT (5)                     */
		0x75, 0x01,	/*   REPORT_SIZE (1)                      */
		0x05, 0x08,	/*   USAGE_PAGE (LEDs)                    */
		0x19, 0x01,	/*   USAGE_MINIMUM (Num Lock)             */
		0x29, 0x05,	/*   USAGE_MAXIMUM (Kana)                 */
		0x91, 0x02,	/*   OUTPUT (Data,Var,Abs)                */
		0x95, 0x01,	/*   REPORT_COUNT (1)                     */
		0x75, 0x03,	/*   REPORT_SIZE (3)                      */
		0x91, 0x03,	/*   OUTPUT (Cnst,Var,Abs)                */
		0x95, 0x06,	/*   REPORT_COUNT (6)                     */
		0x75, 0x08,	/*   REPORT_SIZE (8)                      */
		0x15, 0x00,	/*   LOGICAL_MINIMUM (0)                  */
		0x25, 0x65,	/*   LOGICAL_MAXIMUM (101)                */
		0x05, 0x07,	/*   USAGE_PAGE (Keyboard)                */
		0x19, 0x00,	/*   USAGE_MINIMUM (Reserved)             */
		0x29, 0x65,	/*   USAGE_MAXIMUM (Keyboard Application) */
		0x81, 0x00,	/*   INPUT (Data,Ary,Abs)                 */
		0xc0		/* END_COLLECTION                         */
	}
};


static struct hidg_func_descriptor led_hid_data = {
	.subclass		= 0, /* No subclass */
	.protocol		= 0, /* Keyboard */
	.report_length		= 1,
	.report_desc_length	= 29,
	.report_desc		 = {
		0x05, 0x08,                    // USAGE_PAGE (LEDs)
		0x09, 0x09,                    // USAGE (Mute)
		0xa1, 0x01,                    // COLLECTION (Application)
		0x05, 0x08,                    //   USAGE_PAGE (LEDs)
		0x19, 0x09,                    //   USAGE_MINIMUM (Mute)
		0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
		0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
		0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
		0x95, 0x01,                    //   REPORT_COUNT (1)
		0x75, 0x01,                    //   REPORT_SIZE (1)
		0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
		0x95, 0x01,                    //   REPORT_COUNT (1)
		0x75, 0x03,                    //   REPORT_SIZE (3)
		0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
		0xc0                           // END_COLLECTION
	}
};

static struct hidg_func_descriptor vendor_hid_data = {
	.subclass 		= 0,
	.protocol 		= 0,
	.report_length 		= 1024,
	.report_desc_length  	= 35,
	.report_desc 		= {
		0x05U, 0x81U,		// Usage Page (Vendor defined)
		0x09U, 0x82U,		// Usage (Vendor defined)
		0xA1U, 0x01U,		// Collection (Application)
		0x85U, 0x01U,		// REPORT_ID(1)
		0x09U, 0x82U,		// Usage (Vendor defined)
		0x15U, 0x80U,		// Logical Minimum (-128)
		0x25U, 0x7FU,		// Logical Maximum (127)
		0x75U, 0x08U,		// Report Size (8U)
		0x96U, 0xFFU, 0x03U,// Report Count (1023U)
		0x81U, 0x02U,		// Input(Data, Variable, Absolute)
		0x09U, 0x82U,		// Usage (Vendor defined)
		0x15U, 0x80U,		// Logical Minimum (-128)
		0x25U, 0x7FU,		// Logical Maximum (127)
		0x75U, 0x08U,		// Report Size (8U)
		0x96U, 0xFFU, 0x03U,// Report Count (1023U)
		0x91U, 0x02U,		// Output(Data, Variable, Absolute)
		0xC0U,				// End collection
	}
};

static struct platform_device my_hid = {
	.name			= "hidg",
	.id			= 0,
	.num_resources		= 0,
	.resource		= 0,
	.dev.platform_data	= &vendor_hid_data,
};

struct hidg_func_node {
	struct list_head node;
	struct hidg_func_descriptor *func;
};
static LIST_HEAD(hidg_func_list);


static int hidg_plat_driver_probe(struct platform_device *pdev)
{
	struct hidg_func_descriptor *func = pdev->dev.platform_data;
	struct hidg_func_node *entry;

	if (!func) {
		dev_err(&pdev->dev, "Platform data missing\n");
		return -ENODEV;
	}

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return -ENOMEM;

	entry->func = func;
	list_add_tail(&entry->node, &hidg_func_list);

	return 0;
}

static int hidg_plat_driver_remove(struct platform_device *pdev)
{
	struct hidg_func_node *e, *n;

	list_for_each_entry_safe(e, n, &hidg_func_list, node) {
		list_del(&e->node);
		kfree(e);
	}

	return 0;
}

static struct platform_driver hidg_plat_driver = {
	.remove		= hidg_plat_driver_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "hidg",
	},
};

static int hid_init(struct usb_composite_dev *cdev)
{
	struct list_head *tmp;
	int status, funcs = 0;

	list_for_each(tmp, &hidg_func_list)
		funcs++;
	if (!funcs)
		return -ENODEV;

	status = ghid_setup(cdev->gadget, funcs);
	if (status < 0)
		return status;
	return 0;
}

static void hid_cleanup(struct usb_composite_dev *cdev)
{
	ghid_cleanup();
	return;
}

#endif



int ucamera_bind_hid(struct usb_configuration *c)
{
	int status = 0;

#ifdef INCLUDE_HID_INTERFACE
	struct hidg_func_node *e;
	int func = 0;

	list_for_each_entry(e, &hidg_func_list, node) {
		status = hidg_bind_config(c, e->func, func++);
		if (status)
			break;
	}
#endif
	return status;
}

int ucamera_bind_adb(struct usb_configuration *c)
{
	int ret = 0;

#ifdef INCLUDE_ADB_INTERFACE
	ret = adb_bind_config(c);
#endif
	return ret;
}

/* --------------------------------------------------------------------------
 * USB configuration
 */


static int
ucamera_config_bind(struct usb_configuration *c)
{
	int ret;

	ret = ucam_bind_uvc(c);
	if(ret)
		printk("ucamera bind uvc func failed\n");

	if (1 == audio_start) {
		ret = ucam_bind_uac(c);
		if(ret)
			printk("ucamera bind uac func failed\n");
	}

	if (1 == hid_start) {
		ret = ucamera_bind_hid(c);
		if(ret)
			printk("ucamera bind hid func failed\n");
		if(emergency){
			printk("emergency start up \n");
			return 0;
		}
	}
#ifdef INCLUDE_ACM_INTERFACE
	ret = ucam_bind_acm(c);
		if(ret)
			printk("ucamera bind acm func failed\n");
#endif

	if (1 == adb_start) {
		ret = ucamera_bind_adb(c);
		if(ret)
			printk("ucamera bind adb func failed\n");
	}

	return ret;
}

static struct usb_configuration ucamera_config_driver = {
	.label			= ucamera_config_label,
	.bConfigurationValue	= 1,
	.iConfiguration		= 0, /* dynamic */
	.bmAttributes		= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.MaxPower		= 0xC8,
};

static int /* __init_or_exit */
ucamera_unbind(struct usb_composite_dev *cdev)
{
#ifdef INCLUDE_HID_INTERFACE
	if (hid_start != 0)
		hid_cleanup(cdev);
#endif

#ifdef INCLUDE_ADB_INTERFACE
	if (1 == adb_start)
		adb_cleanup();
#endif
#ifdef INCLUDE_ACM_INTERFACE
	acm_cleanup();
#endif
	return 0;
}

static int
ucamera_bind(struct usb_composite_dev *cdev)
{
	int ret;

	/* Allocate string descriptor numbers ... note that string contents
	 * can be overridden by the composite_dev glue.
	 */
	ret = usb_string_ids_tab(cdev, ucamera_strings);
	if (ret < 0)
		goto error;
	ucamera_device_descriptor.iManufacturer =
		ucamera_strings[USB_GADGET_MANUFACTURER_IDX].id;
	ucamera_device_descriptor.iProduct =
		ucamera_strings[USB_GADGET_PRODUCT_IDX].id;
	ucamera_device_descriptor.iSerialNumber =
		ucamera_strings[USB_GADGET_SERIAL_IDX].id;

#ifdef INCLUDE_HID_INTERFACE
	if (hid_start != 0) {
		ret = hid_init(cdev);
		if (ret)
			printk("%s: Failed to init hid", __func__);

	}
#endif

#ifdef INCLUDE_ADB_INTERFACE
	if (1 == adb_start) {
		ret =  adb_setup();
		if (ret)
			printk("%s: Failed to init adb", __func__);
	}
#endif

#ifdef INCLUDE_ACM_INTERFACE
	ret =  acm_mod_init();
		if (ret)
			printk("%s: Failed to init acm", __func__);
#endif

	/* Register our configuration. */
	if ((ret = usb_add_config(cdev, &ucamera_config_driver,
					ucamera_config_bind)) < 0)
		goto error;

	usb_composite_overwrite_options(cdev, &coverwrite);
	return 0;

error:
	ucamera_unbind(cdev);
	return ret;
}

/* --------------------------------------------------------------------------
 * Driver
 */

static __refdata struct usb_composite_driver ucamera_driver = {
	.name		= "g_camera",
	.dev		= &ucamera_device_descriptor,
	/* .dev		= &adb_device_desc, */
	.strings	= ucamera_device_strings,
	.max_speed	= USB_SPEED_SUPER,
	.bind		= ucamera_bind,
	.unbind		= ucamera_unbind,
};

static int
ucamera_driver_init(void)
{
	int status;

#ifdef INCLUDE_HID_INTERFACE
	if (hid_start != 0) {
		status = platform_device_register(&my_hid);
		if (status < 0) {
			printk("%s hid register failed\n", __func__);
			platform_device_unregister(&my_hid);
			return status;
		}

		status = platform_driver_probe(&hidg_plat_driver,
					       hidg_plat_driver_probe);
		if (status < 0)
			return status;

	}
#endif

	status = usb_composite_probe(&ucamera_driver);
	if (status < 0)
		return status;

	return status;
}

static void
ucamera_driver_cleanup(void)
{
#ifdef INCLUDE_HID_INTERFACE
	if (hid_start) {
		platform_driver_unregister(&hidg_plat_driver);
		platform_device_unregister(&my_hid);

	}
#endif
	usb_composite_unregister(&ucamera_driver);
}


int ucamera_product_msg_config(unsigned long arg)
{
	int ret = 0;
	struct Ucamera_Product_Cfg *pcfg;

	pcfg = kmalloc(sizeof(struct Ucamera_Product_Cfg), GFP_KERNEL);
	ret = copy_from_user(pcfg, (void __user*)arg, sizeof(struct Ucamera_Product_Cfg));
	if (ret) {
		printk("error(%s, %d): failed to copy_from_user!\n", __func__, __LINE__);
		ret = -EIO;
		return ret;
	}

	if (pcfg->Pid)
		UCAMERA_PRODUCT_ID = pcfg->Pid;
	if (pcfg->Vid)
		UCAMERA_VENDOR_ID = pcfg->Vid;
	if (pcfg->version)
		UCAMERA_DEVICE_BCD = pcfg->version;

	memcpy(ucamera_vendor_label, pcfg->manufacturer, 64);
	memcpy(ucamera_product_label, pcfg->product, 64);
	memcpy(ucamera_serial_label, pcfg->serial, 64);

	memcpy(video_device_name, pcfg->video_name, 64);
	memcpy(audio_device_name, pcfg->audio_name, 64);

	ucamera_device_descriptor.idVendor = cpu_to_le16(UCAMERA_VENDOR_ID);
	ucamera_device_descriptor.idProduct = cpu_to_le16(UCAMERA_PRODUCT_ID);
	ucamera_device_descriptor.bcdDevice = cpu_to_le16(UCAMERA_DEVICE_BCD);
	uvc_name_set(video_device_name);
	uac_name_set(audio_device_name);
	return ret;
}

static long ucamera_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0, tmp = 0;
	hwvreg_write_paras hwvreg_paras;
	void __user *argp = (void __user *)arg;


	switch (cmd) {
	case UCAMERA_IOCTL_VIDEO_CFG:
		ret = uvc_set_descriptor(arg);
		break;
	case UCAMERA_IOCTL_AUDIO_CFG:
		ret = uac_set_descriptor(arg);
		break;
	case UCAMERA_IOCTL_PRODUCT_CFG:
		ret = ucamera_product_msg_config(arg);
		break;
	case UCAMERA_IOCTL_DRIVER_INIT:
		if (0 == ucamera_inited) {
			ret = ucamera_driver_init();
			if (0 == ret) {
				ucamera_inited = 1;
			} else {
				printk("error(%s, %d): ucamera_init\n", __func__, __LINE__);
			}
		}
		break;
	case UCAMERA_IOCTL_AUDIO_ENABLE:
		audio_start = 1;
		break;
	case UCAMERA_IOCTL_DRIVER_DEINIT:
		ucamera_driver_cleanup();
		break;
	case UCAMERA_IOCTL_ADB_ENABLE:
		adb_start = 1;
		break;
	case UCAMERA_IOCTL_HID_EMER_ENABLE:
		if(0 == ucamera_inited && 0 == emergency){
			struct Ucamera_Product_Cfg *pcfg;
			pcfg = kmalloc(sizeof(struct Ucamera_Product_Cfg), GFP_KERNEL);

			ucamera_device_descriptor.idVendor = cpu_to_le16(UCAMERA_VENDOR_ID + 1);
			ucamera_device_descriptor.idProduct = cpu_to_le16(UCAMERA_PRODUCT_ID + 2);
			ucamera_device_descriptor.bcdDevice = cpu_to_le16(UCAMERA_DEVICE_BCD);
			ret = ucamera_driver_init();
			if (0 == ret) {
				ucamera_inited = 1;
				emergency = 1;
			} else {
				printk("error(%s, %d): ucamera_init\n", __func__, __LINE__);
			}
		}
		break;
	case UCAMERA_IOCTL_STILL_IMG_CAP:
		uvc_sti_init();
		break;
	case UCAMERA_IOCTL_SET_FLAG:
		ret = copy_from_user(&hwvreg_paras, (hwvreg_write_paras *)argp, sizeof(hwvreg_write_paras));
		if (ret) {
			printk("(%s) ERROR: copy hwreg waddr failed!\n", __func__);
			ret = -EINVAL;
			break;
		}
		direct_erase_norflash(hwvreg_paras.offset, hwvreg_paras.len);
		direct_write_norflash(&hwvreg_paras.value, hwvreg_paras.offset, hwvreg_paras.len);

		break;
	case UCAMERA_IOCTL_READ_FLAG:
		ret = copy_from_user(&hwvreg_paras, argp, sizeof(hwvreg_write_paras));
		if (ret) {
			printk("(%s) ERROR: copy hwreg raddr failed!\n", __func__);
			ret = -EINVAL;
			break;
		}
		direct_read_norflash(&hwvreg_paras.value, hwvreg_paras.offset, hwvreg_paras.len);
		/*tmp = *(volatile unsigned int *)raddr;*/
		ret = copy_to_user((void *)arg, &hwvreg_paras, sizeof(hwvreg_write_paras));
		if (ret) {
			printk("(%s) ERROR: copy hwreg value to user failed!\n", __func__);
			ret = -EINVAL;
		}
		break;

	case UCAMERA_IOCTL_WRITE_NORFLASH:
		{
			hwvreg_write_data hwvreg_data;
			ret = copy_from_user(&hwvreg_data, (hwvreg_write_data *)argp, sizeof(hwvreg_write_data));
			if (ret) {
				printk("(%s) ERROR: copy hwreg waddr failed!\n", __func__);
				ret = -EINVAL;
				break;
			}

			direct_erase_norflash(hwvreg_data.offset, hwvreg_data.len);
			direct_write_norflash(hwvreg_data.data, hwvreg_data.offset, hwvreg_data.len);
		}
		break;

	case UCAMERA_IOCTL_READ_NORFLASH:
		{
			hwvreg_write_data hwvreg_data;
			ret = copy_from_user(&hwvreg_data, argp, sizeof(hwvreg_write_data));
			if (ret) {
				printk("(%s) ERROR: copy hwreg raddr failed!\n", __func__);
				ret = -EINVAL;
			}
			direct_read_norflash(hwvreg_data.data, hwvreg_data.offset, hwvreg_data.len);
			/*tmp = *(volatile unsigned int *)raddr;*/
			ret = copy_to_user((void *)arg, &hwvreg_data, sizeof(hwvreg_write_data));
			if (ret) {
				printk("(%s) ERROR: copy hwreg value to user failed!\n", __func__);
				ret = -EINVAL;
			}
		}
		break;

	default:
		printk("invalid command: 0x%08x\n", cmd);
		ret = -EINVAL;
	}
	return ret;
}

static int ucamera_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int ucamera_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t ucamera_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	int ret = 0;
	ret = copy_to_user(buf, &ucamera_inited, sizeof(ucamera_inited));
	if(ret != 0)
		printk("ERROR:copy to user if failed\n");
	return ret;
}

static struct file_operations ucamera_fops =
{
	.owner = THIS_MODULE,
	.read = ucamera_read,
	.unlocked_ioctl = ucamera_ioctl,
	.open = ucamera_open,
	.release = ucamera_release,
};

static struct miscdevice ucamera_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ucamera",
	.fops = &ucamera_fops,
};

static int __init ucamera_init(void)
{
	int ret = 0;
	ret = misc_register(&ucamera_misc);
	return ret;

}

static void __exit ucamera_exit(void)
{
	misc_deregister(&ucamera_misc);
}

module_init(ucamera_init);
module_exit(ucamera_exit);

MODULE_AUTHOR("Laurent Pinchart");
MODULE_DESCRIPTION("Ucamera Video Gadget");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1.0");

