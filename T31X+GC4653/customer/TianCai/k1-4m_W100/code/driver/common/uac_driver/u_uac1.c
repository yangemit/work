/*
 * u_uac1.c -- ALSA audio utilities for Gadget stack
 *
 * Copyright (C) 2008 Bryan Wu <cooloney@kernel.org>
 * Copyright (C) 2008 Analog Devices, Inc
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/random.h>
#include <linux/syscalls.h>

#include "u_uac1.h"


static int MIC_SATE = 0;

/*
 * This component encapsulates the ALSA devices for USB audio gadget
 */

#define FILE_PCM_PLAYBACK	"/dev/snd/pcmC0D0p"
#define FILE_PCM_CAPTURE	"/dev/snd/pcmC0D0c"
#define FILE_CONTROL		"/dev/snd/controlC0"

static char *fn_play = FILE_PCM_PLAYBACK;
module_param(fn_play, charp, S_IRUGO);
MODULE_PARM_DESC(fn_play, "Playback PCM device file name");

static char *fn_cap = FILE_PCM_CAPTURE;
module_param(fn_cap, charp, S_IRUGO);
MODULE_PARM_DESC(fn_cap, "Capture PCM device file name");

static char *fn_cntl = FILE_CONTROL;
module_param(fn_cntl, charp, S_IRUGO);
MODULE_PARM_DESC(fn_cntl, "Control device file name");

/*-------------------------------------------------------------------------*/

/**
 * Some ALSA internal helper functions
 */
static int snd_interval_refine_set(struct snd_interval *i, unsigned int val)
{
	struct snd_interval t;
	t.empty = 0;
	t.min = t.max = val;
	t.openmin = t.openmax = 0;
	t.integer = 1;
	return snd_interval_refine(i, &t);
}

static int _snd_pcm_hw_param_set(struct snd_pcm_hw_params *params,
				 snd_pcm_hw_param_t var, unsigned int val,
				 int dir)
{
	int changed;
	if (hw_is_mask(var)) {
		struct snd_mask *m = hw_param_mask(params, var);
		if (val == 0 && dir < 0) {
			changed = -EINVAL;
			snd_mask_none(m);
		} else {
			if (dir > 0)
				val++;
			else if (dir < 0)
				val--;
			changed = snd_mask_refine_set(
					hw_param_mask(params, var), val);
		}
	} else if (hw_is_interval(var)) {
		struct snd_interval *i = hw_param_interval(params, var);
		if (val == 0 && dir < 0) {
			changed = -EINVAL;
			snd_interval_none(i);
		} else if (dir == 0)
			changed = snd_interval_refine_set(i, val);
		else {
			struct snd_interval t;
			t.openmin = 1;
			t.openmax = 1;
			t.empty = 0;
			t.integer = 0;
			if (dir < 0) {
				t.min = val - 1;
				t.max = val;
			} else {
				t.min = val;
				t.max = val+1;
			}
			changed = snd_interval_refine(i, &t);
		}
	} else
		return -EINVAL;
	if (changed) {
		params->cmask |= 1 << var;
		params->rmask |= 1 << var;
	}
	return changed;
}
/*-------------------------------------------------------------------------*/

/**
 * Set default hardware params
 */
static int playback_default_hw_params(struct gaudio_snd_dev *snd)
{
	return 0;
}
static int capture_default_hw_params(struct gaudio_snd_dev *snd)
{
	return 0;

}
/**
 * Playback audio buffer data by ALSA PCM device
 */
static size_t u_audio_playback(struct gaudio *card, void *buf, size_t count)
{
	return 0;
}

#ifdef CONFIG_GADGET_UAC1_CAP_MIC
static size_t u_audio_capture(struct gaudio *card, void *buf, size_t count)
{

	return 0;
}
#endif

static int u_audio_get_playback_channels(struct gaudio *card)
{
	return card->playback.channels;
}

static int u_audio_get_playback_rate(struct gaudio *card)
{
	return card->playback.rate;
}
static int u_audio_get_capture_channels(struct gaudio *card)
{
	return card->capture.channels;
}

static int u_audio_get_capture_rate(struct gaudio *card)
{
	return card->capture.rate;
}

