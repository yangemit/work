/*
 * UVC gadget test application
 *
 * Copyright (C) 2010 Ideas on board SPRL <laurent.pinchart@ideasonboard.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 */

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/prctl.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "ucamera.h"

/* Enable debug prints. */
#undef ENABLE_BUFFER_DEBUG
#undef ENABLE_USB_REQUEST_DEBUG

/* #define ENABLE_USB_REQUEST_DEBUG */

#define CLEAR(x)	memset (&(x), 0, sizeof (x))
#define max(a, b)	(((a) > (b)) ? (a) : (b))

#define clamp(val, min, max) ({                 \
        typeof(val) __val = (val);              \
        typeof(min) __min = (min);              \
        typeof(max) __max = (max);              \
        (void) (&__val == &__min);              \
        (void) (&__val == &__max);              \
        __val = __val < __min ? __min: __val;   \
        __val > __max ? __max: __val; })

#define ARRAY_SIZE(a)	((sizeof(a) / sizeof(a[0])))
#define pixfmtstr(x) 	(x) & 0xff, ((x) >> 8) & 0xff, ((x) >> 16) & 0xff, \
			((x) >> 24) & 0xff

/*
 * The UVC webcam gadget kernel driver (g_webcam.ko) supports changing
 * the Brightness attribute of the Processing Unit (PU). by default. If
 * the underlying video capture device supports changing the Brightness
 * attribute of the image being acquired (like the Virtual Video, VIVI
 * driver), then we should route this UVC request to the respective
 * video capture device.
 *
 * Incase, there is no actual video capture device associated with the
 * UVC gadget and we wish to use this application as the final
 * destination of the UVC specific requests then we should return
 * pre-cooked (static) responses to GET_CUR(BRIGHTNESS) and
 * SET_CUR(BRIGHTNESS) commands to keep command verifier test tools like
 * UVC class specific test suite of USBCV, happy.
 *
 * Note that the values taken below are in sync with the VIVI driver and
 * must be changed for your specific video capture device. These values
 * also work well in case there in no actual video capture device.
 */
#define PU_BRIGHTNESS_MIN_VAL		0
#define PU_BRIGHTNESS_MAX_VAL		255
#define PU_BRIGHTNESS_STEP_SIZE		1
#define PU_BRIGHTNESS_DEFAULT_VAL	127

#define UVC_DEFAULT_WIDTH		1280
#define UVC_DEFAULT_HEIGHT		720
/* ---------------------------------------------------------------------------
 * Generic stuff
 */

struct uvc_device *g_udev = NULL;
struct Ucamera_Cfg *g_ucfg = NULL;

static struct uvc_frame_info uvc_frames_yuyv[] = {
	{ 640, 360, { 333333, 666666, 1000000, 0 }, },
	{ 800, 600, { 333333, 0 }, },
	{ 0, 0, { 0, }, },
};
#if 0

static const struct uvc_frame_info uvc_frames_mjpeg[] = {
	{ 640, 360, { 333333, 0 }, },
	{ 1280, 720, { 333333, 0}, },
	{ 1920, 1080, { 333333, 0}, },
	{ 0, 0, { 0, }, },
};

static const struct uvc_frame_info uvc_frames_h264[] = {
	{ 640, 360, { 333333, 0 }, },
	{ 1280, 720, { 333333, 0}, },
	{ 1920, 1080, { 333333, 0}, },
	{ 0, 0, { 0, }, },
};
#endif

struct uvc_format_info uvc_formats_yuyv = {
	.fcc = V4L2_PIX_FMT_YUYV,
	.nframes = 2,
	.frames = NULL,
};

struct uvc_format_info uvc_formats_mjpeg = {
	.fcc = V4L2_PIX_FMT_MJPEG,
	.nframes = 0,
	.frames = NULL,
};

struct uvc_format_info uvc_formats_h264 = {
	.fcc = V4L2_PIX_FMT_H264,
	.nframes = 0,
	.frames = NULL,
};

static struct uvc_format_info **uvc_formats = NULL;

static int g_nFormats = 2;

/* ---------------------------------------------------------------------------
 * V4L2 and UVC device instances
 */


/* forward declarations */
static int uvc_video_stream(struct uvc_device *dev, int enable);

/* ---------------------------------------------------------------------------
 * UVC generic stuff
 */

static const char *uvc_query_name(__u8 query)
{
	switch (query) {
	case UVC_SET_CUR:
		return "SET_CUR";
	case UVC_GET_CUR:
		return "GET_CUR";
	case UVC_GET_MIN:
		return "GET_MIN";
	case UVC_GET_MAX:
		return "GET_MAX";
	case UVC_GET_RES:
		return "GET_RES";
	case UVC_GET_LEN:
		return "GET_LEN";
	case UVC_GET_INFO:
		return "GET_INFO";
	case UVC_GET_DEF:
		return "GET_DEF";
	default:
		return "<invalid>";
	}
}

/*
 * FIXME -- Need to see what this was intended for on the UVC gadget side.  It's
 * 			here, but there's nothing TIED to it right now on the gadget server
 * 			end of things.  Commenting it out to remove warnings.
 */

