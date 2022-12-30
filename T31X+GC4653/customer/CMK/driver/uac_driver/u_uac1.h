/*
 * u_uac1.h -- interface to USB gadget "ALSA AUDIO" utilities
 *
 * Copyright (C) 2008 Bryan Wu <cooloney@kernel.org>
 * Copyright (C) 2008 Analog Devices, Inc
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __U_AUDIO_H
#define __U_AUDIO_H

#include <linux/device.h>
#include <linux/err.h>
#include <linux/usb/audio.h>
#include <linux/usb/composite.h>
#include <linux/module.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <linux/poll.h>
#include <linux/wait.h>

#include "gadget_chips.h"

#include "utils/node_list.h"



#ifndef DEBUG
#define DEBUG
#endif

#define CONFIG_GADGET_UAC1_CAP
#define CONFIG_GADGET_UAC1_CAP_USER
#define CONFIG_GADGET_UAC1_PLAY

#define GAUDIO_IOCTL_MAGIC  	'A'
#define GAUDIO_IOCTL_GET_EVENT			_IO(GAUDIO_IOCTL_MAGIC, 1)
#define GAUDIO_IOCTL_GET_MMAP_INFO		_IO(GAUDIO_IOCTL_MAGIC, 2)
#define GAUDIO_IOCTL_SET_MMAP_INFO		_IO(GAUDIO_IOCTL_MAGIC, 3)
#define GAUDIO_IOCTL_GET_MMAP_ADDR		_IO(GAUDIO_IOCTL_MAGIC, 4)
#define GAUDIO_IOCTL_DEQUEUE_BUF		_IO(GAUDIO_IOCTL_MAGIC, 5)
#define GAUDIO_IOCTL_QUEUE_BUF			_IO(GAUDIO_IOCTL_MAGIC, 6)

#define UAC_EVENT_SET_SPK_MUTE		1
#define UAC_EVENT_SET_SPK_VOLUME	2
#define UAC_EVENT_SET_MIC_MUTE		3
#define UAC_EVENT_SET_MIC_VOLUME	4

#define UAC_EVENT_SPEAK_ON		5
#define UAC_EVENT_SPEAK_OFF		6
#define UAC_EVENT_RECORD_ON		7
#define UAC_EVENT_RECORD_OFF		8


#define MIC_CH		    1
#define SPK_SATE	    16000
#define SPK_CH		    1

/* Set the parameters of the audio */
struct audio_param {
    int mic_sample_rate;
    int spk_samle_rate;
    short mic_volume;
    short spk_volume;
    int speak_en;
};

/*
 * This represents the USB side of an audio card device, managed by a USB
 * function which provides control and stream interfaces.
 */

struct gaudio_snd_dev {
	struct gaudio			*card;
	struct file			*filp;
	struct snd_pcm_substream	*substream;
	int				access;
	int				format;
	int				channels;
	int				rate;
};

struct gaudio {
	struct usb_function		func;
	struct usb_gadget		*gadget;

	/* ALSA sound device interfaces */
	struct gaudio_snd_dev		control;
	struct gaudio_snd_dev		playback;
	struct gaudio_snd_dev		capture;

	/* TODO */
};

struct f_audio_buf {
	u8 *buf;
	int actual;
	struct list_head list;
	u8 *node;
};

struct f_audio_event_list {
	struct usb_audio_control *event_con;
	int event;
	struct list_head list;
};

struct f_audio {
	struct gaudio			card;

	/* endpoints handle full and/or high speeds */
	struct usb_ep			*out_ep;
	struct usb_ep			*in_ep;

	spinlock_t			lock;
	struct f_audio_buf *copy_buf;
	struct work_struct playback_work;
	node_list_t *play_frame_list;

	wait_queue_head_t		event_queue;
	struct list_head   		event_list;
	spinlock_t			event_lock;

#ifdef CONFIG_GADGET_UAC1_PLAY
	struct list_head play_queue;
	struct list_head out_reqs_list;
#endif

	int in_online;
	int out_online;

#ifdef CONFIG_GADGET_UAC1_CAP_MIC
	struct task_struct *capture_thread;
	struct list_head cb_list_free;
	struct list_head cb_list_queued;
	spinlock_t cb_list_lock;
#endif


#ifdef CONFIG_GADGET_UAC1_CAP_USER
	spinlock_t audio_list_lock;
	struct list_head idle_reqs;
	struct list_head audio_data_list;
	wait_queue_head_t online_wq;
	wait_queue_head_t write_wq;
	atomic_t open_excl;
	atomic_t write_excl;
	atomic_t read_excl;
#endif


	atomic_t  interface_conn;
	volatile u8  mic_disconn;

	wait_queue_head_t read_wq;
	struct usb_request * cur_req;

	/* Control Set command */
	struct list_head cs;
	u8 set_cmd;
	struct usb_audio_control *set_con;

};

struct gaudio_event {
	int type;
	int value;
	char data[8];
};

int gaudio_setup(struct f_audio *);
void gaudio_cleanup(void);

#endif /* __U_AUDIO_H */
