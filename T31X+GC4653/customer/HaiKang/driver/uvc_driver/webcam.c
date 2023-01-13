/*
 *	webcam.c -- USB webcam gadget driver
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
#include "video.h"

#include "f_uvc.h"

/*
 * Kbuild is not very cooperative with respect to linking separately
 * compiled library objects into one module.  So for now we won't use
 * separate compilation ... ensuring init/exit sections work to shrink
 * the runtime footprint, and giving us at least some parts of what
 * a "gcc --combine ... part1.c part2.c part3.c ... " build would.
 */
#include "uvc_queue.c"
#include "uvc_video.c"
#include "uvc_v4l2.c"
#include "f_uvc.c"


int af_en = 0;
int uvc_extension_en = 0;
module_param(uvc_extension_en, int, S_IRUGO);

static struct uvc_descriptor_header ** ucamera_streaming_cls = NULL;
static unsigned int stillimg_en = 0;

/* --------------------------------------------------------------------------
 * Device descriptor
 */


DECLARE_UVC_HEADER_DESCRIPTOR(1);

static const struct UVC_HEADER_DESCRIPTOR(1) uvc_control_header = {
	.bLength		= UVC_DT_HEADER_SIZE(1),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VC_HEADER,
	.bcdUVC			= cpu_to_le16(0x0100),
	.wTotalLength		= 0, /* dynamic */
	.dwClockFrequency	= cpu_to_le32(48000000),
	.bInCollection		= 0, /* dynamic */
	.baInterfaceNr[0]	= 0, /* dynamic */
	/*what about 2 interfaceNr ?*/
};

static struct uvc_camera_terminal_descriptor uvc_camera_terminal = {
	.bLength		= UVC_DT_CAMERA_TERMINAL_SIZE(3),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VC_INPUT_TERMINAL,
	.bTerminalID		= 1,
	.wTerminalType		= cpu_to_le16(0x0201),
	.bAssocTerminal		= 0,
	.iTerminal		= 0,
	.wObjectiveFocalLengthMin	= cpu_to_le16(0),
	.wObjectiveFocalLengthMax	= cpu_to_le16(0),
	.wOcularFocalLength		= cpu_to_le16(0),
	.bControlSize		= 3,
	.bmControls[0] = 0x2a,
	.bmControls[1] = 0x22,
	.bmControls[2] = 0x02,
};

static const struct uvc_processing_unit_descriptor uvc_processing = {
	.bLength		= UVC_DT_PROCESSING_UNIT_SIZE(2),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VC_PROCESSING_UNIT,
	.bUnitID		= 2,
	.bSourceID		= 1,
	.wMaxMultiplier		= cpu_to_le16(16*1024),
	.bControlSize		= 2,
	.bmControls[0]		= 0xDF,
	.bmControls[1]		= 0x13,
	.iProcessing		= 0,
};

DECLARE_UVC_EXTENSION_UNIT_DESCRIPTOR(1, 2);

static const struct UVC_EXTENSION_UNIT_DESCRIPTOR(1, 2) uvc_extersion_unit = {
	.bLength 		= UVC_DT_EXTENSION_UNIT_SIZE(1, 2),
	.bDescriptorType 	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType 	= UVC_VC_EXTENSION_UNIT,
	.bUnitID 		= 4,
	.guidExtensionCode 	=
		{0x41, 0x76, 0x9e, 0xa2, 0x04, 0xde, 0xe3, 0x47,
		0x8b, 0x2b, 0xf4, 0x34, 0x1a, 0xff, 0x00, 0x3b},
	.bNumControls 		= 0x0F,
	.bNrInPins 		= 1,
	.baSourceID 		= 2,
	.bControlSize 		= 0x02,
	.bmControls[0] 		= 0xff,
	.bmControls[1] 		= 0x0,
	.iExtension 		= 0,
};

static struct uvc_output_terminal_descriptor uvc_output_terminal = {
	.bLength		= UVC_DT_OUTPUT_TERMINAL_SIZE,
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VC_OUTPUT_TERMINAL,
	.bTerminalID		= 3,
	.wTerminalType		= cpu_to_le16(0x0101),
	.bAssocTerminal		= 0,
	.bSourceID		= 2,
	.iTerminal		= 0,
};

DECLARE_UVC_INPUT_HEADER_DESCRIPTOR(1, 3);

static struct UVC_INPUT_HEADER_DESCRIPTOR(1, 3) uvc_input_header = {
	.bLength		= UVC_DT_INPUT_HEADER_SIZE(1, 3),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_INPUT_HEADER,
	.bNumFormats		= 3,
	.wTotalLength		= 0, /* dynamic */
	.bEndpointAddress	= 0, /* dynamic */
	.bmInfo			= 0,
	.bTerminalLink		= 3,
	.bStillCaptureMethod	= 0,
	.bTriggerSupport	= 0,
	.bTriggerUsage		= 0,
	.bControlSize		= 1,
	.bmaControls[0][0]	= 0,
	.bmaControls[1][0]	= 4,
};

DECLARE_UVC_INPUT_HEADER_DESCRIPTOR(1, 2);
static struct UVC_INPUT_HEADER_DESCRIPTOR(1, 2) uvc_input_header2 = {
	.bLength		= UVC_DT_INPUT_HEADER_SIZE(1, 2),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_INPUT_HEADER,
	.bNumFormats		= 2,
	.wTotalLength		= 0, /* dynamic */
	.bEndpointAddress	= 0, /* dynamic */
	.bmInfo			= 0,
	.bTerminalLink		= 3,
	.bStillCaptureMethod	= 0,
	.bTriggerSupport	= 0,
	.bTriggerUsage		= 0,
	.bControlSize		= 1,
	.bmaControls[0][0]	= 0,
	.bmaControls[1][0]	= 4,
};

