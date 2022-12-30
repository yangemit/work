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

#ifndef _UVC_GADGET_H_
#define _UVC_GADGET_H_

#include <stdint.h>
#include <linux/ioctl.h>
//#include <linux/types.h>
#include <linux/usb/ch9.h>
#include <linux/videodev2.h>

#include "video.h"
#include "usbcamera.h"

#define UVC_EVENT_FIRST			(V4L2_EVENT_PRIVATE_START + 0)
#define UVC_EVENT_CONNECT		(V4L2_EVENT_PRIVATE_START + 0)
#define UVC_EVENT_DISCONNECT		(V4L2_EVENT_PRIVATE_START + 1)
#define UVC_EVENT_STREAMON		(V4L2_EVENT_PRIVATE_START + 2)
#define UVC_EVENT_STREAMOFF		(V4L2_EVENT_PRIVATE_START + 3)
#define UVC_EVENT_SETUP			(V4L2_EVENT_PRIVATE_START + 4)
#define UVC_EVENT_DATA			(V4L2_EVENT_PRIVATE_START + 5)
#define UVC_EVENT_LAST			(V4L2_EVENT_PRIVATE_START + 5)

#define UVC_INPUT_TERMINAL_ID		1
#define UVC_PROCESSING_UNIT_ID		2
#define UVC_EXTENSION_UNIT_ID		4


#define UCAMERA_DEV 				"/dev/ucamera"
#define UCAMERA_IOCTL_MAGIC  			'U'
#define UCAMERA_IOCTL_FORMAT_CFG		_IO(UCAMERA_IOCTL_MAGIC, 1)
#define UCAMERA_IOCTL_START			_IO(UCAMERA_IOCTL_MAGIC, 2)
#define UCAMERA_IOCTL_STOP			_IO(UCAMERA_IOCTL_MAGIC, 3)
#define UCAMERA_IOCTL_AUDIO_ENABLE		_IO(UCAMERA_IOCTL_MAGIC, 4)
#define UCAMERA_IOCTL_AUDIO_CFG			_IO(UCAMERA_IOCTL_MAGIC, 5)
#define UCAMERA_IOCTL_ADB_ENABLE		_IO(UCAMERA_IOCTL_MAGIC, 6)
#define UCAMERA_IOCTL_PRODUCT_INFO_CFG		_IO(UCAMERA_IOCTL_MAGIC, 7)
#define UCAMERA_IOCTL_HID_ENABLE		_IO(UCAMERA_IOCTL_MAGIC, 8)
#define UCAMERA_IOCTL_STILL_IMG_CAP_EN		_IO(UCAMERA_IOCTL_MAGIC, 9)

#define UVCIOC_STILL_IMAGE_CAP          	_IO('U', 17)

#define UCAMERA_VIDEO_DEV 			"/dev/video0"
#define UCAMERA_AUDIO_DEV			"/dev/gaudio"


#define UCAMERA_HID_DEV				"/dev/hidg0"

struct uvc_request_data
{
	__s32 length;
	__u8 data[60];
};

struct uvc_event
{
	union {
		enum usb_device_speed speed;
		struct usb_ctrlrequest req;
		struct uvc_request_data data;
	};
};

/* IO methods supported */
enum io_method {
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
};

/* Buffer representing one video frame */
struct buffer {
	struct v4l2_buffer buf;
	void *start;
	size_t length;
};

/* ---------------------------------------------------------------------------
 * UVC specific stuff
 */

struct uvc_frame_info {
	unsigned int width;
	unsigned int height;
	unsigned int intervals[4];
};

struct uvc_format_info {
	unsigned int fcc;
	unsigned int nframes;
	struct uvc_frame_info *frames;
};

struct uvc_streaming_still_control
{
	__u8 bFormatIndex;
	__u8 bFrameIndex;
	__u8 wCompQuality;
	__u32 dwMaxVideoFrameSize;
	__u32 dwMaxPayloadTransferSize;
}__attribute__((__packed__));