/**
 * Open ALSA PCM and control device files
 * Initial the PCM or control device
 */
static int gaudio_open_snd_dev(struct gaudio *card)
{
	struct snd_pcm_file *pcm_file;
	struct gaudio_snd_dev *snd;
	int ret = 0;

	if (!card)
		return -ENODEV;

	/* Open control device */
	snd = &card->control;
	snd->filp = filp_open(fn_cntl, O_RDWR, 0);
	if (IS_ERR(snd->filp)) {
		ret = PTR_ERR(snd->filp);
		ERROR(card, "unable to open sound control device file: %s\n",
				fn_cntl);
		snd->filp = NULL;
		goto control_failed;
	}
	snd->card = card;

#ifdef CONFIG_GADGET_UAC1_PLAY
	/* Open PCM playback device and setup substream */
	snd = &card->playback;
	snd->filp = filp_open(fn_play, O_WRONLY, 0);
	if (IS_ERR(snd->filp)) {
		ret = PTR_ERR(snd->filp);
		ERROR(card, "No such PCM playback device: %s\n", fn_play);
		snd->filp = NULL;
		filp_close(card->control.filp, current->files);
		goto playback_failed;
	}
	pcm_file = snd->filp->private_data;
	snd->substream = pcm_file->substream;
	snd->card = card;
	playback_default_hw_params(snd);
#endif

	/* Open PCM capture device and setup substream */
#ifdef CONFIG_GADGET_UAC1_CAP
	snd = &card->capture;
	snd->filp = filp_open(fn_cap, O_RDONLY, 0);
	if (IS_ERR(snd->filp)) {
		ERROR(card, "No such PCM capture device: %s\n", fn_cap);
		snd->substream = NULL;
		snd->card = NULL;
		snd->filp = NULL;
		filp_close(card->playback.filp, current->files);
		filp_close(card->control.filp, current->files);
		goto capture_failed;
	} else {
		pcm_file = snd->filp->private_data;
		snd->substream = pcm_file->substream;
		snd->card = card;
		capture_default_hw_params(snd);
	}
#endif

	return 0;
capture_failed:
	card->playback.filp = NULL;
	card->playback.substream = NULL;
	card->playback.card = NULL;
	card->playback.channels = 0;
	card->playback.access = 0;
	card->playback.rate = 0;
	card->playback.format = 0;
playback_failed:
	card->control.card = NULL;
control_failed:
	return ret;
}

/**
 * Close ALSA PCM and control device files
 */
static int gaudio_close_snd_dev(struct gaudio *gau)
{
	struct gaudio_snd_dev	*snd;

	/* Close control device */
	snd = &gau->control;
	if (snd->filp)
		filp_close(snd->filp, NULL);

	/* Close PCM playback device and setup substream */
	snd = &gau->playback;
	if (snd->filp)
		filp_close(snd->filp, NULL);

	/* Close PCM capture device and setup substream */
	snd = &gau->capture;
	if (snd->filp)
		filp_close(snd->filp, NULL);

	return 0;
}

static struct gaudio *the_card;
static struct f_audio *audio_card;
/**
 * gaudio_setup - setup ALSA interface and preparing for USB transfer
 *
 * This sets up PCM, mixer or MIDI ALSA devices fore USB gadget using.
 *
 * Returns negative errno, or zero on success
 */
int gaudio_setup(struct f_audio * audio)
{
	int	ret;

	audio_card = audio;
	ret = gaudio_open_snd_dev(&audio->card);
	if (ret)
	{
		ERROR(&audio->card, "we need at least one control device\n");
		the_card = NULL;
	}
	else if (!the_card)
		the_card = &audio->card;

	return ret;

}

/**
 * gaudio_cleanup - remove ALSA device interface
 *
 * This is called to free all resources allocated by @gaudio_setup().
 */
void gaudio_cleanup(void)
{
	if (the_card) {
		gaudio_close_snd_dev(the_card);
		the_card = NULL;
	}

	kfree(audio_card);

}

/* int set_uac_param(void) */
/* { */
/*     int ret = 0; */
/*     MIC_SATE = audio_param->mic_sampling_rate; */
/*     return 0; */
/* } */