static struct uvc_format_uncompressed uvc_format_yuv = {
	.bLength		= UVC_DT_FORMAT_UNCOMPRESSED_SIZE,
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FORMAT_UNCOMPRESSED,
	.bFormatIndex		= 1,
	.bNumFrameDescriptors	= 2,
	.guidFormat		=
		{ 'Y',  'U',  'Y',  '2', 0x00, 0x00, 0x10, 0x00,
		 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71},
	.bBitsPerPixel		= 16,
	.bDefaultFrameIndex	= 1,
	.bAspectRatioX		= 0,
	.bAspectRatioY		= 0,
	.bmInterfaceFlags	= 0,
	.bCopyProtect		= 0,
};

static struct uvc_format_uncompressed uvc_format_nv12 = {
	.bLength		= UVC_DT_FORMAT_UNCOMPRESSED_SIZE,
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FORMAT_UNCOMPRESSED,
	.bFormatIndex		= 1,
	.bNumFrameDescriptors	= 2,
	.guidFormat		=
		{ 'N',  'V',  '1',  '2', 0x00, 0x00, 0x10, 0x00,
		 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71},
	.bBitsPerPixel		= 12,
	.bDefaultFrameIndex	= 1,
	.bAspectRatioX		= 0,
	.bAspectRatioY		= 0,
	.bmInterfaceFlags	= 0,
	.bCopyProtect		= 0,
};

DECLARE_UVC_FRAME_UNCOMPRESSED(1);
DECLARE_UVC_FRAME_UNCOMPRESSED(3);

static const struct UVC_FRAME_UNCOMPRESSED(3) uvc_frame_yuv_360p = {
	.bLength		= UVC_DT_FRAME_UNCOMPRESSED_SIZE(3),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_UNCOMPRESSED,
	.bFrameIndex		= 1,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(640),
	.wHeight		= cpu_to_le16(360),
	.dwMinBitRate		= cpu_to_le32(18432000),
	.dwMaxBitRate		= cpu_to_le32(55296000),
	.dwMaxVideoFrameBufferSize	= cpu_to_le32(460800),
	.dwDefaultFrameInterval	= cpu_to_le32(400000),
	.bFrameIntervalType	= 3,
	.dwFrameInterval[0]	= cpu_to_le32(400000),
	.dwFrameInterval[1]	= cpu_to_le32(666666),
	.dwFrameInterval[2]	= cpu_to_le32(1000000),
};

static const struct UVC_FRAME_UNCOMPRESSED(1) uvc_frame_yuv_720p = {
	.bLength		= UVC_DT_FRAME_UNCOMPRESSED_SIZE(1),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_UNCOMPRESSED,
	.bFrameIndex		= 2,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(800),
	.wHeight		= cpu_to_le16(600),
	.dwMinBitRate		= cpu_to_le32(29491200),
	.dwMaxBitRate		= cpu_to_le32(29491200),
	.dwMaxVideoFrameBufferSize	= cpu_to_le32(1843200),
	.dwDefaultFrameInterval	= cpu_to_le32(333333),
	.bFrameIntervalType	= 1,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
};

typedef struct uvc_image {
	__u16 wWidth;
	__u16 wHeight;
}uvc_image_t;

struct uvc_still_image_frame {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bDescriptorSubType;
	__u8  bEndpointAddress;
	__u8  bNumImageSizePatterns;
	uvc_image_t img[1];
	__u8  bNumCompressionPattern;
} __attribute__((__packed__));


#define UVC_DT_STILL_IMAGE_FRAME_SIZE(n)		(10+(4*n)-4)

static const struct uvc_still_image_frame uvc_still_image_frame_base = {
	.bLength 		= UVC_DT_STILL_IMAGE_FRAME_SIZE(1),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_STILL_IMAGE_FRAME,
	.bEndpointAddress	= 0x00,
	.bNumImageSizePatterns	= 1,
	.img[0].wWidth		= cpu_to_le16(1920),
	.img[0].wHeight		= cpu_to_le16(1080),
	.bNumCompressionPattern = 0,
};

static struct uvc_format_mjpeg uvc_format_mjpg = {
	.bLength		= UVC_DT_FORMAT_MJPEG_SIZE,
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FORMAT_MJPEG,
	.bFormatIndex		= 2,
	.bNumFrameDescriptors	= 4,
	.bmFlags		= 0,
	.bDefaultFrameIndex	= 1,
	.bAspectRatioX		= 0,
	.bAspectRatioY		= 0,
	.bmInterfaceFlags	= 0,
	.bCopyProtect		= 0,
};

DECLARE_UVC_FRAME_MJPEG(1);
DECLARE_UVC_FRAME_MJPEG(2);
DECLARE_UVC_FRAME_MJPEG(3);
DECLARE_UVC_FRAME_MJPEG(4);

static const struct UVC_FRAME_MJPEG(2) uvc_frame_mjpg_360p = {
	.bLength		= UVC_DT_FRAME_MJPEG_SIZE(2),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_MJPEG,
	.bFrameIndex		= 3,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(640),
	.wHeight		= cpu_to_le16(360),
	.dwMinBitRate		= cpu_to_le32(7372800),
	.dwMaxBitRate		= cpu_to_le32(7372800),
	.dwMaxVideoFrameBufferSize	= cpu_to_le32(460800),
	.dwDefaultFrameInterval	= cpu_to_le32(333333),
	.bFrameIntervalType	= 2,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
	.dwFrameInterval[1]	= cpu_to_le32(666666),
};

static const struct UVC_FRAME_MJPEG(4) uvc_frame_mjpg_360p_4 = {
	.bLength		= UVC_DT_FRAME_MJPEG_SIZE(4),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_MJPEG,
	.bFrameIndex		= 3,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(640),
	.wHeight		= cpu_to_le16(360),
	.dwMinBitRate		= cpu_to_le32(7372800),
	.dwMaxBitRate		= cpu_to_le32(7372800),
	.dwMaxVideoFrameBufferSize	= cpu_to_le32(460800),
	.dwDefaultFrameInterval	= cpu_to_le32(333333),
	.bFrameIntervalType	= 4,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
	.dwFrameInterval[1]	= cpu_to_le32(666666),
};