typedef struct _gaudio_fifo_list_info {
	unsigned int cnt;
	unsigned int size;
	void  *base;
	unsigned int data;
} gaudio_fifo_list_info_t;

/* Represents a UVC based video output device */
struct uvc_device {
	/* uvc device specific */
	int uvc_fd;
	int uac_fd;
	int hid_fd;
	int is_streaming;
	int sdk_inited;
	int run_standalone;
	char *uvc_devname;

	/* uvc control request specific */
	struct uvc_streaming_control probe;
	struct uvc_streaming_control commit;
	struct uvc_streaming_still_control still;
	int control;
	struct uvc_request_data request_error_code;
	unsigned int brightness_val;

	/* uvc buffer specific */
	enum io_method io;
	struct buffer *mem;
	struct buffer *dummy_buf;
	unsigned int nbufs;
	unsigned int fcc;
	unsigned int width;
	unsigned int height;
	unsigned int interval;

	unsigned int bulk;
	uint8_t color;
	unsigned int imgsize;
	void *imgdata;
	void *audio_buf;

	/* USB speed specific */
	int mult;
	int burst;
	int maxpkt;
	enum usb_device_speed speed;

	/* uvc specific flags */
	int first_buffer_queued;
	int uvc_shutdown_requested;

	/* uvc buffer queue and dequeue counters */
	unsigned long long int qbuf_count;
	unsigned long long int dqbuf_count;

	int uvc_event;	/*0:inited; 1:stream on; 2:stream off*/

	struct Ucamera_Video_CB_Func *v_func;
	struct Ucamera_Audio_CB_Func *a_func;

	int (*event_process)(int event_id, void *data);
	struct Ucamera_Video_PU_Control v_puctl[16];
	struct Ucamera_Video_CT_Control v_ctctl[20];
	struct Ucamera_Video_EU_Control v_euctl;
	unsigned char puctl_cur;
	unsigned char unit_cur;
	void *audio_base_addr_mmap;
	unsigned int audio_base_addr_phy;
	gaudio_fifo_list_info_t fifo_info;

};

struct gaudio_event {
	int type;
	int value;
	char data[8];
};

#define GAUDIO_IOCTL_MAGIC  			'A'
#define GAUDIO_IOCTL_GET_EVENT			_IO(GAUDIO_IOCTL_MAGIC, 1)
#define GAUDIO_IOCTL_GET_MMAP_INFO		_IO(GAUDIO_IOCTL_MAGIC, 2)
#define GAUDIO_IOCTL_SET_MMAP_INFO		_IO(GAUDIO_IOCTL_MAGIC, 3)
#define GAUDIO_IOCTL_GET_MMAP_ADDR		_IO(GAUDIO_IOCTL_MAGIC, 4)
#define GAUDIO_IOCTL_DEQUEUE_BUF		_IO(GAUDIO_IOCTL_MAGIC, 5)
#define GAUDIO_IOCTL_QUEUE_BUF			_IO(GAUDIO_IOCTL_MAGIC, 6)


#define UAC_EVENT_SET_SPK_MUTE			1
#define UAC_EVENT_SET_SPK_VOLUME		2
#define UAC_EVENT_SET_MIC_MUTE			3
#define UAC_EVENT_SET_MIC_VOLUME		4
#define UAC_EVENT_SPEAK_ON			5
#define UAC_EVENT_SPEAK_OFF			6
#define UAC_EVENT_RECORD_ON			7
#define UAC_EVENT_RECORD_OFF			8

#define UVCIOC_SEND_RESPONSE			_IOW('U', 1, struct uvc_request_data)

#define UVC_INTF_CONTROL			0
#define UVC_INTF_STREAMING			1



//#define SUPPORT_UAC_FUNC
#endif /* _UVC_GADGET_H_ */