static int
uvc_video_still_image_cap_triggle(struct uvc_device *dev)
{
	int ret;

	ret = ioctl(dev->uvc_fd, UVCIOC_STILL_IMAGE_CAP, NULL);
	if (ret < 0) {
		Ucamera_LOG("UVC: Unable to triggle still image capture %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}

	Ucamera_LOG("UVC: Triggle Still Image Capture. \n");

	return 0;
}

static int
uvc_video_set_format(struct uvc_device *dev)
{
	struct v4l2_format fmt;
	int ret;

	CLEAR(fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	fmt.fmt.pix.width = dev->width;
	fmt.fmt.pix.height = dev->height;
	fmt.fmt.pix.pixelformat = dev->fcc;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;
	if (dev->fcc == V4L2_PIX_FMT_MJPEG || dev->fcc == V4L2_PIX_FMT_H264)
		fmt.fmt.pix.sizeimage = dev->imgsize * 1.5 ;
	else
		fmt.fmt.pix.sizeimage = dev->imgsize * 2 ;

	ret = ioctl(dev->uvc_fd, VIDIOC_S_FMT, &fmt);
	if (ret < 0) {
		Ucamera_LOG("UVC: Unable to set format %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}

	Ucamera_LOG("UVC: Setting format to: %c%c%c%c %ux%u\n",	pixfmtstr(dev->fcc), dev->width, dev->height);

	return 0;
}


static int
uvc_video_stream(struct uvc_device *dev, int enable)
{
	int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	int ret;

	if (!enable) {
		ret = ioctl(dev->uvc_fd, VIDIOC_STREAMOFF, &type);
		if (ret < 0) {
			Ucamera_LOG("UVC: VIDIOC_STREAMOFF failed: %s (%d).\n",
					strerror(errno), errno);
			return ret;
		}

		Ucamera_LOG("UVC: Stopping video stream.\n");

		return 0;
	}

	ret = ioctl(dev->uvc_fd, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		Ucamera_LOG("UVC: Unable to start streaming %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}

	Ucamera_LOG("UVC: Starting video stream.\n");

	dev->uvc_shutdown_requested = 0;

	return 0;
}

static int
uvc_uninit_device(struct uvc_device *dev)
{
	unsigned int i;
	int ret;

	switch (dev->io) {
	case IO_METHOD_MMAP:
		for (i = 0; i < dev->nbufs; ++i) {
			ret = munmap(dev->mem[i].start, dev->mem[i].length);
			if (ret < 0) {
				Ucamera_LOG("UVC: munmap failed\n");
				return ret;
			}
		}

		free(dev->mem);
		break;

	case IO_METHOD_USERPTR:
	default:
		if (dev->run_standalone) {
			for (i = 0; i < dev->nbufs; ++i)
				free(dev->dummy_buf[i].start);

			free(dev->dummy_buf);
		}
		break;
	}

	return 0;
}

static int
uvc_open(struct uvc_device **uvc, char *devname)
{
	struct uvc_device *dev;
	struct v4l2_capability cap;
	int fd;
	int ret = -EINVAL;

	fd = open(devname, O_RDWR | O_NONBLOCK);
	if (fd == -1) {
		Ucamera_LOG("UVC: device open failed: %s (%d).\n",
				strerror(errno), errno);
		return ret;
	}

	ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		Ucamera_LOG("UVC: unable to query uvc device: %s (%d)\n",
				strerror(errno), errno);
		goto err;
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) {
		Ucamera_LOG("UVC: %s is no video output device\n", devname);
		goto err;
	}

	dev = calloc(1, sizeof *dev);
	if (dev == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	//Ucamera_LOG("uvc device is %s on bus %s\n", cap.card, cap.bus_info);
	//Ucamera_LOG("uvc open succeeded, file descriptor = %d\n", fd);

	dev->uvc_fd = fd;
	*uvc = dev;

	return 0;

err:
	close(fd);
	return ret;
}

static void
uvc_close(struct uvc_device *dev)
{
	close(dev->uvc_fd);
	free(dev->imgdata);
	free(dev);
}

/* ---------------------------------------------------------------------------
 * UVC streaming related
 */

static void
uvc_video_fill_buffer(struct uvc_device *dev, struct v4l2_buffer *buf)
{
	unsigned int len = 0;
	struct Ucamera_Video_CB_Func *v_func = dev->v_func;

	switch (dev->fcc) {
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_NV12:
			/* Fill the buffer with video data. */
			len = v_func->get_YuvFrame(dev->mem[buf->index].start);
			buf->bytesused = len > 0 ? len : dev->imgsize;
			//Ucamera_LOG("UVC video get yuv frame len:%d\n", len);
			break;

		case V4L2_PIX_FMT_MJPEG:

			len = v_func->get_JpegFrame(dev->mem[buf->index].start);
			buf->bytesused = len > 0 ? len : dev->imgsize;
			if (len > dev->imgsize) {
				Ucamera_LOG("UVC: image size too big!\n");
			}
			//Ucamera_LOG("UVC video get jpeg frame len:%d\n", len);
			break;
		case V4L2_PIX_FMT_H264:
		#if 1
			len = v_func->get_H264Frame(dev->mem[buf->index].start);
			buf->bytesused = len > 0 ? len : dev->imgsize;
			//memset(dev->mem[buf->index].start, 0xaa, len);
			if (len > dev->imgsize) {
				Ucamera_LOG("UVC: image size too large!\n");
			}
			//Ucamera_LOG("UVC video get H264 frame len:%d\n", len);
		#else
			{
				node_t *node = Get_Use_Node(frame_list);
				memcpy(dev->mem[buf->index].start, node->data, node->size);
				//memset(dev->mem[buf->index].start, 0xaa, node->size);
				buf->bytesused = node->size;
				//Ucamera_LOG("UVC video get h264 frame len:%d\n", buf->bytesused);
				Put_Free_Node(frame_list, node);
			}
		#endif
			/*if(stream_fp)
				fwrite(dev->mem[buf->index].start, len, 1, stream_fp);*/
			break;

	}

}

static int
uvc_video_process(struct uvc_device *dev)
{
	struct v4l2_buffer ubuf;
	int ret;

	/*
	 * Return immediately if UVC video output device has not started
	 * streaming yet.
	 */
	if (!dev->is_streaming) {
		usleep(100);
		return 0;
	}

	/* Prepare a v4l2 buffer to be dequeued from UVC domain. */
	CLEAR(ubuf);

	ubuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	switch (dev->io) {
		case IO_METHOD_MMAP:
			ubuf.memory = V4L2_MEMORY_MMAP;
			break;

		case IO_METHOD_USERPTR:
		default:
			ubuf.memory = V4L2_MEMORY_USERPTR;
			break;
	}

	if (dev->run_standalone) {
		/* UVC stanalone setup. */
		ret = ioctl(dev->uvc_fd, VIDIOC_DQBUF, &ubuf);
		if (ret < 0)
			return ret;

		dev->dqbuf_count++;

#ifdef ENABLE_BUFFER_DEBUG
		Ucamera_LOG("DeQueued buffer at UVC side = %d\n", ubuf.index);
#endif
		uvc_video_fill_buffer(dev, &ubuf);

		ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &ubuf);
		if (ret < 0) {
			Ucamera_LOG("UVC: Unable to queue buffer: %s (%d).\n",
					strerror(errno), errno);
			return ret;
		}

		dev->qbuf_count++;

#ifdef ENABLE_BUFFER_DEBUG
		Ucamera_LOG("ReQueueing buffer at UVC side = %d\n", ubuf.index);
#endif
	}

	return 0;
}

static int
uvc_video_qbuf_mmap(struct uvc_device *dev)
{
	unsigned int i;
	int ret;

	for (i = 0; i < dev->nbufs; ++i) {
		memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

		dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
		dev->mem[i].buf.index = i;

		/* UVC standalone setup. */
		if (dev->run_standalone)
			uvc_video_fill_buffer(dev, &(dev->mem[i].buf));

		ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &(dev->mem[i].buf));
		if (ret < 0) {
			Ucamera_LOG("UVC: VIDIOC_QBUF failed : %s (%d).\n",
					strerror(errno), errno);
			return ret;
		}

		dev->qbuf_count++;
	}

	return 0;
}

static int
uvc_video_qbuf_userptr(struct uvc_device *dev)
{
	unsigned int i;
	int ret;

	/* UVC standalone setup. */
	if (dev->run_standalone) {
		for (i = 0; i < dev->nbufs; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
			buf.memory = V4L2_MEMORY_USERPTR;
			buf.m.userptr = (unsigned long)dev->dummy_buf[i].start;
			buf.length = dev->dummy_buf[i].length;
			buf.index = i;

			ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &buf);
			if (ret < 0) {
				Ucamera_LOG("UVC: VIDIOC_QBUF failed : %s (%d).\n",
						strerror(errno), errno);
				return ret;
			}

			dev->qbuf_count++;
		}
	}

	return 0;
}

static int
uvc_video_qbuf(struct uvc_device *dev)
{
	int ret = 0;

	switch (dev->io) {
	case IO_METHOD_MMAP:
		ret = uvc_video_qbuf_mmap(dev);
		break;

	case IO_METHOD_USERPTR:
		ret = uvc_video_qbuf_userptr(dev);
		break;

	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int
uvc_video_reqbufs_mmap(struct uvc_device *dev, int nbufs)
{
	struct v4l2_requestbuffers rb;
	unsigned int i;
	int ret;

	CLEAR(rb);
	//Ucamera_LOG("UVC: start to mmap\n");
	rb.count = nbufs;
	rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	rb.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(dev->uvc_fd, VIDIOC_REQBUFS, &rb);
	if (ret < 0) {
		if (ret == -EINVAL)
			Ucamera_LOG("UVC: does not support memory mapping\n");
		else
			Ucamera_LOG("UVC: Unable to allocate buffers: %s (%d).\n",
					strerror(errno), errno);
		goto err;
	}

	if (!rb.count)
		return 0;
#if 0
	if (rb.count < 2) {
		Ucamera_LOG("UVC: Insufficient buffer memory.\n");
		ret = -EINVAL;
		goto err;
	}
#endif
	/* Map the buffers. */
	dev->mem = calloc(rb.count, sizeof dev->mem[0]);
	if (!dev->mem) {
		Ucamera_LOG("UVC: Out of memory\n");
		ret = -ENOMEM;
		goto err;
	}

	for (i = 0; i < rb.count; ++i) {
		memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

		dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
		dev->mem[i].buf.index = i;

		ret = ioctl(dev->uvc_fd, VIDIOC_QUERYBUF, &(dev->mem[i].buf));
		if (ret < 0) {
			Ucamera_LOG("UVC: VIDIOC_QUERYBUF failed for buf %d: "
				"%s (%d).\n", i, strerror(errno), errno);
			ret = -EINVAL;
			goto err_free;
		}

		dev->mem[i].start = mmap(NULL /* start anywhere */,
					dev->mem[i].buf.length,
					PROT_READ | PROT_WRITE /* required */,
					MAP_SHARED /* recommended */,
					dev->uvc_fd, dev->mem[i].buf.m.offset);

		if (MAP_FAILED == dev->mem[i].start) {
			Ucamera_LOG("UVC: Unable to map buffer %u: %s (%d).\n", i,
				strerror(errno), errno);
			dev->mem[i].length = 0;
			ret = -EINVAL;
			goto err_free;
		}

		dev->mem[i].length = dev->mem[i].buf.length;
		//Ucamera_LOG("UVC: Buffer %u mapped at address %p len:%d.\n", i,
		//		dev->mem[i].start, dev->mem[i].buf.length);
	}

	dev->nbufs = rb.count;
	//Ucamera_LOG("UVC: %u buffers allocated.\n", rb.count);

	return 0;

err_free:
	free(dev->mem);
err:
	return ret;
}

static int
uvc_video_reqbufs_userptr(struct uvc_device *dev, int nbufs)
{
	struct v4l2_requestbuffers rb;
	unsigned int i, j, bpl = 0, payload_size = 0;
	int ret;

	CLEAR(rb);

	rb.count = nbufs;
	rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	rb.memory = V4L2_MEMORY_USERPTR;

	ret = ioctl(dev->uvc_fd, VIDIOC_REQBUFS, &rb);
	if (ret < 0) {
		if (ret == -EINVAL)
			Ucamera_LOG("UVC: does not support user pointer i/o\n");
		else
			Ucamera_LOG("UVC: VIDIOC_REQBUFS error %s (%d).\n",
					strerror(errno), errno);
		goto err;
	}

	if (!rb.count)
		return 0;

	dev->nbufs = rb.count;
	//Ucamera_LOG("UVC: %u buffers allocated.\n", rb.count);

	if (dev->run_standalone) {
		/* Allocate buffers to hold dummy data pattern. */
		dev->dummy_buf = calloc(rb.count, sizeof dev->dummy_buf[0]);
		if (!dev->dummy_buf) {
			Ucamera_LOG("UVC: Out of memory\n");
			ret = -ENOMEM;
			goto err;
		}

		switch (dev->fcc) {
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_NV12:
			bpl = dev->width * 2;
			payload_size = dev->width * dev->height * 2;
			break;
		case V4L2_PIX_FMT_MJPEG:
		case V4L2_PIX_FMT_H264:
			payload_size = dev->imgsize;
			break;
		}

		for (i = 0; i < rb.count; ++i) {
			dev->dummy_buf[i].length = payload_size;
			dev->dummy_buf[i].start = malloc(payload_size);
			if (!dev->dummy_buf[i].start) {
				Ucamera_LOG("UVC: Out of memory\n");
				ret = -ENOMEM;
				goto err;
			}

			if (V4L2_PIX_FMT_YUYV == dev->fcc || V4L2_PIX_FMT_NV12 == dev->fcc)
				for (j = 0; j < dev->height; ++j)
					memset(dev->dummy_buf[i].start + j*bpl,
						dev->color++, bpl);

			if (V4L2_PIX_FMT_MJPEG == dev->fcc)
				memcpy(dev->dummy_buf[i].start, dev->imgdata,
						dev->imgsize);
		}
	}

	return 0;

err:
	return ret;

}

static int
uvc_video_reqbufs(struct uvc_device *dev, int nbufs)
{
	int ret = 0;

	switch (dev->io) {
	case IO_METHOD_MMAP:
		ret = uvc_video_reqbufs_mmap(dev, nbufs);
		break;

	case IO_METHOD_USERPTR:
		ret = uvc_video_reqbufs_userptr(dev, nbufs);
		break;

	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/*
 * This function is called in response to either:
 * 	- A SET_ALT(interface 1, alt setting 1) command from USB host,
 * 	  if the UVC gadget supports an ISOCHRONOUS video streaming endpoint
 * 	  or,
 *
 *	- A UVC_VS_COMMIT_CONTROL command from USB host, if the UVC gadget
 *	  supports a BULK type video streaming endpoint.
 */
static int
uvc_handle_streamon_event(struct uvc_device *dev)
{
	int ret;

	ret = uvc_video_reqbufs(dev, dev->nbufs);
	if (ret < 0)
		goto err;

	/* Common setup. */

	/* Queue buffers to UVC domain and start streaming. */
	ret = uvc_video_qbuf(dev);
	if (ret < 0)
		goto err;

	if (dev->run_standalone) {
		uvc_video_stream(dev, 1);
		dev->first_buffer_queued = 1;
		dev->is_streaming = 1;
	}

	return 0;

err:
	return ret;
}

/* ---------------------------------------------------------------------------
 * UVC Request processing
 */

void uvc_streaming_control_show(struct uvc_streaming_control *ctrl)
{
	printf("\n");
	printf("bmHint:					%d\n", ctrl->bmHint);
	printf("bFormatIndex:			%d\n", ctrl->bFormatIndex);
	printf("bFrameIndex:			%d\n", ctrl->bFrameIndex);
	printf("dwFrameInterval:		%d\n", ctrl->dwFrameInterval);
	printf("wKeyFrameRate:			%d\n", ctrl->wKeyFrameRate);
	printf("wPFrameRate:			%d\n", ctrl->wPFrameRate);
	printf("wCompQuality:			%d\n", ctrl->wCompQuality);
	printf("wCompWindowSize:		%d\n", ctrl->wCompWindowSize);
	printf("wDelay:					%d\n", ctrl->wDelay);
	printf("dwMaxVideoFrameSize:	%d\n", ctrl->dwMaxVideoFrameSize);
	printf("dwMaxPayloadTransferSize:%d\n", ctrl->dwMaxPayloadTransferSize);
	printf("dwClockFrequency:		%d\n", ctrl->dwClockFrequency);
	printf("bmFramingInfo:			%d\n", ctrl->bmFramingInfo);
	printf("bPreferedVersion:		%d\n", ctrl->bPreferedVersion);
	printf("bMinVersion:			%d\n", ctrl->bMinVersion);
	printf("bMaxVersion:			%d\n", ctrl->bMaxVersion);
	printf("\n");
	return;
}

static void
uvc_fill_streaming_still_control(struct uvc_device *dev,
			   struct uvc_streaming_still_control *ctrl,
			   int iframe, int iformat)
{
	const struct uvc_format_info *format;
	unsigned int nframes;

	if (iformat < 0)
		iformat = g_nFormats + iformat;
	if (iformat < 0 || iformat >= g_nFormats)
		return;
	format = uvc_formats[iformat];

	nframes = 0;
	while (format->frames[nframes].width != 0)
		++nframes;

	if (iframe < 0)
		iframe = nframes + iframe;
	if (iframe < 0 || iframe >= (int)nframes)
		return;

	memset(ctrl, 0, sizeof *ctrl);

	ctrl->bFormatIndex = 1;
	ctrl->bFrameIndex = 1;
	ctrl->wCompQuality = 0xf8;
	ctrl->dwMaxVideoFrameSize = 0x3f4a4df8;
	ctrl->dwMaxPayloadTransferSize = 0;

}

static void
uvc_fill_streaming_control(struct uvc_device *dev,
			   struct uvc_streaming_control *ctrl,
			   int iframe, int iformat)
{
	const struct uvc_format_info *format;
	const struct uvc_frame_info *frame;
	unsigned int nframes;

	if (iformat < 0)
		iformat = g_nFormats + iformat;
	if (iformat < 0 || iformat >= g_nFormats)
		return;
	format = uvc_formats[iformat];

	nframes = 0;
	while (format->frames[nframes].width != 0)
		++nframes;

	if (iframe < 0)
		iframe = nframes + iframe;
	if (iframe < 0 || iframe >= (int)nframes)
		return;
	frame = &format->frames[iframe];

	memset(ctrl, 0, sizeof *ctrl);

	ctrl->bmHint = 1;
	ctrl->bFormatIndex = iformat + 1;
	ctrl->bFrameIndex = iframe + 1;
	ctrl->dwFrameInterval = frame->intervals[0];
	switch (format->fcc) {
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_NV12:
		ctrl->dwMaxVideoFrameSize = frame->width * frame->height * 2;
		break;
	case V4L2_PIX_FMT_MJPEG:
	case V4L2_PIX_FMT_H264:
		ctrl->dwMaxVideoFrameSize = dev->imgsize;
		break;
	}

	/* TODO: the UVC maxpayload transfer size should be filled
	 * by the driver.
	 */
	if (!dev->bulk)
		ctrl->dwMaxPayloadTransferSize = (dev->maxpkt) *
					(dev->mult + 1) * (dev->burst + 1);
	else
		ctrl->dwMaxPayloadTransferSize = ctrl->dwMaxVideoFrameSize;

	ctrl->bmFramingInfo = 3;
	ctrl->bPreferedVersion = 1;
	ctrl->bMaxVersion = 1;
	//uvc_streaming_control_show(ctrl);
}

static void
uvc_events_process_standard(struct uvc_device *dev,
			    struct usb_ctrlrequest *ctrl,
			    struct uvc_request_data *resp)
{
	Ucamera_LOG("standard request\n");
	(void)dev;
	(void)ctrl;
	(void)resp;
}

static void
uvc_events_process_control(struct uvc_device *dev, uint8_t req,
			   uint8_t cs, uint8_t entity_id,
			   uint8_t len, struct uvc_request_data *resp)
{
#ifdef ENABLE_USB_REQUEST_DEBUG
	Ucamera_LOG("UVC: control request (req %s cs %02x) entity_id: %d\n", uvc_query_name(req), cs, entity_id);
#endif
	int value = 0;
	int type = 0;
	struct Ucamera_Video_PU_Control *puctl = dev->v_puctl;
	struct Ucamera_Video_EU_Control *euctl = dev->v_euctl;
	struct Ucamera_Video_CT_Control *ctctl = dev->v_ctctl;

	switch (entity_id) {
	case 0:
		switch (cs) {
		case UVC_VC_REQUEST_ERROR_CODE_CONTROL:
			/* Send the request error code last prepared. */
			resp->data[0] = dev->request_error_code.data[0];
			resp->length = dev->request_error_code.length;
			break;

		default:
			/*
			 * If we were not supposed to handle this
			 * 'cs', prepare an error code response.
			 */
			dev->request_error_code.data[0] = 0x06;
			dev->request_error_code.length = 1;
			break;
		}
		break;

	/* Camera terminal unit 'UVC_VC_INPUT_TERMINAL'. */
	case UVC_INPUT_TERMINAL_ID:
		switch (cs) {
		case UVC_CT_ZOOM_ABSOLUTE_CONTROL:
                case UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
                case UVC_CT_FOCUS_ABSOLUTE_CONTROL:
			switch (req) {
			case UVC_SET_CUR:
				dev->puctl_cur = cs;
				dev->unit_cur = entity_id;
				resp->data[0] = 0x0;
				resp->length = 2;

				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
			case UVC_GET_MIN:
				resp->length = 2;
				memcpy(&resp->data[0], &ctctl[cs-1].data[UVC_MIN], resp->length);

				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_MAX:
				resp->length = 2;
				memcpy(&resp->data[0], &ctctl[cs-1].data[UVC_MAX], resp->length);

				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_CUR:
				if (ctctl[cs-1].get != NULL) {
                                value = ctctl[cs-1].get();
				}

				/*value = 2;*/
				resp->length = 2;
				memcpy(&resp->data[0], &value, resp->length);
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_INFO:
				/*
				 * We support Set and Get requests and don't
				 * support async updates on an interrupt endpt
				 */
				resp->data[0] = 0x0F;
				resp->length = 1;
				/*
				 * For every successfully handled control
				 * request, set the request error code to no
				 * error.
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_DEF:
				value = ctctl[cs-1].data[UVC_DEF];
				resp->length = 2;
				memcpy(&resp->data[0], &value, resp->length);
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error.
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_RES:
				value = 0x01 ;
				resp->length = 2;
				memcpy(&resp->data[0], &value, resp->length);
				/*
				 * For every successfully handled control
				 * request, set the request error code to no
				 * error.
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			default:
				/*
				 * We don't support this control, so STALL the
				 * default control ep.
				 */
				resp->length = -EL2HLT;
				/*
				 * For every unsupported control request
				 * set the request error code to appropriate
				 * code.
				 */
				dev->request_error_code.data[0] = 0x07;
				dev->request_error_code.length = 1;
				break;
			}
			break;
		/*
		 * We support only 'UVC_CT_AE_MODE_CONTROL' for CAMERA
		 * terminal, as our bmControls[0] = 2 for CT. Also we
		 * support only auto exposure.
		 */
		case UVC_CT_AE_MODE_CONTROL:
                case UVC_CT_FOCUS_AUTO_CONTROL:
			switch (req) {
			case UVC_SET_CUR:
				/* Incase of auto exposure, attempts to
				 * programmatically set the auto-adjusted
				 * controls are ignored.
				 */
				dev->puctl_cur = cs;
				dev->unit_cur = entity_id;
				resp->data[0] = 0x0;
				resp->length = len;
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				resp->data[0] = 0x01;
				resp->length = 1;
				break;

			case UVC_GET_CUR:
				if (ctctl[cs-1].get != NULL) {
                                    value = ctctl[cs-1].get();
				}
                                else
                                    value = 0;

				resp->length = 1;
				memcpy(&resp->data[0], &value, resp->length);
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;

			case UVC_GET_DEF:
                                value = ctctl[cs-1].data[UVC_DEF];
				resp->length = 1;
				memcpy(&resp->data[0], &value, resp->length);
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;

			case UVC_GET_INFO:
				/*
				 * TODO: We support Set and Get requests, but
				 * don't support async updates on an video
				 * status (interrupt) endpoint as of
				 * now.
				 */
				resp->data[0] = 0x03;
				resp->length = 1;
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error.
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;

			case UVC_GET_RES:
				/* Auto Mode a?? auto Exposure Time, auto Iris. */
				resp->data[0] = 0x01;
				resp->length = 1;
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error.
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			default:
				/*
				 * We don't support this control, so STALL the
				 * control ep.
				 */
				resp->length = -EL2HLT;
				/*
				 * For every unsupported control request
				 * set the request error code to appropriate
				 * value.
				 */
				dev->request_error_code.data[0] = 0x07;
				dev->request_error_code.length = 1;
				break;
			}
			break;

		default:
			/*
			 * We don't support this control, so STALL the control
			 * ep.
			 */
			resp->length = -EL2HLT;
			/*
			 * If we were not supposed to handle this
			 * 'cs', prepare a Request Error Code response.
			 */
			dev->request_error_code.data[0] = 0x06;
			dev->request_error_code.length = 1;
			break;
		}
		break;

	/* processing unit 'UVC_VC_PROCESSING_UNIT' */
	case UVC_PROCESSING_UNIT_ID:
		switch (cs) {
		/*
		 * We support only 'UVC_PU_BRIGHTNESS_CONTROL' for Processing
		 * Unit, as our bmControls[0] = 1 for PU.
		 */
		case UVC_PU_BACKLIGHT_COMPENSATION_CONTROL:
		case UVC_PU_BRIGHTNESS_CONTROL:
		case UVC_PU_CONTRAST_CONTROL:
		//case UVC_PU_GAIN_CONTROL:
		case UVC_PU_HUE_CONTROL:
		case UVC_PU_SATURATION_CONTROL:
		case UVC_PU_SHARPNESS_CONTROL:
                case UVC_PU_GAMMA_CONTROL:
                case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
			switch (req) {
			case UVC_SET_CUR:
#if 0
				if (puctl[cs-1].set != NULL) {
					puctl[cs-1].set(10);
					puctl[cs-1].data[UVC_CUR] = 10;
				}
#endif
				dev->puctl_cur = cs;
				dev->unit_cur = entity_id;
				resp->data[0] = 0x0;
				resp->length = len;
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_MIN:
				resp->length = 2;
				memcpy(&resp->data[0], &puctl[cs-1].data[UVC_MIN], resp->length);
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_MAX:
				resp->length = 2;
				memcpy(&resp->data[0], &puctl[cs-1].data[UVC_MAX], resp->length);
				//resp->data[0] = 0;//puctl[cs-1].data[UVC_MAX];
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_CUR:
				if (puctl[cs-1].get != NULL) {
					value = puctl[cs-1].get();
					/* puctl[cs-1].data[UVC_CUR] = value & 0xffff; */
				}

				/* value = puctl[cs-1].data[UVC_CUR]; */
				resp->length = 2;
				memcpy(&resp->data[0], &value, resp->length);
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_INFO:
				/*
				 * We support Set and Get requests and don't
				 * support async updates on an interrupt endpt
				 */
				resp->data[0] = 0x03;
				resp->length = 1;
				/*
				 * For every successfully handled control
				 * request, set the request error code to no
				 * error.
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_DEF:
				if ((cs == UVC_PU_GAMMA_CONTROL) && (puctl[cs-1].get != NULL)) {
					value = puctl[cs-1].get();
					puctl[cs-1].data[UVC_DEF] = value & 0xffff;
				}
				value = puctl[cs-1].data[UVC_DEF];
				resp->length = 2;
				memcpy(&resp->data[0], &value, resp->length);
				//resp->data[0] = PU_BRIGHTNESS_DEFAULT_VAL;
				//resp->length = 2;
				/*
				 * For every successfully handled control
				 * request, set the request error code to no
				 * error.
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_RES:
				resp->data[0] = 1;
				resp->length = 2;
				/*
				 * For every successfully handled control
				 * request, set the request error code to no
				 * error.
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			default:
				/*
				 * We don't support this control, so STALL the
				 * default control ep.
				 */
				resp->length = -EL2HLT;
				/*
				 * For every unsupported control request
				 * set the request error code to appropriate
				 * code.
				 */
				dev->request_error_code.data[0] = 0x07;
				dev->request_error_code.length = 1;
				break;
			}
			break;
		case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
			switch (req) {
			case UVC_SET_CUR:
			#if 0
				if (puctl[cs-1].set != NULL) {
					puctl[cs-1].set(10);
					puctl[cs-1].data[UVC_CUR] = 10;
				}
			#endif
				dev->puctl_cur = cs;
				dev->unit_cur = entity_id;
				resp->data[0] = 0x0;
				resp->length = len;
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_MIN:
				resp->length = 1;
				memcpy(&resp->data[0], &puctl[cs-1].data[UVC_MIN], resp->length);
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_MAX:
				resp->length = 1;
				memcpy(&resp->data[0], &puctl[cs-1].data[UVC_MAX], resp->length);
				//resp->data[0] = 0;//puctl[cs-1].data[UVC_MAX];
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_CUR:
				if (puctl[cs-1].get != NULL) {
					value = puctl[cs-1].get();
					/* puctl[cs-1].data[UVC_CUR] = value & 0xffff; */
				}
				/* value = puctl[cs-1].data[UVC_CUR]; */
				resp->length = 1;
				memcpy(&resp->data[0], &value, resp->length);
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_INFO:
				/*
				 * We support Set and Get requests and don't
				 * support async updates on an interrupt endpt
				 */
				resp->data[0] = 0x03;
				resp->length = 1;

				/*
				 * For every successfully handled control
				 * request, set the request error code to no
				 * error.
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_DEF:
				value = puctl[cs-1].data[UVC_DEF];
				resp->length = 1;
				memcpy(&resp->data[0], &value, resp->length);
				//resp->data[0] = PU_BRIGHTNESS_DEFAULT_VAL;
				//resp->length = 2;
				/*
				 * For every successfully handled control
				 * request, set the request error code to no
				 * error.
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_RES:
				resp->data[0] = 1;
				resp->length = 2;
				/*
				 * For every successfully handled control
				 * request, set the request error code to no
				 * error.
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			default:
				/*
				 * We don't support this control, so STALL the
				 * default control ep.
				 */
				resp->length = -EL2HLT;
				/*
				 * For every unsupported control request
				 * set the request error code to appropriate
				 * code.
				 */
				dev->request_error_code.data[0] = 0x07;
				dev->request_error_code.length = 1;
				break;
			}
			break;

		case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
			switch (req) {
			case UVC_SET_CUR:
				dev->puctl_cur = cs;
				dev->unit_cur = entity_id;
				resp->data[0] = 0x0;
				resp->length = len;
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_CUR:

				if (puctl[UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL-1].get
						!= NULL) {
					value = puctl[UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL-1].get();
				}
				if ((value & 0xffff0000) >> 16 == 0)
					resp->data[0] = 1;
				else
					resp->data[0] = 0;
				/* puctl[cs-1].data[UVC_CUR] = resp->data[0]; */
				resp->length = 1;
				/*
				 * For every successfully handled control
				 * request set the request error code to no
				 * error
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;
			case UVC_GET_DEF:
				resp->data[0] = 1;
				resp->length = 1;
				/*
				 * For every successfully handled control
				 * request, set the request error code to no
				 * error.
				 */
				dev->request_error_code.data[0] = 0x00;
				dev->request_error_code.length = 1;
				break;

			default:
				/*
				 * We don't support this control, so STALL the
				 * default control ep.
				 */
				resp->length = -EL2HLT;
				/*
				 * For every unsupported control request
				 * set the request error code to appropriate
				 * code.
				 */
				dev->request_error_code.data[0] = 0x07;
				dev->request_error_code.length = 1;
				break;
			}
			break;

		default:
			/*
			 * We don't support this control, so STALL the control
			 * ep.
			 */
			resp->length = -EL2HLT;
			/*
			 * If we were not supposed to handle this
			 * 'cs', prepare a Request Error Code response.
			 */
			dev->request_error_code.data[0] = 0x06;
			dev->request_error_code.length = 1;
			break;
		}

		break;

	/* processing unit 'UVC_VC_EXTENSION_UNIT' */
	case UVC_EXTENSION_UNIT_ID:
#if 0
	 Ucamera_LOG("UVC: control request (req %s cs %02x) entity_id: %d\n", uvc_query_name(req), cs, entity_id); 
#endif
	 /*switch (cs) {*/
		 /*case UVC_EU_CMD_USR1:*/
		 /*case UVC_EU_CMD_USR2:*/
		 /*case UVC_EU_CMD_USR3:*/
		 /*case UVC_EU_CMD_USR4:*/
		 /*case UVC_EU_CMD_USR5:*/
		 /*case UVC_EU_CMD_USR6:*/
		 /*case UVC_EU_CMD_USR7:*/
		 /*case UVC_EU_CMD_USR8:*/
			 switch (req) {
				 case UVC_SET_CUR:
					 dev->puctl_cur = cs;
					 dev->unit_cur = entity_id;
					 resp->data[0] = 0x0;
					 resp->length = len;
					 /*
					  * For every successfully handled control
					  * request set the request error code to no
					  * error
					  */
					 dev->request_error_code.data[0] = 0x00;
					 dev->request_error_code.length = 1;
					 break;
				 case UVC_GET_LEN:
					    /*resp->data[0] = euctl[cs-1].len;*/
					    /*resp->data[0] = 32;*/
					    xu_req_set_GetLen(cs, resp->data);
					    /*resp->data[1] = 0x00;*/
					    resp->length = 2;
					    dev->request_error_code.data[0] = 0x00;
					    dev->request_error_code.length = 1;
					 break;
				 case UVC_GET_CUR:
					 xu_req_process_usr(cs, NULL, 0, resp->data, &resp->length);
					 dev->request_error_code.data[0] = 0x00;
					 dev->request_error_code.length = 1;
					 break;
				 case UVC_GET_DEF:
				 case UVC_GET_MAX:
				 case UVC_GET_RES:
				 case UVC_GET_MIN:
					 resp->data[0] = 0x00;
					 resp->length = 0x01;
					 dev->request_error_code.data[0] = 0x00;
					 dev->request_error_code.length = 1;
					 break;
				 case UVC_GET_INFO:
					 /*
					  * We support Set and Get requests and don't
					  * support async updates on an interrupt endpt
					  */
					 resp->data[0] = 0x03;
					 resp->length = 1;
					 /*
					  * For every successfully handled control
					  * request, set the request error code to no
					  * error.
					  */
					 dev->request_error_code.data[0] = 0x00;
					 dev->request_error_code.length = 1;
					 break;
				 default:
					 /*
					  * We don't support this control, so STALL the
					  * default control ep.
					  */
					 resp->length = -EL2HLT;
					 /*
					  * For every unsupported control request
					  * set the request error code to appropriate
					  * code.
					  */
					 dev->request_error_code.data[0] = 0x07;
					 dev->request_error_code.length = 1;
					 break;
			 }
			 break;
	/* }*/
	 break;
	default:
	 /*
	  * If we were not supposed to handle this
	  * 'cs', prepare a Request Error Code response.
	  */
	 dev->request_error_code.data[0] = 0x06;
	 dev->request_error_code.length = 1;
	 break;
	}

}

static void
uvc_events_process_streaming(struct uvc_device *dev, uint8_t req, uint8_t cs,
			     struct uvc_request_data *resp)
{
	struct uvc_streaming_control *ctrl;
	struct uvc_streaming_still_control *ctrl2;
#ifdef ENABLE_USB_REQUEST_DEBUG
	Ucamera_LOG("UVC: streaming request (req %s cs %02x)\n", uvc_query_name(req), cs);
#endif

	if (cs != UVC_VS_PROBE_CONTROL && cs != UVC_VS_COMMIT_CONTROL && cs != UVC_VS_STILL_PROBE_CONTROL &&
	    cs != UVC_VS_STILL_COMMIT_CONTROL && cs != UVC_VS_STILL_IMAGE_TRIGGER_CONTROL)
		return;

	ctrl = (struct uvc_streaming_control *)&resp->data;
	ctrl2 = (struct uvc_streaming_still_control *)&resp->data;

	resp->length = sizeof *ctrl;
	if (cs == UVC_VS_STILL_PROBE_CONTROL)
		resp->length = sizeof *ctrl2;

	switch (req) {
	case UVC_SET_CUR:
		dev->control = cs;
		break;

	case UVC_GET_CUR:
		if (cs == UVC_VS_PROBE_CONTROL)
			memcpy(ctrl, &dev->probe, sizeof *ctrl);
		else if(cs == UVC_VS_COMMIT_CONTROL)
			memcpy(ctrl, &dev->commit, sizeof *ctrl);
		else
			memcpy(ctrl2, &dev->still, sizeof *ctrl2);
		break;

	case UVC_GET_MIN:
	case UVC_GET_MAX:
	case UVC_GET_DEF:
		if (cs == UVC_VS_STILL_PROBE_CONTROL)
			uvc_fill_streaming_still_control(dev, ctrl2, 0, 0);
		else
			uvc_fill_streaming_control(dev, ctrl, req == UVC_GET_MAX ? -1 : 0,
						req == UVC_GET_MAX ? -1 : 0);
		break;

	case UVC_GET_RES:
		CLEAR(ctrl);
		break;

	case UVC_GET_LEN:
		resp->data[0] = 0x00;
		resp->data[1] = 0x22;
		resp->length = 2;
		break;

	case UVC_GET_INFO:
		resp->data[0] = 0x03;
		resp->length = 1;
		break;
	}
}

static void
uvc_events_process_class(struct uvc_device *dev, struct usb_ctrlrequest *ctrl,
			 struct uvc_request_data *resp)
{
	if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
		return;

	switch (ctrl->wIndex & 0xff) {
	case UVC_INTF_CONTROL:
		uvc_events_process_control(dev, ctrl->bRequest,
						ctrl->wValue >> 8,
						ctrl->wIndex >> 8,
						ctrl->wLength, resp);
		break;

	case UVC_INTF_STREAMING:
		uvc_events_process_streaming(dev, ctrl->bRequest,
						ctrl->wValue >> 8, resp);
		break;

	default:
		break;
	}
}

static void
uvc_events_process_setup(struct uvc_device *dev, struct usb_ctrlrequest *ctrl,
			 struct uvc_request_data *resp)
{
	dev->control = 0;

#ifdef ENABLE_USB_REQUEST_DEBUG
	Ucamera_LOG("UVC: bRequestType %02x bRequest %02x wValue %04x wIndex %04x "
		"wLength %04x\n", ctrl->bRequestType, ctrl->bRequest,
		ctrl->wValue, ctrl->wIndex, ctrl->wLength);
#endif

	switch (ctrl->bRequestType & USB_TYPE_MASK) {
	case USB_TYPE_STANDARD:
		uvc_events_process_standard(dev, ctrl, resp);
		break;

	case USB_TYPE_CLASS:
		uvc_events_process_class(dev, ctrl, resp);
		break;

	default:
		break;
	}
}

static int uvc_update_ctl_data_parse(uint8_t cs, struct uvc_request_data *data)
{
/*
	struct Ucamera_Video_EU_Control *euctl = g_udev->v_euctl;

	switch (cs) {
	case UVC_EU_CMD_USR1:
	case UVC_EU_CMD_USR2:
	case UVC_EU_CMD_USR3:
	case UVC_EU_CMD_USR4:
	case UVC_EU_CMD_USR5:
	case UVC_EU_CMD_USR6:
	case UVC_EU_CMD_USR7:
	case UVC_EU_CMD_USR8:
		if (euctl[cs-1].set != NULL)
			euctl[cs-1].set(data->data, data->length);
		break;
	default:
		break;

	}
*/
	int ret = -1;
	ret = xu_req_process_usr(cs, data->data, data->length, NULL, NULL);
	if (ret < 0) {
		Ucamera_LOG("xu_req_process_usr failed! (cs : %d)\n", cs);
	}

	return 0;
}

static int
uvc_events_process_control_data(struct uvc_device *dev,
				uint8_t cs, uint8_t entity_id,
				struct uvc_request_data *data)
{
	int type, tmp = 0;
	int value = 0;
	struct Ucamera_Video_PU_Control *puctl = dev->v_puctl;
	struct Ucamera_Video_CT_Control *ctctl = dev->v_ctctl;

	switch (entity_id) {
	case UVC_INPUT_TERMINAL_ID:
            switch (cs){
                case UVC_CT_ZOOM_ABSOLUTE_CONTROL:
                case UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
                case UVC_CT_FOCUS_ABSOLUTE_CONTROL:
                case UVC_CT_AE_MODE_CONTROL:
                case UVC_CT_FOCUS_AUTO_CONTROL:
			memcpy(&value, data->data, data->length);
			if (ctctl[cs-1].set != NULL)
				ctctl[cs-1].set(value);
			ctctl[type-1].data[UVC_CUR] = value;
			break;
		default:
			break;
		}
	/* Processing unit 'UVC_VC_EXTENSION_UNIT'. */
	case UVC_EXTENSION_UNIT_ID:
		uvc_update_ctl_data_parse(cs, data);
		break;
	/* Processing unit 'UVC_VC_PROCESSING_UNIT'. */
	case UVC_PROCESSING_UNIT_ID:
		switch (cs) {
		/*
		 * We support only 'UVC_PU_BRIGHTNESS_CONTROL' for Processing
		 * Unit, as our bmControls[0] = 1 for PU.
		 */
		case UVC_PU_BACKLIGHT_COMPENSATION_CONTROL:
		case UVC_PU_BRIGHTNESS_CONTROL:
		case UVC_PU_CONTRAST_CONTROL:
		case UVC_PU_GAIN_CONTROL:
		case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
		case UVC_PU_HUE_CONTROL:
		case UVC_PU_SATURATION_CONTROL:
		case UVC_PU_SHARPNESS_CONTROL:
		case UVC_PU_GAMMA_CONTROL:
			memcpy(&value, data->data, data->length);
			if (puctl[cs-1].set != NULL)
				puctl[cs-1].set(value);
			puctl[cs-1].data[UVC_CUR] = value;
			/* UVC - V4L2 integrated path. */
			break;
		case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
			memcpy(&value, data->data, data->length);
			if (puctl[UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL-1].set != NULL) {
				if (puctl[UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL-1].data[UVC_CUR] == 0)
					tmp = (1<<16)|value;
					puctl[cs-1].set(tmp);
			}
			puctl[cs-1].data[UVC_CUR] = value;
			break;
		case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
			memcpy(&value, data->data, data->length);
			if (puctl[UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL-1].set != NULL) {
					puctl[UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL-1].set(!value);

			}
			puctl[cs-1].data[UVC_CUR] = value;
			break;
		default:
			break;
		}

		break;

	default:
		break;
	}
#ifdef ENABLE_USB_REQUEST_DEBUG
	Ucamera_LOG("UVC: Control Request data phase (cs 0x%02x entity 0x%02x value 0x%02x)\n",
			cs, entity_id, value);
#endif
	return 0;
}

static int
uvc_events_process_data(struct uvc_device *dev, struct uvc_request_data *data)
{
	struct uvc_streaming_control *target;
	struct uvc_streaming_still_control *target1;
	struct uvc_streaming_control *ctrl;
	//struct v4l2_format fmt;				// FIXME -- Unused right now.
	const struct uvc_format_info *format;
	const struct uvc_frame_info *frame;
	const unsigned int *interval;
	unsigned int iformat, iframe;
	unsigned int nframes;
	int ret;

	switch (dev->control) {
	case UVC_VS_PROBE_CONTROL:
		//Ucamera_LOG("setting probe control, length = %d\n", data->length);
		target = &dev->probe;
		break;

	case UVC_VS_COMMIT_CONTROL:
		//Ucamera_LOG("setting commit control, length = %d\n", data->length);
		target = &dev->commit;
		break;

	case UVC_VS_STILL_PROBE_CONTROL:
		//Ucamera_LOG("setting still probe control, length = %d\n", data->length);
		target1 = &dev->still;
		break;
	case UVC_VS_STILL_IMAGE_TRIGGER_CONTROL:
		uvc_video_still_image_cap_triggle(dev);
		return 0;
	default:
		//Ucamera_LOG("setting unknown control, cs:%d length = %d\n", dev->control, data->length);

		/*
		 * As we support only BRIGHTNESS control, this request is
		 * for setting BRIGHTNESS control.
		 * Check for any invalid SET_CUR(BRIGHTNESS) requests
		 * from Host. Note that we support Brightness levels
		 * from 0x0 to 0x10 in a step of 0x1. So, any request
		 * with value greater than 0x10 is invalid.
		 */

			if (dev->puctl_cur > 0) {

				ret = uvc_events_process_control_data(dev,
							dev->puctl_cur,
							dev->unit_cur, data);
				if (ret < 0)
					goto err;
			}
			dev->puctl_cur = 0;
			dev->unit_cur = 0;

			return 0;
	}

	ctrl = (struct uvc_streaming_control *)&data->data;
	iformat = clamp((unsigned int)ctrl->bFormatIndex, 1U, g_nFormats);
	format = uvc_formats[iformat-1];
	nframes = 0;
	while (format->frames[nframes].width != 0)
		++nframes;

	iframe = clamp((unsigned int)ctrl->bFrameIndex, 1U, nframes);
	frame = &format->frames[iframe-1];
	interval = frame->intervals;
	//Ucamera_LOG("UVC: set format:%d, frame:%d\n", iformat, iframe);
	while (interval[0] < ctrl->dwFrameInterval && interval[1])
		++interval;
	if (dev->control != UVC_VS_STILL_PROBE_CONTROL) {
		target->bFormatIndex = iformat;
		target->bFrameIndex = iframe;
	} else {
		target1->bFormatIndex = 1;
		target1->bFrameIndex = 1;
	}
	switch (format->fcc) {
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_NV12:
		if (dev->control != UVC_VS_STILL_PROBE_CONTROL)
			target->dwMaxVideoFrameSize = frame->width * frame->height * 2;
		else
			target1->dwMaxVideoFrameSize = frame->width * frame->height * 2;
		break;
	case V4L2_PIX_FMT_MJPEG:
	case V4L2_PIX_FMT_H264:
		if (dev->imgsize == 0)
			Ucamera_LOG("WARNING: MJPEG/h.264 requested and no image loaded.\n");
		if (dev->control != UVC_VS_STILL_PROBE_CONTROL)
			target->dwMaxVideoFrameSize = dev->imgsize;
		else
			target1->dwMaxVideoFrameSize = dev->imgsize;
		break;
	}
	if (dev->control != UVC_VS_STILL_PROBE_CONTROL)
		target->dwFrameInterval = *interval;

	if (dev->control == UVC_VS_COMMIT_CONTROL) {
		//system("sync");
		//system("echo 3 > /proc/sys/vm/drop_caches");
		dev->fcc = format->fcc;
		dev->width = frame->width;
		dev->height = frame->height;
		uvc_video_set_format(dev);
		if (dev->bulk) {
			ret = uvc_handle_streamon_event(dev);
			if (ret < 0)
				goto err;
		}

	}
	return 0;

err:
	return ret;
}
static int streaming_flag = 0;
struct Ucamera_Video_Frame u_frame;
static void
uvc_events_process(struct uvc_device *dev)
{
	struct v4l2_event v4l2_event;
	struct uvc_event *uvc_event = (void *)&v4l2_event.u.data;
	struct uvc_request_data resp;
	int ret;

	ret = ioctl(dev->uvc_fd, VIDIOC_DQEVENT, &v4l2_event);
	if (ret < 0) {
		Ucamera_LOG("UVC: VIDIOC_DQEVENT failed: %s (%d)\n", strerror(errno),
			errno);
		return;
	}

	memset(&resp, 0, sizeof resp);
	resp.length = -EL2HLT;

	switch (v4l2_event.type) {
	case UVC_EVENT_CONNECT:
		return;

	case UVC_EVENT_DISCONNECT:
		dev->uvc_shutdown_requested = 1;
		Ucamera_LOG("UVC: Possible USB shutdown requested from "
				"Host, seen via UVC_EVENT_DISCONNECT\n");
		return;

	case UVC_EVENT_SETUP:
		uvc_events_process_setup(dev, &uvc_event->req, &resp);
		break;

	case UVC_EVENT_DATA:
		ret = uvc_events_process_data(dev, &uvc_event->data);
		if (ret < 0)
			break;

		return;

	case UVC_EVENT_STREAMON: {
		Ucamera_LOG("UVC: uvc event stream on!\n");
		if(streaming_flag == 1){
			uvc_video_stream(dev, 0);
			uvc_uninit_device(dev);
			uvc_video_reqbufs(dev, 0);
			dev->is_streaming = 0;
			dev->first_buffer_queued = 0;
			dev->event_process(UCAMERA_EVENT_STREAMOFF, &u_frame);
			streaming_flag = 0;
			Ucamera_LOG("UVC:Device re insertion\n");
		}
		u_frame.fcc = dev->fcc;
		u_frame.width = dev->width;
		u_frame.height = dev->height;
		/*u_frame.intervals = dev->intervals;*/
		dev->event_process(UCAMERA_EVENT_STREAMON, &u_frame);
		if (!dev->bulk)
			uvc_handle_streamon_event(dev);
		streaming_flag = 1;
		return;
	}

	case UVC_EVENT_STREAMOFF: {
		Ucamera_LOG("UVC: uvc event stream off!\n");
		/* ... and now Stop UVC streaming.. */
		if (dev->is_streaming) {
			uvc_video_stream(dev, 0);
			uvc_uninit_device(dev);
			uvc_video_reqbufs(dev, 0);
			dev->is_streaming = 0;
			dev->first_buffer_queued = 0;
			/*struct Ucamera_Video_Frame u_frame;*/
			u_frame.fcc = dev->fcc;
			u_frame.width = dev->width;
			u_frame.height = dev->height;
			dev->event_process(UCAMERA_EVENT_STREAMOFF, &u_frame);
			streaming_flag = 0;
		}
		return;
		}
	}

	ret = ioctl(dev->uvc_fd, UVCIOC_SEND_RESPONSE, &resp);
	if (ret < 0) {
		Ucamera_LOG("UVC: UVCIOC_S_EVENT failed: %s (%d)\n", strerror(errno),
			errno);
		return;
	}
}

static void
uvc_events_init(struct uvc_device *dev)
{
	struct v4l2_event_subscription sub;
	unsigned int payload_size = 0;

	switch (dev->fcc) {
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_NV12:
		payload_size = dev->width * dev->height * 2;
		break;
	case V4L2_PIX_FMT_MJPEG:
	case V4L2_PIX_FMT_H264:
		payload_size = dev->imgsize;
		break;
	}

	uvc_fill_streaming_control(dev, &dev->probe, 0, 0);
	uvc_fill_streaming_control(dev, &dev->commit, 0, 0);
	uvc_fill_streaming_still_control(dev, &dev->still, 0, 0);

	if (dev->bulk)
		/* FIXME Crude hack, must be negotiated with the driver. */
		dev->probe.dwMaxPayloadTransferSize =
			dev->commit.dwMaxPayloadTransferSize = payload_size;

	memset(&sub, 0, sizeof sub);
	sub.type = UVC_EVENT_SETUP;
	ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
	sub.type = UVC_EVENT_DATA;
	ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
	sub.type = UVC_EVENT_STREAMON;
	ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
	sub.type = UVC_EVENT_STREAMOFF;
	ioctl(dev->uvc_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
}

int Ucamera_Audio_Get_Frame(struct Ucamera_Audio_Frame *frame)
{
	char *addr = NULL;
	struct uvc_device *udev = g_udev;
#if 0
	int len = read(udev->uac_fd, udev->audio_buf, AUDIO_OUT_BUF_SIZE);
	if (len < 0)
		return -1;
	else if (len == 0){
		frame->len = 0;
		return 0;
	}
	frame->data = udev->audio_buf;
	if (len > AUDIO_OUT_BUF_SIZE)
		len = AUDIO_OUT_BUF_SIZE;
	frame->len = len;
#endif
	int len = ioctl(udev->uac_fd, GAUDIO_IOCTL_DEQUEUE_BUF, &udev->fifo_info);
	if (len <= 0) {
		frame->len = 0;
		return 0;
	}
	addr = udev->audio_base_addr_mmap + (udev->fifo_info.data -udev->audio_base_addr_phy);
	frame->len = udev->fifo_info.size;
	frame->data = addr;
	return 0;
}

int Ucamera_Audio_Release_Frame(struct Ucamera_Audio_Frame *frame)
{
	int ret;
	struct uvc_device *udev = g_udev;
	ret = ioctl(udev->uac_fd, GAUDIO_IOCTL_QUEUE_BUF, &udev->fifo_info);
	if (ret) {
		Ucamera_LOG("[ERROR]audio release freame failed\n");
		return ret;
	}
	return 0;
}


static int cmk_audio_en = 1;
static void
uvc_audio_events_process(struct uvc_device *dev)
{
	int ret;
	struct gaudio_event event;
	struct Ucamera_Audio_CB_Func *a_func = dev->a_func;
	static int spk_vol = 0;
	static int mic_vol = 0;

	ret = ioctl(dev->uac_fd, GAUDIO_IOCTL_GET_EVENT, &event);
	if (ret < 0) {
		Ucamera_LOG("gaudio ioctl failed\n");
		return;
	}

	//Ucamera_LOG("Audio Event type:0x%02x, Value:0x%04x\n", event.type, event.value);
	switch (event.type) {
	case UAC_EVENT_SET_SPK_MUTE:
		a_func->set_Spk_Mute(event.value);
		break;
	case UAC_EVENT_SET_SPK_VOLUME:
		//spk_vol = aduio_spk_volume_vert(event.value);
		spk_vol = event.value;
		a_func->set_Spk_Volume(spk_vol);
		break;
	case UAC_EVENT_SET_MIC_MUTE:
		a_func->set_Mic_Mute(event.value);
		break;
	case UAC_EVENT_SET_MIC_VOLUME:
		mic_vol = event.value;
		//mic_vol = aduio_mic_volume_vert(event.value);
		a_func->set_Mic_Volume(mic_vol);
		break;
	case UAC_EVENT_RECORD_OFF:
		cmk_audio_en = 0;
		break;
	case UAC_EVENT_RECORD_ON:
		cmk_audio_en = 1;
		break;
	case UAC_EVENT_SPEAK_OFF:
	case UAC_EVENT_SPEAK_ON:
		break;
	default:
		Ucamera_LOG("gaudio unkonwn event:%d\n", event.type);
	}

	return;
}

static void *usb_audio_event_process_thread_entry(void *argv)
{
	int retval, fd = 0;
	struct uvc_device *udev = (struct uvc_device *) argv;

	fd_set rfds;
	fd = udev->uac_fd;

	prctl(PR_SET_NAME, "audio_routine");

	while(1) {


		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		retval = select(fd + 1, &rfds, NULL, NULL, NULL);
		if (retval == -1 && errno == EINTR)
			continue;
		if (retval < 0) {
			perror("select()");
			return NULL;
		}
		if (FD_ISSET(fd, &rfds)) {
			uvc_audio_events_process(udev);
		}
	}
	return NULL;
}

static void *usb_audio_recording_thread_entry(void *argv)
{
	int len = 0;
	int fd = 0;
	short pcm_buf[AUDIO_PCM_SIZE] = {0};
	struct uvc_device *udev = (struct uvc_device *) argv;
	struct Ucamera_Audio_CB_Func *a_func = udev->a_func;

	fd = udev->uac_fd;

	prctl(PR_SET_NAME, "audio_recording");

	while(1) {
		if (cmk_audio_en == 1) { 
			len = a_func->get_AudioPcm(pcm_buf);
			if (len <= 0) {
				usleep(50*1000); /*sleep 50 ms*/
				continue;
			}
			ssize_t size = write(fd, pcm_buf, len);
			if (len != size) {
				if (0 != size)
					Ucamera_LOG("[ERROR]: gaudio write pcm(%d) failed,  actual(%d)\n", len, size);
			}
		}
		else {
			usleep(1000);
		}

    }
	return NULL;
}

int gaudio_play_fifo_init(int fd)
{
	int ret = 0;
	void *addr = NULL;
	struct uvc_device *udev = g_udev;

	gaudio_fifo_list_info_t fifo_info = {0};

	ret = ioctl(fd, GAUDIO_IOCTL_GET_MMAP_INFO, &fifo_info);
	if (ret < 0) {
		Ucamera_LOG("[ERROR]gaudio ioctl get mmap info failed\n");
		return -1;
	}
	if (fifo_info.cnt <= 0 || fifo_info.size > AUDIO_OUT_BUF_SIZE) {
		Ucamera_LOG("[ERROR]gaudio get mmap info failed!\n");
		return -1;
	}
	udev->audio_base_addr_phy = fifo_info.base;

	addr = (void *)mmap(NULL, fifo_info.size*fifo_info.cnt, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)fifo_info.base);
	if (addr == MAP_FAILED) {
		Ucamera_LOG("[ERROR]: gaudio mmap failed! error:%s\n", strerror(errno));
		return -1;
	}
	udev->audio_base_addr_mmap = addr;

	return ret;
}

int Ucamera_Audio_Start(void)
{
	int ret = -1;
	pthread_t audio_thread_id;
	struct uvc_device *udev = g_udev;

	int fd = open(UCAMERA_AUDIO_DEV, O_RDWR);
	if (fd <= 0) {
		Ucamera_LOG("[ERROR]: audio dev open %s", strerror(errno));
		return NULL;
	}
	udev->uac_fd = fd;
#if 0
	udev->audio_buf = malloc(AUDIO_OUT_BUF_SIZE);
	if (udev->audio_buf == NULL) {
		Ucamera_LOG("[ERROR] %s alloc pcm buf failed\n", __func__);
		return NULL;
	}
#endif

	if (g_ucfg->acfg.speak_enable) {
		if (gaudio_play_fifo_init(fd)) {
			Ucamera_LOG("[ERROR] audio init play fifo failed!\n");
		}
	}
	Ucamera_LOG("[INFO]: Audio Start.\n");


	ret = pthread_create(&audio_thread_id, NULL, usb_audio_recording_thread_entry, udev);
	if(ret != 0) {
		Ucamera_LOG("[ERROR] %s: pthread_create Audio thread failed\n", __func__);
		return -1;
	}

	pthread_t audio_ep_thread_id;
	ret = pthread_create(&audio_ep_thread_id, NULL, usb_audio_event_process_thread_entry, udev);
	if(ret != 0) {
		Ucamera_LOG("[ERROR] %s: pthread_create Audio thread failed\n", __func__);
		return -1;
	}

	return 0;
}


int ucamera_device_init(struct Ucamera_Cfg *ucfg)
{
	int ret = 0;
	int ucamera_fd = -1;


	ucamera_fd = open(UCAMERA_DEV, O_RDWR);
	if (ucamera_fd < 0) {
		Ucamera_LOG("error(%s %d):open ucamera device failed %d\n", __func__, __LINE__, ucamera_fd);
		return -1;
	}
	if (ucfg->stillcap == 1) {
		ret = ioctl(ucamera_fd, UCAMERA_IOCTL_STILL_IMG_CAP_EN, NULL);
		if (ret < 0) {
			Ucamera_LOG("error(%s,%d): ucamera ioctl failed\n", __func__, __LINE__);
			goto ucam_device_init_exit;
		}
	}
	ret = ioctl(ucamera_fd, UCAMERA_IOCTL_FORMAT_CFG, uvc_formats);
	if (ret < 0) {
		Ucamera_LOG("error(%s,%d): ucamera ioctl failed\n", __func__, __LINE__);
		goto ucam_device_init_exit;
	}

	ret = ioctl(ucamera_fd, UCAMERA_IOCTL_PRODUCT_INFO_CFG, &ucfg->pcfg);
	if (ret < 0) {
		Ucamera_LOG("error(%s,%d): ucamera ioctl failed\n", __func__, __LINE__);
		goto ucam_device_init_exit;
	}

	if (ucfg->audio_en == 1) {
		ret = ioctl(ucamera_fd, UCAMERA_IOCTL_AUDIO_ENABLE, NULL);
		if (ret < 0) {
			Ucamera_LOG("error(%s,%d): ucamera ioctl failed\n", __func__, __LINE__);
			goto ucam_device_init_exit;
		}
		ret = ioctl(ucamera_fd, UCAMERA_IOCTL_AUDIO_CFG, &ucfg->acfg);
		if (ret < 0) {
			Ucamera_LOG("error(%s,%d): ucamera ioctl failed\n", __func__, __LINE__);
			goto ucam_device_init_exit;
		}
	}
	if (ucfg->adb_en == 1) {
		ret = ioctl(ucamera_fd, UCAMERA_IOCTL_ADB_ENABLE, NULL);
		if (ret < 0) {
			Ucamera_LOG("error(%s,%d): ucamera ioctl failed\n", __func__, __LINE__);
			goto ucam_device_init_exit;
		}
	}
	if (ucfg->hid_en == 1) {
		ret = ioctl(ucamera_fd, UCAMERA_IOCTL_HID_ENABLE, NULL);
		if (ret < 0) {
			Ucamera_LOG("error(%s,%d): ucamera ioctl failed\n", __func__, __LINE__);
			goto ucam_device_init_exit;
		}
	}
	ret = ioctl(ucamera_fd, UCAMERA_IOCTL_START, NULL);
	if (ret < 0) {
		Ucamera_LOG("error(%s,%d): ucamera ioctl dev enable failed\n", __func__, __LINE__);
		goto ucam_device_init_exit;
	}

ucam_device_init_exit:
	close(ucamera_fd);

	return 0;
}

static void *ucamera_routines_process_thread(void *argv)
{
	int ret, len, i;
	int fd, uvc_fd, hid_fd = 0;
	struct timeval tv;
	fd_set fdsu, rfds;
	char hid_buf[512] = {0};
	struct uvc_device *udev;

	udev = (struct uvc_device *) argv;
	uvc_fd = udev->uvc_fd;
	if (udev->hid_fd > 0)
		hid_fd = udev->hid_fd;
	fd = uvc_fd;
	if (fd < hid_fd)
		fd = hid_fd;

	prctl(PR_SET_NAME, "video_routine");

	while (1) {

		if(udev->is_streaming == 0)
			usleep(5000);

		FD_ZERO(&fdsu);
		FD_ZERO(&rfds);

		/* We want both setup and data events on UVC interface.. */
		FD_SET(uvc_fd, &fdsu);
		if (hid_fd)
			FD_SET(hid_fd, &rfds);

		fd_set efds = fdsu;
		fd_set dfds = fdsu;

		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		ret = select(fd + 1, &rfds,
					&dfds, &efds, NULL);

		if (-1 == ret) {
			Ucamera_LOG("select error %d, %s\n",
					errno, strerror (errno));
			if (EINTR == errno)
				continue;

			break;
		}

		if (0 == ret) {
			Ucamera_LOG("select timeout\n");
			break;
		}

		if (FD_ISSET(uvc_fd, &efds))
			uvc_events_process(udev);
		if (FD_ISSET(uvc_fd, &dfds))
			uvc_video_process(udev);
		if (hid_fd) {
			if (FD_ISSET(hid_fd, &rfds)) {
				len = read(fd, hid_buf, 512 - 1);
				Ucamera_LOG("Hid report:");
				for (i = 0; i < len; i++)
					printf(" 0x%02x", hid_buf[i]);
				printf("\n");
			}
		}

	}
	return NULL;
}

int Ucamera_Audio_Regesit_CB(struct Ucamera_Audio_CB_Func *a_func)
{
	struct uvc_device *udev = g_udev;

	if (!a_func)
		return -1;
	if (!a_func->get_AudioPcm || !a_func->set_Spk_Volume || !a_func->set_Mic_Volume) {
		Ucamera_LOG("[ERROR]%s regesit failed\n", __func__);
		return -1;
	}
	udev->a_func = a_func;
	return 0;
}

int UCamera_Registe_Event_Process_CB(int (*event_process)(int , void *))
{
	int ret = 0;
	struct uvc_device *udev = g_udev;
	if (!event_process)
		return -1;
	udev->event_process = event_process;
	return ret;
}

int Ucamera_Init(int FrmNum, int FrmMaxSize)
{
	int i, ret;
	struct uvc_device *udev = NULL;
	struct Ucamera_Video_PU_Control *puctl = NULL;
	struct Ucamera_Video_EU_Control *euctl = NULL;
	struct Ucamera_Video_CT_Control *ctctl = NULL;

	/* Open the UVC device. */
	ret = uvc_open(&g_udev, UCAMERA_VIDEO_DEV);
	if (g_udev == NULL || ret < 0)
		return 0;
	udev = g_udev;

	udev->uvc_devname = UCAMERA_VIDEO_DEV;

	/* Set parameters as passed by user. */
	udev->width =  UVC_DEFAULT_WIDTH;
	udev->height =  UVC_DEFAULT_HEIGHT;
	udev->imgsize = udev->width * udev->height;

	udev->fcc = V4L2_PIX_FMT_H264;
	udev->io = IO_METHOD_MMAP;
	udev->bulk = 0;
	udev->nbufs = 3;
	udev->mult = 0;
	udev->burst = 0;
	udev->speed = USB_SPEED_SUPER;
	udev->run_standalone = 1;
	udev->maxpkt = 1024;
	udev->uvc_event = 0;
	udev->uac_fd = -1;
	udev->hid_fd = -1;
	udev->puctl_cur = 0;
	udev->unit_cur = 0;

	if (FrmNum <= 0 && FrmNum > 20) {
		Ucamera_LOG("Invalid FrmNum valued:%d\n", FrmNum);
		return 0;
	}
	if (FrmMaxSize <= 0) {
		Ucamera_LOG("Invalid FrmSize valued:%d\n", FrmMaxSize);
		return 0;
	}
	udev->nbufs = FrmNum;
	udev->imgsize = FrmMaxSize;

	puctl = udev->v_puctl;
	for (i = 0; i < 12; i++) {
		puctl[i].cmd = 0;
		puctl[i].type = 0;
		puctl[i].get = NULL;
		puctl[i].set = NULL;
		memset(puctl[i].data, 0, 8*sizeof(int));
	}
	euctl = udev->v_euctl;
	for (i = 0; i < 12; i++) {
		euctl[i].type = 0;
		euctl[i].len = 0;
		euctl[i].get = NULL;
		euctl[i].set = NULL;
		//memset(puctl[i].data, 0, 8*sizeof(int));
	}
	ctctl = udev->v_ctctl;
	for (i = 0; i < 8; i++) {
		puctl[i].cmd = 0;
		ctctl[i].type = 0;
		ctctl[i].get = NULL;
		ctctl[i].set = NULL;
		memset(ctctl[i].data, 0, 8*sizeof(int));
	}
	/* Init UVC events. */
	uvc_events_init(udev);
	return 0;
}

int Ucamera_DeInit(void)
{
	struct uvc_device *udev = g_udev;

	if (udev->is_streaming) {
		/* ... and now UVC streaming.. */
		uvc_video_stream(udev, 0);
		uvc_uninit_device(udev);
		uvc_video_reqbufs(udev, 0);
		udev->is_streaming = 0;
	}

	uvc_close(udev);
	return 0;
}

int UCamera_Video_Start(void)
{
	pthread_t tid; /* Stream capture in another thread */
	int ret;
	struct uvc_device *udev = g_udev;

	Ucamera_LOG("[INFO]: Video Start.\n");
	ret = pthread_create(&tid, NULL, ucamera_routines_process_thread, (void *)udev);
	if (ret) {
		Ucamera_LOG("[ERROR]%s ucamra event thread create error\n", __func__);
		return -1;
	}
	return 0;
}

int UCamera_Regesit_SDK_Init_CB()
{
	return 0;
}

int Ucamera_Video_Regesit_CB(struct Ucamera_Video_CB_Func *v_func)
{
	struct uvc_device *udev = g_udev;

	if (!v_func)
		return -1;
	if (!v_func->get_H264Frame || !v_func->get_JpegFrame || !v_func->get_YuvFrame) {
		Ucamera_LOG("[ERROR]%s regesit failed\n", __func__);
		return -1;
	}
	udev->v_func = v_func;
	return 0;
}


#if 0
 else if (strcmp(key, "nframes") == 0) {
			Ucamera_LOG("%s %s\n", key, value);
			nframes = strToInt(value);

			i = 0;
			uvc_formats = malloc(g_nFormats*sizeof(struct uvc_format_info *));
			frame_buf = malloc((2*nframes + 2)*sizeof(struct uvc_frame_info));
			memset(frame_buf, 0, (2*nframes + 2)*sizeof(struct uvc_frame_info));

			Ucamera_LOG("frames:\n");
			while (i < nframes) {
				if (fscanf(fp, "%[^\n]", line_str) < 0)
					break;
				sscanf(line_str, "{%d, %d}", &width, &height);
				frames_mjpeg = (struct uvc_frame_info *)frame_buf + i;
				frames_h264 = (struct uvc_frame_info *)frame_buf + i + nframes + 1;
				if (width > 0 && height > 0 && (width%16 == 0)) {
					Ucamera_LOG("%d %d\n", width, height);

					frames_mjpeg->width = width;
					frames_mjpeg->height = height;
					frames_mjpeg->intervals[0] = intervals;
					frames_h264->width = width;
					frames_h264->height = height;
					frames_h264->intervals[0] = intervals;
				} else {
					Ucamera_LOG("error(%s %d)Invalid width or height(%d %d)\n", __func__, __LINE__, width, height);
				}
				i++;
				fseek(fp , 1, SEEK_CUR);
			}
			uvc_formats_mjpeg.frames = (struct uvc_frame_info *)frame_buf;
			uvc_formats_h264.frames = (struct uvc_frame_info *)frame_buf + nframes + 1;
			uvc_formats_h264.nframes = nframes;
			uvc_formats_mjpeg.nframes = nframes;

			uvc_formats[0] = &uvc_formats_yuyv;
			uvc_formats[1] = &uvc_formats_mjpeg;
			uvc_formats[2] = &uvc_formats_h264;
		}
#endif
int Ucamera_Config(struct Ucamera_Cfg *ucfg)
{
	int nframes, i, cnt;
	struct uvc_frame_info *frame;
	struct Ucamera_Video_Cfg *vcfg;
	int intervals = 10000000/30;

	vcfg = &(ucfg->vcfg);
	nframes = vcfg->h264num + vcfg->jpegnum + vcfg->yuyvnum;
	uvc_formats = malloc(3*sizeof(struct uvc_format_info *));
	frame = malloc((nframes + 3) *sizeof(struct uvc_frame_info));
	memset(frame, 0, (nframes + 3) *sizeof(struct uvc_frame_info));

	i = 0;
	cnt = 0;

	uvc_formats_yuyv.frames = &frame[i];
	uvc_formats_yuyv.nframes = vcfg->yuyvnum;


	while (i < vcfg->yuyvnum) {
		frame[cnt].width = vcfg->yuyvlist[i].width;
		frame[cnt].height = vcfg->yuyvlist[i].height;
		if (frame[cnt].width >= 1920)
			frame[cnt].intervals[0] = intervals*6;
		else if (frame[cnt].width >= 800)
			frame[cnt].intervals[0] = intervals*3;
		else
			frame[cnt].intervals[0] = intervals;
		//Ucamera_LOG("yuv %d W:%d H:%d\n", cnt, frame[i].width, frame[i].height);
		cnt++;
		i++;
	}

	cnt++;

	uvc_formats_mjpeg.frames = &frame[cnt];
	uvc_formats_mjpeg.nframes = vcfg->jpegnum;
	i = 0;
	while (i < vcfg->jpegnum) {
		frame[cnt].width = vcfg->jpeglist[i].width;
		frame[cnt].height = vcfg->jpeglist[i].height;
		frame[cnt].intervals[0] = intervals;
		//Ucamera_LOG("jpeg %d W:%d H:%d\n", cnt, frame[i].width, frame[i].height);
		i++;
		cnt++;
	}
	cnt++;

	uvc_formats_h264.frames = &frame[cnt];
	if (ucfg->h264_en == 1) {
		uvc_formats_h264.nframes = vcfg->h264num;
		g_nFormats = 3;
	} else
		uvc_formats_h264.nframes = 0;

	i = 0;
	while (i < vcfg->h264num) {
		frame[cnt].width = vcfg->h264list[i].width;
		frame[cnt].height = vcfg->h264list[i].height;
		frame[cnt].intervals[0] = intervals;
		//Ucamera_LOG("h264 %d W:%d H:%d\n", cnt, frame[i].width, frame[i].height);
		i++;
		cnt++;
	}

	uvc_formats[1] = &uvc_formats_yuyv;
	uvc_formats[0] = &uvc_formats_mjpeg;
	uvc_formats[2] = &uvc_formats_h264;

	if (ucamera_device_init(ucfg) != 0) {
		Ucamera_LOG("ucamera init failed!\n");
		return 0;
	}

	g_ucfg = ucfg;

	return 0;
}

int Ucamera_Audio_Config(struct Ucamera_Audio_Cfg *acfg)
{
	return 0;
}

int Ucamera_Video_Regesit_Camera_Terminal_CB(struct Ucamera_Video_CT_Control *ctctl[])
{
	int i , type, ret = 0;
	int nformat = 0;
	struct Ucamera_Video_CT_Control *ictl, *u_ictl;
	struct uvc_device *udev = g_udev;

	u_ictl = udev->v_ctctl;
	nformat = ARRAY_SIZE(ctctl);
	for (i = 0; (ctctl[i] != NULL) && (i < UVC_FOCUS_AUTO_CONTROL); i++) {
		ictl = ctctl[i];
		type = ictl->type;
		switch (type)
		{
        case UVC_ZOOM_ABSOLUTE_CONTROL:
        case UVC_EXPOSURE_TIME_CONTROL:
        case UVC_AUTO_EXPOSURE_MODE_CONTROL:
        case UVC_FOCUS_CONTROL:
        case UVC_FOCUS_AUTO_CONTROL:
            u_ictl[type-1].data[UVC_MIN] = ictl->data[UVC_MIN];
            u_ictl[type-1].data[UVC_MAX] = ictl->data[UVC_MAX];
            u_ictl[type-1].data[UVC_DEF] = ictl->data[UVC_DEF];
            u_ictl[type-1].data[UVC_CUR] = ictl->data[UVC_CUR];
			break;
		default:
			Ucamera_LOG("[ERROR] Unknown input unit type:%d\n", ictl->type);
			ret = -1;
			break;
		}
		if (ret)
			continue;

		u_ictl[type-1].type = ictl->type;
		u_ictl[type-1].get = ictl->get;
		u_ictl[type-1].set = ictl->set;
	}

	return 0;

}

int Ucamera_Video_Regesit_Process_Unit_CB(struct Ucamera_Video_PU_Control *puctl[])
{
	int i , type, ret = 0;
	int nformat = 0;
	struct Ucamera_Video_PU_Control *ictl, *u_ictl;
	struct uvc_device *udev = g_udev;

	u_ictl = udev->v_puctl;
	nformat = ARRAY_SIZE(puctl);
	for (i = 0; (puctl[i] != NULL) && (i < UVC_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL); i++) {
		ictl = puctl[i];
		type = ictl->type;
		switch (type)
		{
		case UVC_BACKLIGHT_COMPENSATION_CONTROL:
		case UVC_BRIGHTNESS_CONTROL:
		case UVC_CONTRAST_CONTROL:
		case UVC_SATURATION_CONTROL:
		case UVC_SHARPNESS_CONTROL:
		case UVC_GAIN_CONTROL:
		case UVC_POWER_LINE_FREQUENCY_CONTROL:
		case UVC_HUE_CONTROL:
		case UVC_GAMMA_CONTROL:
		case UVC_WHITE_BALANCE_TEMPERATURE_CONTROL:
			u_ictl[type-1].data[UVC_MIN] = ictl->data[UVC_MIN];
			u_ictl[type-1].data[UVC_MAX] = ictl->data[UVC_MAX];
			u_ictl[type-1].data[UVC_DEF] = ictl->data[UVC_DEF];
			break;
		default:
			Ucamera_LOG("[ERROR] Unknown process unit type:%d\n", ictl->type);
			ret = -1;
			break;
		}
		if (ret)
			continue;

		u_ictl[type-1].type = ictl->type;
		u_ictl[type-1].get = ictl->get;
		u_ictl[type-1].set = ictl->set;
	}

	return 0;

}

int Ucamera_Video_Regesit_Extension_Unit_CB(struct Ucamera_Video_EU_Control *euctl[])
{
	int i , type, known = 0;
	int nformat = 0;
	struct Ucamera_Video_EU_Control *ictl, *u_ictl;
	struct uvc_device *udev = g_udev;

	u_ictl = udev->v_euctl;
	nformat = ARRAY_SIZE(euctl);
	for (i = 0; (euctl[i] != NULL) && (i < UVC_EU_CMD_USR8); i++) {
		ictl = euctl[i];
		type = ictl->type;
		switch (type)
		{
		case UVC_EU_CMD_USR1:
		case UVC_EU_CMD_USR2:
		case UVC_EU_CMD_USR3:
		case UVC_EU_CMD_USR4:
		case UVC_EU_CMD_USR5:
		case UVC_EU_CMD_USR6:
		case UVC_EU_CMD_USR7:
		case UVC_EU_CMD_USR8:
			known = 1;
			break;
		default:
			Ucamera_LOG("[ERROR] Unknown extension unit type:%d\n", ictl->type);
			known = 0;
			break;
		}
		if (!known)
			continue;

		u_ictl[type-1].type = ictl->type;
		if (ictl->get != NULL)
			u_ictl[type-1].get = ictl->get;
		if (ictl->set != NULL)
			u_ictl[type-1].set = ictl->set;
		u_ictl[type-1].len = ictl->len;
	}

	return 0;

}
int Ucamera_Hid_Init(void)
{
	struct uvc_device *udev = g_udev;

	if ((udev->hid_fd = open(UCAMERA_HID_DEV, O_RDWR, 0666)) == -1) {
		Ucamera_LOG("[ERROR] Ucamera open Hid devices failed!\n");
		udev->hid_fd = -1;
		return -1;
	}
	return 0;
}