static const struct UVC_FRAME_MJPEG(2) uvc_frame_mjpg_720p = {
	.bLength		= UVC_DT_FRAME_MJPEG_SIZE(2),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_MJPEG,
	.bFrameIndex		= 4,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(1280),
	.wHeight		= cpu_to_le16(720),
	.dwMinBitRate		= cpu_to_le32(221184000),
	.dwMaxBitRate		= cpu_to_le32(442368000),
	.dwMaxVideoFrameBufferSize	= cpu_to_le32(1843200),
	.dwDefaultFrameInterval	= cpu_to_le32(333333),
	.bFrameIntervalType	= 1,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
	.dwFrameInterval[1]	= cpu_to_le32(666666),
};


static const struct UVC_FRAME_MJPEG(2) uvc_frame_mjpg_960p = {
	.bLength		= UVC_DT_FRAME_MJPEG_SIZE(2),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_MJPEG,
	.bFrameIndex		= 2,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(1280),
	.wHeight		= cpu_to_le16(960),
	.dwMinBitRate		= cpu_to_le32(196608000),
	.dwMaxBitRate		= cpu_to_le32(589824000),
	.dwMaxVideoFrameBufferSize	= cpu_to_le32(2457600),
	.dwDefaultFrameInterval	= cpu_to_le32(333333),
	.bFrameIntervalType	= 1,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
	.dwFrameInterval[1]	= cpu_to_le32(666666),
};


static const struct UVC_FRAME_MJPEG(1) uvc_frame_mjpg_1080p = {
	.bLength		= UVC_DT_FRAME_MJPEG_SIZE(1),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_MJPEG,
	.bFrameIndex		= 1,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(1920),
	.wHeight		= cpu_to_le16(1080),
	.dwMinBitRate		= cpu_to_le32(497664000),
	.dwMaxBitRate		= cpu_to_le32(995328000),
	.dwMaxVideoFrameBufferSize	= cpu_to_le32(4147200),
	.dwDefaultFrameInterval	= cpu_to_le32(333333),
	.bFrameIntervalType	= 1,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
};

#if 0
static struct UVC_FRAME_MJPEG(3) uvc_frame_mjpg_1080p_3 = {
	.bLength		= UVC_DT_FRAME_MJPEG_SIZE(3),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_MJPEG,
	.bFrameIndex		= 1,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(1920),
	.wHeight		= cpu_to_le16(1080),
	.dwMinBitRate		= cpu_to_le32(497664000),
	.dwMaxBitRate		= cpu_to_le32(995328000),
	.dwMaxVideoFrameBufferSize	= cpu_to_le32(4147200),
	.dwDefaultFrameInterval	= cpu_to_le32(333333),
	.bFrameIntervalType	= 3,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
	.dwFrameInterval[1]	= cpu_to_le32(666666),
	.dwFrameInterval[2]	= cpu_to_le32(999999),
};

static struct UVC_FRAME_MJPEG(2) uvc_frame_mjpg_2mp = {
	.bLength		= UVC_DT_FRAME_MJPEG_SIZE(2),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_MJPEG,
	.bFrameIndex		= 5,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(2560),
	.wHeight		= cpu_to_le16(1440),
	.dwMinBitRate		= cpu_to_le32(884736000),
	.dwMaxBitRate		= cpu_to_le32(1769472000),
	.dwMaxVideoFrameBufferSize	= cpu_to_le32(4147200),
	.dwDefaultFrameInterval	= cpu_to_le32(333333),
	.bFrameIntervalType	= 2,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
	.dwFrameInterval[1]	= cpu_to_le32(400000),
};
static const struct UVC_FRAME_MJPEG(1) uvc_frame_mjpg_5mp = {
	.bLength		= UVC_DT_FRAME_MJPEG_SIZE(1),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_MJPEG,
	.bFrameIndex		= 6,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(2592),
	.wHeight		= cpu_to_le16(1944),
	.dwMinBitRate		= cpu_to_le32(806215680),
	.dwMaxBitRate		= cpu_to_le32(2015539200),
	.dwMaxVideoFrameBufferSize	= cpu_to_le32(4147200),
	.dwDefaultFrameInterval	= cpu_to_le32(333333),
	.bFrameIntervalType	= 1,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
};
#endif

static struct uvc_format_framebased uvc_format_h264 = {
	.bLength		= UVC_DT_FORMAT_FRAMEBASED_SIZE,
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FORMAT_FRAME_BASED,
	.bFormatIndex		= 3,
	.bNumFrameDescriptors	= 4,
	.guidFormat		=
		{ 'H',  '2',  '6',  '4', 0x00, 0x00, 0x10, 0x00,
		 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71},
	.bBitsPerPixel		= 16,
	.bDefaultFrameIndex	= 1,
	.bAspectRatioX		= 0,
	.bAspectRatioY		= 0,
	.bmInterfaceFlags	= 0,
	.bCopyProtect		= 0,
	.bVariableSize		= 1,
};

DECLARE_UVC_FRAME_FRAMEBASED(1);
DECLARE_UVC_FRAME_FRAMEBASED(2);
DECLARE_UVC_FRAME_FRAMEBASED(3);
DECLARE_UVC_FRAME_FRAMEBASED(4);

static const struct UVC_FRAME_FRAMEBASED(1) uvc_frame_h264_360p = {
	.bLength		= UVC_DT_FRAME_FRAMEBASED_SIZE(1),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_FRAME_BASED,
	.bFrameIndex		= 3,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(640),
	.wHeight		= cpu_to_le16(360),
	.dwMinBitRate		= cpu_to_le32(7372800),
	.dwMaxBitRate		= cpu_to_le32(7372800),
	.dwDefaultFrameInterval	= cpu_to_le32(10000000/25),
	.bFrameIntervalType	= 1,
	.dwFrameInterval[0]	= cpu_to_le32(10000000/25),
	/*.dwFrameInterval[1]	= cpu_to_le32(666666),*/
};

