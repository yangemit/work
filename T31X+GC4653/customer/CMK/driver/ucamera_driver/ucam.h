/*
 *	uvc_gadget.h  --  USB Video Class Gadget driver
 *
 *	Copyright (C) 2009-2010
 *	    Laurent Pinchart (laurent.pinchart@ideasonboard.com)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 */

#ifndef _U_CAM_H_
#define _U_CAM_H_

#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/usb/ch9.h>

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

struct Ucamera_Product_Cfg {
	int Pid;
	int Vid;
	int version;
	char manufacturer[64];
	char product[64];
	char serial[64];
};

int ucam_bind_uvc(struct usb_configuration *c);
int uvc_set_descriptor(unsigned long arg);
int uvc_sti_init(void);
int uvc_name_set(char *name);

int ucam_bind_uac(struct usb_configuration *c);
int uac_name_set(char *name);
int uac_set_descriptor(unsigned long arg);

#endif /* _U_CAM_H_ */