static const struct UVC_FRAME_FRAMEBASED(4) uvc_frame_h264_360p_4 = {
	.bLength		= UVC_DT_FRAME_FRAMEBASED_SIZE(4),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_FRAME_BASED,
	.bFrameIndex		= 3,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(640),
	.wHeight		= cpu_to_le16(360),
	.dwMinBitRate		= cpu_to_le32(7372800),
	.dwMaxBitRate		= cpu_to_le32(7372800),
	.dwDefaultFrameInterval	= cpu_to_le32(333333),
	.bFrameIntervalType	= 4,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
	.dwFrameInterval[1]	= cpu_to_le32(666666),
};

static const struct UVC_FRAME_FRAMEBASED(1) uvc_frame_h264_720p = {
	.bLength		= UVC_DT_FRAME_FRAMEBASED_SIZE(1),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_FRAME_BASED,
	.bFrameIndex		= 4,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(1280),
	.wHeight		= cpu_to_le16(720),
	.dwMinBitRate		= cpu_to_le32(221184000),
	.dwMaxBitRate		= cpu_to_le32(368640000),
	.dwDefaultFrameInterval	= cpu_to_le32(400000),
	.bFrameIntervalType	= 1,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
};

static const struct UVC_FRAME_FRAMEBASED(1) uvc_frame_h264_960p = {
	.bLength		= UVC_DT_FRAME_FRAMEBASED_SIZE(1),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_FRAME_BASED,
	.bFrameIndex		= 2,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(1280),
	.wHeight		= cpu_to_le16(960),
	.dwMinBitRate		= cpu_to_le32(196608000),
	.dwMaxBitRate		= cpu_to_le32(589824000),
	.dwDefaultFrameInterval	= cpu_to_le32(333333),
	.bFrameIntervalType	= 1,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
};
static const struct UVC_FRAME_FRAMEBASED(1) uvc_frame_h264_1080p = {
	.bLength		= UVC_DT_FRAME_FRAMEBASED_SIZE(1),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_FRAME_BASED,
	.bFrameIndex		= 1,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(1920),
	.wHeight		= cpu_to_le16(1080),
	.dwMinBitRate		= cpu_to_le32(497664000),
	.dwMaxBitRate		= cpu_to_le32(829440000),
	.dwDefaultFrameInterval	= cpu_to_le32(333333),
	.bFrameIntervalType	= 1,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
};

#if 0
static struct UVC_FRAME_FRAMEBASED(2) uvc_frame_h264_2mp = {
	.bLength		= UVC_DT_FRAME_FRAMEBASED_SIZE(2),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_FRAME_BASED,
	.bFrameIndex		= 5,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(2560),
	.wHeight		= cpu_to_le16(1440),
	.dwMinBitRate		= cpu_to_le32(884736000),
	.dwMaxBitRate		= cpu_to_le32(1769472000),
	.dwDefaultFrameInterval	= cpu_to_le32(333333),
	.bFrameIntervalType	= 2,
	.dwFrameInterval[0]	= cpu_to_le32(333333),
	.dwFrameInterval[1]	= cpu_to_le32(400000),
};
static const struct UVC_FRAME_FRAMEBASED(1) uvc_frame_h264_5mp = {
	.bLength		= UVC_DT_FRAME_FRAMEBASED_SIZE(1),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_FRAME_FRAME_BASED,
	.bFrameIndex		= 6,
	.bmCapabilities		= 0,
	.wWidth			= cpu_to_le16(2592),
	.wHeight		= cpu_to_le16(1944),
	.dwMinBitRate		= cpu_to_le32(806215680),
	.dwMaxBitRate		= cpu_to_le32(1209323520),
	.dwDefaultFrameInterval	= cpu_to_le32(666666),
	.bFrameIntervalType	= 1,
	.dwFrameInterval[0]	= cpu_to_le32(666666),
};
#endif

static struct uvc_color_matching_descriptor uvc_color_matching = {
	.bLength		= UVC_DT_COLOR_MATCHING_SIZE,
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubType	= UVC_VS_COLORFORMAT,
	.bColorPrimaries	= 1,
	.bTransferCharacteristics	= 1,
	.bMatrixCoefficients	= 4,
};

static struct uvc_descriptor_header * uvc_fs_control_cls[8] = {
	(const struct uvc_descriptor_header *) &uvc_control_header,
	(const struct uvc_descriptor_header *) &uvc_camera_terminal,
	(const struct uvc_descriptor_header *) &uvc_processing,
	(const struct uvc_descriptor_header *) &uvc_output_terminal,
	NULL,
};

static const struct uvc_descriptor_header * const uvc_ss_control_cls[] = {
	(const struct uvc_descriptor_header *) &uvc_control_header,
	(const struct uvc_descriptor_header *) &uvc_camera_terminal,
	(const struct uvc_descriptor_header *) &uvc_processing,
	(const struct uvc_descriptor_header *) &uvc_output_terminal,
	NULL,
};

static const struct uvc_descriptor_header * const uvc_fs_streaming_cls[] = {
	(const struct uvc_descriptor_header *) &uvc_input_header,
	(const struct uvc_descriptor_header *) &uvc_format_yuv,
	(const struct uvc_descriptor_header *) &uvc_frame_yuv_360p,
	(const struct uvc_descriptor_header *) &uvc_frame_yuv_720p,
	(const struct uvc_descriptor_header *) &uvc_format_mjpg,
//	(const struct uvc_descriptor_header *) &uvc_frame_mjpg_360p,
	(const struct uvc_descriptor_header *) &uvc_frame_mjpg_720p,
	(const struct uvc_descriptor_header *) &uvc_frame_mjpg_960p,
	(const struct uvc_descriptor_header *) &uvc_frame_mjpg_1080p,
	(const struct uvc_descriptor_header *) &uvc_color_matching,
	NULL,
};

static const struct uvc_descriptor_header * const uvc_hs_streaming_cls[] = {
	(const struct uvc_descriptor_header *) &uvc_input_header,
	(const struct uvc_descriptor_header *) &uvc_format_yuv,
	(const struct uvc_descriptor_header *) &uvc_frame_yuv_360p,
	(const struct uvc_descriptor_header *) &uvc_frame_yuv_720p,
	(const struct uvc_descriptor_header *) &uvc_still_image_frame_base,
	(const struct uvc_descriptor_header *) &uvc_format_mjpg,
	(const struct uvc_descriptor_header *) &uvc_frame_mjpg_1080p,
	(const struct uvc_descriptor_header *) &uvc_frame_mjpg_960p,
	(const struct uvc_descriptor_header *) &uvc_frame_mjpg_360p,
	(const struct uvc_descriptor_header *) &uvc_frame_mjpg_720p,
	 /*(const struct uvc_descriptor_header *) &uvc_frame_mjpg_4mp, */
	/* (const struct uvc_descriptor_header *) &uvc_frame_mjpg_5mp, */
	(const struct uvc_descriptor_header *) &uvc_still_image_frame_base,
	(const struct uvc_descriptor_header *) &uvc_format_h264,
	(const struct uvc_descriptor_header *) &uvc_frame_h264_1080p,
	(const struct uvc_descriptor_header *) &uvc_frame_h264_960p,
	(const struct uvc_descriptor_header *) &uvc_frame_h264_360p,
	(const struct uvc_descriptor_header *) &uvc_frame_h264_720p,
	/* (const struct uvc_descriptor_header *) &uvc_frame_h264_4mp, */
	/* (const struct uvc_descriptor_header *) &uvc_frame_h264_5mp, */
	(const struct uvc_descriptor_header *) &uvc_color_matching,
	NULL,
};

static const struct uvc_descriptor_header * const uvc_ss_streaming_cls[] = {
	(const struct uvc_descriptor_header *) &uvc_input_header,
	(const struct uvc_descriptor_header *) &uvc_format_yuv,
	(const struct uvc_descriptor_header *) &uvc_frame_yuv_360p,
	(const struct uvc_descriptor_header *) &uvc_frame_yuv_720p,
	(const struct uvc_descriptor_header *) &uvc_format_mjpg,
//	(const struct uvc_descriptor_header *) &uvc_frame_mjpg_360p,
	(const struct uvc_descriptor_header *) &uvc_frame_mjpg_720p,
	(const struct uvc_descriptor_header *) &uvc_frame_mjpg_960p,
	(const struct uvc_descriptor_header *) &uvc_frame_mjpg_1080p,
	(const struct uvc_descriptor_header *) &uvc_color_matching,
	NULL,
};


/* --------------------------------------------------------------------------
 * USB configuration
 */

int ucam_bind_uvc(struct usb_configuration *c)
{
	if (ucamera_streaming_cls == NULL) {
		printk("webcam bind uvc config failed\n");
		return -1;
	}

	if (af_en){
		uvc_camera_terminal.bmControls[0] = 0x2a;
		uvc_camera_terminal.bmControls[2] = 0x02;
	}

	if (uvc_extension_en) {
		uvc_output_terminal.bSourceID = 4;
		uvc_fs_control_cls[3] = (struct uvc_descriptor_header *)&uvc_extersion_unit;
		uvc_fs_control_cls[4] = (struct uvc_descriptor_header *)&uvc_output_terminal;
		uvc_fs_control_cls[5] = NULL;
	}

	return uvc_bind_config(c, uvc_fs_control_cls, uvc_ss_control_cls,
		uvc_fs_streaming_cls, (const struct uvc_descriptor_header * const *)ucamera_streaming_cls,
		uvc_ss_streaming_cls);
}
EXPORT_SYMBOL(ucam_bind_uvc);

/* --------------------------------------------------------------------------
 * Driver
 */



struct uvc_frame_info {
	unsigned int width;
	unsigned int height;
	unsigned int intervals[4];
};

struct uvc_format_info {
	unsigned int fcc;
	unsigned int nframe;
	struct uvc_frame_info *frames;
};

/*
 * UVC_INPUT_HEADER_DESCRIPTOR
 */
static int set_uvc_input_header(int nformat, int index)
{
	int cnt = index;
	switch (nformat) {
	case 3:
		ucamera_streaming_cls[cnt] = (struct uvc_descriptor_header *)&uvc_input_header;
		break;
	case 2:
		ucamera_streaming_cls[cnt] = (struct uvc_descriptor_header *)&uvc_input_header2;
		break;
	case 1:
		ucamera_streaming_cls[cnt] = (struct uvc_descriptor_header *)&uvc_input_header2;
		break;
	default:
		ucamera_streaming_cls[cnt] =(struct uvc_descriptor_header *)&uvc_input_header2;
		break;
	}

	return cnt;
}
/*
 * frame: frame_info
 * iframe: frame index
 * index: ucamera_streaming_cls index
 *
 */
static int set_uvc_yuyv_frames(struct uvc_frame_info *frame, int iframe, int index)
{
	int cnt = index;
	struct UVC_FRAME_UNCOMPRESSED(1) *frame_yuv = NULL;

	frame_yuv = (struct UVC_FRAME_UNCOMPRESSED(1) *)ucamera_streaming_cls[cnt];
	frame_yuv->wWidth = cpu_to_le16(frame[iframe].width);
	frame_yuv->wHeight = cpu_to_le16(frame[iframe].height);
	frame_yuv->bFrameIndex = iframe + 1;
	if(frame_yuv->wHeight >= 960) {
		frame_yuv->dwFrameInterval[0] = 10000000 / 5;
		frame_yuv->dwDefaultFrameInterval = 10000000 /5;
	} else if (frame_yuv->wHeight == 720) {
		frame_yuv->dwFrameInterval[0] = 10000000 / 10;
		frame_yuv->dwDefaultFrameInterval = 10000000 / 10;
	} else {
		frame_yuv->dwFrameInterval[0] = 10000000 / 30;
		frame_yuv->dwDefaultFrameInterval = 10000000 / 30;
	}

	return index;
}

/*
 * frame: frame_info
 * iframe: frame index
 * index: ucamera_streaming_cls index
 *
 */
static int set_uvc_nv12_frames(struct uvc_frame_info *frame, int iframe, int index)
{
	int cnt = index;
	struct UVC_FRAME_UNCOMPRESSED(1) *frame_yuv = NULL;

	frame_yuv = (struct UVC_FRAME_UNCOMPRESSED(1) *)ucamera_streaming_cls[cnt];
	frame_yuv->wWidth = cpu_to_le16(frame[iframe].width);
	frame_yuv->wHeight = cpu_to_le16(frame[iframe].height);
	frame_yuv->bFrameIndex = iframe + 1;
	if(frame_yuv->wHeight > 960) {
		frame_yuv->dwFrameInterval[0] = 10000000 / 5;
		frame_yuv->dwDefaultFrameInterval = 10000000 /5;
	} else if (frame_yuv->wHeight == 960) {
		frame_yuv->dwFrameInterval[0] = 10000000 / 10;
		frame_yuv->dwDefaultFrameInterval = 10000000 / 10;
	} else if (frame_yuv->wHeight == 720) {
		frame_yuv->dwFrameInterval[0] = 10000000 / 15;
		frame_yuv->dwDefaultFrameInterval = 10000000 / 15;
	} else {
		frame_yuv->dwFrameInterval[0] = 10000000 / 30;
		frame_yuv->dwDefaultFrameInterval = 10000000 / 30;
	}

	return index;
}

/*
 * frame: frame_info
 * iframe: frame index
 * index: ucamera_streaming_cls index
 *
 */
static int set_uvc_h264_frames(struct uvc_frame_info *frame, int iframe, int support_nframes, int index)
{
	int cnt = index;

	switch(support_nframes) {
	case 2:
		{
			struct UVC_FRAME_FRAMEBASED(2) *frame_h264 = NULL;

			frame_h264 = (struct UVC_FRAME_FRAMEBASED(2) *)ucamera_streaming_cls[cnt];
			frame_h264->wWidth = cpu_to_le16(frame[iframe].width);
			frame_h264->wHeight = cpu_to_le16(frame[iframe].height);
			frame_h264->bFrameIndex = iframe + 1;
			frame_h264->dwDefaultFrameInterval = 10000000 /30;

			frame_h264->dwFrameInterval[0] = 10000000 / 30;
			frame_h264->dwFrameInterval[1] = 10000000 / 25;

		}
		break;
	case 4:
		{
			struct UVC_FRAME_FRAMEBASED(4) *frame_h264 = NULL;

			frame_h264 = (struct UVC_FRAME_FRAMEBASED(4) *)ucamera_streaming_cls[cnt];
			frame_h264->wWidth = cpu_to_le16(frame[iframe].width);
			frame_h264->wHeight = cpu_to_le16(frame[iframe].height);
			frame_h264->bFrameIndex = iframe + 1;
			frame_h264->dwDefaultFrameInterval = 10000000 /30;

			frame_h264->dwFrameInterval[0] = 10000000 / 60;
			frame_h264->dwFrameInterval[1] = 10000000 / 50;
			frame_h264->dwFrameInterval[2] = 10000000 / 30;
			frame_h264->dwFrameInterval[3] = 10000000 / 25;
		}
		break;
	default:
		{
			struct UVC_FRAME_FRAMEBASED(1) *frame_h264 = NULL;

			frame_h264 = (struct UVC_FRAME_FRAMEBASED(1) *)ucamera_streaming_cls[cnt];
			frame_h264->wWidth = cpu_to_le16(frame[iframe].width);
			frame_h264->wHeight = cpu_to_le16(frame[iframe].height);
			frame_h264->bFrameIndex = iframe + 1;
			frame_h264->dwDefaultFrameInterval = 10000000 /25;

			frame_h264->dwFrameInterval[0] = 10000000 / 25;

		}
		break;
	}

	return index;
}

/*
 * frame: frame_info
 * iframe: frame index
 * index: ucamera_streaming_cls index
 *
 */
static int set_uvc_mipeg_frames(struct uvc_frame_info *frame, int iframe, int support_nframes, int index)
{
	int cnt = index;

	switch(support_nframes) {
	case 2:
		{
			struct UVC_FRAME_MJPEG(2) *frame_mjpeg = NULL;

			frame_mjpeg = (struct UVC_FRAME_MJPEG(2) *)ucamera_streaming_cls[cnt];
			frame_mjpeg->wWidth = cpu_to_le16(frame[iframe].width);
			frame_mjpeg->wHeight = cpu_to_le16(frame[iframe].height);
			frame_mjpeg->bFrameIndex = iframe + 1;
			frame_mjpeg->dwDefaultFrameInterval = 10000000 /30;

			frame_mjpeg->dwFrameInterval[0] = 10000000 / 30;
			frame_mjpeg->dwFrameInterval[1] = 10000000 / 25;

		}
		break;
	case 4:
		{
			struct UVC_FRAME_MJPEG(4) *frame_mjpeg = NULL;

			frame_mjpeg = (struct UVC_FRAME_MJPEG(4) *)ucamera_streaming_cls[cnt];
			frame_mjpeg->wWidth = cpu_to_le16(frame[iframe].width);
			frame_mjpeg->wHeight = cpu_to_le16(frame[iframe].height);
			frame_mjpeg->bFrameIndex = iframe + 1;
			frame_mjpeg->dwDefaultFrameInterval = 10000000 /30;

			frame_mjpeg->dwFrameInterval[0] = 10000000 / 60;
			frame_mjpeg->dwFrameInterval[1] = 10000000 / 50;
			frame_mjpeg->dwFrameInterval[2] = 10000000 / 30;
			frame_mjpeg->dwFrameInterval[3] = 10000000 / 25;
		}
		break;
	default:
		{
			struct UVC_FRAME_MJPEG(2) *frame_mjpeg = NULL;

			frame_mjpeg = (struct UVC_FRAME_MJPEG(2) *)ucamera_streaming_cls[cnt];
			frame_mjpeg->wWidth = cpu_to_le16(frame[iframe].width);
			frame_mjpeg->wHeight = cpu_to_le16(frame[iframe].height);
			frame_mjpeg->bFrameIndex = iframe + 1;
			frame_mjpeg->dwDefaultFrameInterval = 10000000 /30;

			frame_mjpeg->dwFrameInterval[0] = 10000000 / 30;
			frame_mjpeg->dwFrameInterval[1] = 10000000 / 25;

		}
		break;
	}

	return index;
}
#if 0
static int set_uvc_mipeg_frames(struct uvc_frame_info *frame, int iframe, int index)
{
	int cnt = index;
	struct UVC_FRAME_MJPEG(2) *frame_mjpeg = NULL;
	struct UVC_FRAME_MJPEG(4) *frame_mjpeg_4 = NULL;

	frame_mjpeg = (struct UVC_FRAME_MJPEG(2) *)ucamera_streaming_cls[cnt];
#ifndef HIK_2K_CAMERA
	frame_mjpeg->wWidth = cpu_to_le16(frame[iframe].width);
	frame_mjpeg->wHeight = cpu_to_le16(frame[iframe].height);
	frame_mjpeg->bFrameIndex = iframe + 1;
	frame_mjpeg->dwFrameInterval[0] = 10000000 / 30;
	frame_mjpeg->dwFrameInterval[1] = 10000000 / 25;
	frame_mjpeg->dwDefaultFrameInterval = 10000000 /30;
#else
	int w = cpu_to_le16(frame[iframe].width);
	if (w <= 1920) {
		frame_mjpeg_4 = (struct UVC_FRAME_MJPEG(4) *)ucamera_streaming_cls[cnt];
		frame_mjpeg_4->wWidth = cpu_to_le16(frame[iframe].width);
		frame_mjpeg_4->wHeight = cpu_to_le16(frame[iframe].height);
		frame_mjpeg_4->bFrameIndex = iframe + 1;
		frame_mjpeg_4->dwFrameInterval[0] = 10000000 / 60;
		frame_mjpeg_4->dwFrameInterval[1] = 10000000 / 50;
		frame_mjpeg_4->dwFrameInterval[2] = 10000000 / 30;
		frame_mjpeg_4->dwFrameInterval[3] = 10000000 / 25;
		frame_mjpeg_4->dwDefaultFrameInterval = 10000000 /30;
	} else {
		/* 2k */
		frame_mjpeg->wWidth = cpu_to_le16(frame[iframe].width);
		frame_mjpeg->wHeight = cpu_to_le16(frame[iframe].height);
		frame_mjpeg->bFrameIndex = iframe + 1;
		frame_mjpeg->dwFrameInterval[0] = 10000000 / 30;
		frame_mjpeg->dwFrameInterval[1] = 10000000 / 25;
		frame_mjpeg->dwDefaultFrameInterval = 10000000 /30;
	}
#endif

}
#endif

static int set_uvc_fromats_info(struct uvc_format_info ***uvc_formats_info, int nformat, int index)
{
	int iformat = 0,iframe = 0, nframe = 0;
	int len = 0;
	int ret = -1;
	int cnt = index;
	struct uvc_descriptor_header * frame_header = NULL;
	struct uvc_descriptor_header * frame_header4 = NULL;
	struct uvc_frame_info *frame;

	struct uvc_format_info **uvc_formats = *uvc_formats_info;
	while (iformat < nformat) {
		cnt++;
		nframe = uvc_formats[iformat]->nframe;
		switch (uvc_formats[iformat]->fcc) {
		case V4L2_PIX_FMT_YUYV:
			uvc_format_yuv.bFormatIndex = iformat + 1;
			uvc_format_yuv.bNumFrameDescriptors = nframe;
			ucamera_streaming_cls[cnt] = (struct uvc_descriptor_header *)&uvc_format_yuv;
			frame_header = (struct uvc_descriptor_header *)&uvc_frame_yuv_720p;

			break;
		case V4L2_PIX_FMT_NV12:
			uvc_format_nv12.bFormatIndex = iformat + 1;
			uvc_format_nv12.bNumFrameDescriptors = nframe;
			ucamera_streaming_cls[cnt] = (struct uvc_descriptor_header *)&uvc_format_nv12;
			frame_header = (struct uvc_descriptor_header *)&uvc_frame_yuv_720p;

			break;
		case V4L2_PIX_FMT_MJPEG:
			uvc_format_mjpg.bFormatIndex = iformat + 1;
			uvc_format_mjpg.bNumFrameDescriptors = nframe;
			ucamera_streaming_cls[cnt] = (struct uvc_descriptor_header *)&uvc_format_mjpg;
			frame_header = (struct uvc_descriptor_header *)&uvc_frame_mjpg_360p; /* 2 frames*/
			frame_header4 = (struct uvc_descriptor_header *)&uvc_frame_mjpg_360p_4; /* 4 frames */
			break;
		case V4L2_PIX_FMT_H264:
			uvc_format_h264.bFormatIndex = iformat + 1;
			uvc_format_h264.bNumFrameDescriptors = nframe;
			ucamera_streaming_cls[cnt] = (struct uvc_descriptor_header *)&uvc_format_h264;
			frame_header = (struct uvc_descriptor_header *)&uvc_frame_h264_360p;   /* 25fps */
			frame_header4 = (struct uvc_descriptor_header *)&uvc_frame_h264_360p_4;
			break;
		}

		len = uvc_formats[iformat]->nframe * sizeof(struct uvc_frame_info);
		frame = kmalloc(len, GFP_KERNEL);
		ret = copy_from_user(frame, (void __user*)uvc_formats[iformat]->frames, len);
		if (ret){
			printk("error(%s, %d): failed to copy_from_user!\n", __func__, __LINE__);
			ret = -EIO;
			return ret;
		}
		iframe = 0;
		while (iframe < uvc_formats[iformat]->nframe) {
			cnt++;
			int support_nframes = 2;
#ifdef HIK_2K_CAMERA
			int w = cpu_to_le16(frame[iframe].width);
			if (w <= 1920) {
				support_nframes = 4;
			}

			/* skip yuv */
			if (uvc_formats[iformat]->fcc == V4L2_PIX_FMT_YUYV || uvc_formats[iformat]->fcc == V4L2_PIX_FMT_NV12) {
				support_nframes = 1;
			}
#endif

			if (support_nframes == 4) {
				ucamera_streaming_cls[cnt] = kmalloc(frame_header4->bLength, GFP_KERNEL);
				memcpy(ucamera_streaming_cls[cnt], frame_header4, frame_header4->bLength);
			} else if (support_nframes == 2) {
				ucamera_streaming_cls[cnt] = kmalloc(frame_header->bLength, GFP_KERNEL);
				memcpy(ucamera_streaming_cls[cnt], frame_header, frame_header->bLength);
			} else {
				ucamera_streaming_cls[cnt] = kmalloc(frame_header->bLength, GFP_KERNEL);
				memcpy(ucamera_streaming_cls[cnt], frame_header, frame_header->bLength);
			}
			switch (uvc_formats[iformat]->fcc) {
			case V4L2_PIX_FMT_YUYV:
				set_uvc_yuyv_frames(frame, iframe , cnt);
				break;
			case V4L2_PIX_FMT_NV12:
				set_uvc_nv12_frames(frame, iframe, cnt);
				break;
			case V4L2_PIX_FMT_MJPEG:
				set_uvc_mipeg_frames(frame, iframe ,support_nframes, cnt);
				break;
			case V4L2_PIX_FMT_H264:
				support_nframes = 1;
				set_uvc_h264_frames(frame, iframe ,support_nframes, cnt);
				break;
			}

			/* printk("webcam:%d %d %d\n", frame_tmp->wWidth, frame_tmp->wHeight, frame_tmp->bFrameIndex); */
			/* frame[iframes].height; */
			iframe++;
		}
		if (stillimg_en) {
			if (uvc_formats[iformat]->fcc == V4L2_PIX_FMT_YUYV ||
			    uvc_formats[iformat]->fcc == V4L2_PIX_FMT_MJPEG) {
				cnt++;
				ucamera_streaming_cls[cnt] = (struct uvc_descriptor_header *)&uvc_still_image_frame_base;
			}

		}
		kfree(frame);
		iformat++;

		cnt++;
		ucamera_streaming_cls[cnt] = (struct uvc_descriptor_header *)&uvc_color_matching;
	}

	return cnt;
}

int uvc_set_descriptor(unsigned long arg)
{
	int len, ret;
	unsigned int nframe, nformat, cnt = 0;
	int iformat = 0;
	struct uvc_format_info **uvc_formats;
	struct uvc_format_info *format;

	uvc_formats = kmalloc(3*sizeof(struct uvc_format_info *), GFP_KERNEL);
	ret = copy_from_user(uvc_formats, (void __user*)arg, 3*sizeof(struct uvc_format_info *));
	if (ret) {
		printk("error(%s, %d): failed to copy_from_user!\n", __func__, __LINE__);
		ret = -EIO;
		return ret;
	}

	nframe = 0;
	nformat = 0;
	/*Step1: get format num */
	while(iformat < 3) {

		len = sizeof(struct uvc_format_info);
		format = kmalloc(len,  GFP_KERNEL);
		ret = copy_from_user(format, (void __user*)uvc_formats[iformat], len);
		if (ret){
			printk("error(%s, %d): failed to copy_from_user!\n", __func__, __LINE__);
			ret = -EIO;
			return ret;
		}
		/* printk("webcam fcc:%d nframes:%d\n", format->fcc, format->nframe); */
		uvc_formats[iformat] = format;
		if (format->nframe > 0) {
			nframe +=format->nframe;
			nformat++;
		}
		iformat++;
	}

	/*
	 * VS Input Header: 1
	 * Format Type: nformat
	 * Frame Type: nframe
	 * still Image: 1
	 * color match: 1
	 *
	 */
	cnt = 1 + nformat + nframe + 1 + 1;
	if (stillimg_en)
		cnt += 2;
	ucamera_streaming_cls = kmalloc(cnt*sizeof(struct uvc_descriptor_header *), GFP_KERNEL);

	/* Step1: set uvc_input_header */
	cnt = 0;
	cnt = set_uvc_input_header(nformat, cnt);

	/*Step2: set format header */
	cnt = set_uvc_fromats_info(&uvc_formats, nformat, cnt);

	cnt++;
	ucamera_streaming_cls[cnt] = NULL;
	nframe = 0;
	iformat = 0;
	while(iformat < 3) {
		kfree(uvc_formats[iformat]);
		iformat++;
	}
	kfree(uvc_formats);

	return 0;
}
EXPORT_SYMBOL(uvc_set_descriptor);

int uvc_sti_init(void)
{
	uvc_input_header.bStillCaptureMethod = 2;
	uvc_input_header.bTriggerSupport = 1;
	uvc_input_header.bTriggerUsage = 1;

	uvc_input_header2.bStillCaptureMethod = 2;
	uvc_input_header2.bTriggerSupport = 1;
	uvc_input_header2.bTriggerUsage = 1;

	stillimg_en = 1;
	return 0;
}
EXPORT_SYMBOL(uvc_sti_init);