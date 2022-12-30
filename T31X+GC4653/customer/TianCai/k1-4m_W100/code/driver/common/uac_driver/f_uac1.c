/*
 * f_audio.c -- USB Audio class function driver
  *
 * Copyright (C) 2008 Bryan Wu <cooloney@kernel.org>
 * Copyright (C) 2008 Analog Devices, Inc
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/atomic.h>
#include <linux/kthread.h>
#include <linux/sched/rt.h>
#include <linux/interrupt.h>

#include "u_uac1.h"

#define IN_EP_REQ_COUNT 		(MIC_CH * 25)
#define IN_EP_MAX_PACKET_SIZE	384//IN_EP_ACTUAL_SIZE

#define AUDIO_NODE_NUM		40
static int IN_SAMPLE_RATE = 16000;
static int IN_EP_ACTUAL_SIZE = 16000 * 1 * 2 / 1000;

static int OUT_SAMPLE_RATE = 48000;

#define OUT_EP_MAX_PACKET_SIZE	    0x200
static int req_buf_size = OUT_EP_MAX_PACKET_SIZE;
module_param(req_buf_size, int, S_IRUGO);
MODULE_PARM_DESC(req_buf_size, "ISO OUT endpoint request buffer size");

static int req_count = 256;
module_param(req_count, int, S_IRUGO);
MODULE_PARM_DESC(req_count, "ISO OUT endpoint request count");

static int audio_buf_size =  48000 * 2 * 20 /1000;
module_param(audio_buf_size, int, S_IRUGO);
MODULE_PARM_DESC(audio_buf_size, "Audio buffer size");

static int generic_set_cmd(struct usb_audio_control *con, u8 cmd, int value);
static int generic_get_cmd(struct usb_audio_control *con, u8 cmd);

static struct f_audio * __audio = NULL;

static struct usb_descriptor_header **f_audio_desc = NULL;
static int speak_enable = 0;
static int F_AUDIO_NUM_INTERFACES = 1;
static int UAC_DT_AC_HEADER_LENGTH = 0;
static int UAC_DT_TOTAL_LENGTH = 0;

/*************************************************************/
/*
 * DESCRIPTORS ... most are static, but strings and full
 * configuration descriptors are built on demand.
 */
/* static struct usb_string audio_strings_def[] = { */
/* 	[0].s	= "USB Audio Device", */
/* 	[1].s	= "USB Audio SPK Interface", */
/* 	[2].s	= "Ingenic USB Audio", */
/* 	{  },	/1* end of list *1/ */
/* }; */

struct usb_string audio_strings_def[3] = {0};

static struct usb_gadget_strings uac_string_table = {
	.language		= 0x0409,	/* en-US */
	.strings		= audio_strings_def,
};

static struct usb_gadget_strings *uac_strings[] = {
	&uac_string_table,
	NULL,
};

/*
 * We have two interfaces- AudioControl and AudioStreaming
 * TODO: only supcard playback currently
 */
 /*
#define F_AUDIO_AC_INTERFACE	0
#define F_AUDIO_SPK_AS_INTERFACE    1
#define F_AUDIO_MIC_AS_INTERFACE    2
*/
/* #if defined(CONFIG_GADGET_UAC1_PLAY) && defined(CONFIG_GADGET_UAC1_CAP) */
/* #define F_AUDIO_NUM_INTERFACES	    2 */
/* #elif defined(CONFIG_GADGET_UAC1_PLAY) || defined(CONFIG_GADGET_UAC1_CAP) */
/* #define F_AUDIO_NUM_INTERFACES	    1 */
/* #else */
/* #error cap or paly should be defined */
/* #endif */

/* B.3.1  Standard AC Interface Descriptor */
static struct usb_interface_descriptor ac_interface_desc  = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	.bNumEndpoints =	0,
	.bInterfaceClass =	USB_CLASS_AUDIO,
	.bInterfaceSubClass =	USB_SUBCLASS_AUDIOCONTROL,
};

DECLARE_UAC_AC_HEADER_DESCRIPTOR(2);

/* #define UAC_DT_AC_HEADER_LENGTH	UAC_DT_AC_HEADER_SIZE(F_AUDIO_NUM_INTERFACES) */
/* 2 input terminal, 2 output terminal and 2 feature unit */
/* #if defined(CONFIG_GADGET_UAC1_PLAY) && defined(CONFIG_GADGET_UAC1_CAP) */
/* #define UAC_DT_TOTAL_LENGTH (UAC_DT_AC_HEADER_LENGTH +	    \ */
/* 			    UAC_DT_INPUT_TERMINAL_SIZE +    \ */
/* 			    UAC_DT_OUTPUT_TERMINAL_SIZE +   \ */
/* 			    UAC_DT_FEATURE_UNIT_SIZE(0) +   \ */
/* 			    UAC_DT_INPUT_TERMINAL_SIZE +    \ */
/* 			    UAC_DT_OUTPUT_TERMINAL_SIZE +   \ */
/* 			    UAC_DT_FEATURE_UNIT_SIZE(0)) */
/* #elif defined(CONFIG_GADGET_UAC1_PLAY) || defined(CONFIG_GADGET_UAC1_CAP) */
/* #define UAC_DT_TOTAL_LENGTH (UAC_DT_AC_HEADER_LENGTH +	    \ */
/* 			    UAC_DT_INPUT_TERMINAL_SIZE +    \ */
/* 			    UAC_DT_OUTPUT_TERMINAL_SIZE +   \ */
/* 			    UAC_DT_FEATURE_UNIT_SIZE(0)) */
/* #else */
/* #error cap or paly should be defined */
/* #endif */
/* B.3.2  Class-Specific AC Interface Descriptor */
static struct uac1_ac_header_descriptor_2 ac_header_desc = {
	/* .bLength =		UAC_DT_AC_HEADER_LENGTH, */
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubtype =	UAC_HEADER,
	.bcdADC =		__constant_cpu_to_le16(0x0100),
	/* .wTotalLength =		__constant_cpu_to_le16(UAC_DT_TOTAL_LENGTH), */
	/* .bInCollection =	F_AUDIO_NUM_INTERFACES, */
	.baInterfaceNr = {
		[0] =		0, /*dynamic*/
		[1] =		0, /*dynamic*/
	}
};

#define SPK_INPUT_TERMINAL_ID	1
#define SPK_OUTPUT_TERMINAL_ID	3
#define SPK_FEATURE_UNIT_ID	5
static struct uac_input_terminal_descriptor spk_input_terminal_desc = {
	.bLength =		UAC_DT_INPUT_TERMINAL_SIZE,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubtype =	UAC_INPUT_TERMINAL,
	.bTerminalID =		SPK_INPUT_TERMINAL_ID,
	.wTerminalType =	UAC_TERMINAL_STREAMING,
	.bAssocTerminal =	0,
	.wChannelConfig =	0x3,
};

#define MIC_INPUT_TERMINAL_ID	2
#define MIC_OUTPUT_TERMINAL_ID	4
#define MIC_FEATURE_UNIT_ID	6
static struct uac_input_terminal_descriptor mic_input_terminal_desc = {
	.bLength =		UAC_DT_INPUT_TERMINAL_SIZE,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubtype =	UAC_INPUT_TERMINAL,
	.bTerminalID =		MIC_INPUT_TERMINAL_ID,
	.wTerminalType =	UAC_INPUT_TERMINAL_MICROPHONE,
	.bAssocTerminal =	0,
	.wChannelConfig =	0x3,
};

DECLARE_UAC_FEATURE_UNIT_DESCRIPTOR(0);


static struct uac_feature_unit_descriptor_0 spk_feature_unit_desc = {
	.bLength		= UAC_DT_FEATURE_UNIT_SIZE(0),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubtype	= UAC_FEATURE_UNIT,
	.bUnitID		= SPK_FEATURE_UNIT_ID,
	.bSourceID		= SPK_INPUT_TERMINAL_ID,
	.bControlSize		= (0 + 1) * sizeof(__le16),
	.bmaControls[0]		= (UAC_FU_MUTE | UAC_FU_VOLUME),
	//.bmaControls[0]		= UAC_FU_VOLUME,
};

static struct uac_feature_unit_descriptor_0 mic_feature_unit_desc = {
	.bLength		= UAC_DT_FEATURE_UNIT_SIZE(0),
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubtype	= UAC_FEATURE_UNIT,
	.bUnitID		= MIC_FEATURE_UNIT_ID,
	.bSourceID		= MIC_INPUT_TERMINAL_ID,
	.bControlSize		= (0 + 1) * sizeof(__le16),
	.bmaControls[0]		= (UAC_FU_MUTE | UAC_FU_VOLUME),
	//.bmaControls[0]		= UAC_FU_VOLUME,
};

static struct usb_audio_control spk_mute_control = {
	.list = LIST_HEAD_INIT(spk_mute_control.list),
	.name = "Spk Mute Control",
	.type = UAC_FU_MUTE,
	/* Todo: add real Mute control code */
	.set = generic_set_cmd,
	.get = generic_get_cmd,
};

static struct usb_audio_control mic_mute_control = {
	.list = LIST_HEAD_INIT(mic_mute_control.list),
	.name = "Mic Mute Control",
	.type = UAC_FU_MUTE,
	/* Todo: add real Mute control code */
	.set = generic_set_cmd,
	.get = generic_get_cmd,
};

static struct usb_audio_control spk_volume_control = {
	.list = LIST_HEAD_INIT(spk_volume_control.list),
	.name = "Spk Volume Control",
	.type = UAC_FU_VOLUME,
	/* Todo: add real Volume control code */
	.set = generic_set_cmd,
	.get = generic_get_cmd,
};

static struct usb_audio_control mic_volume_control = {
	.list = LIST_HEAD_INIT(mic_volume_control.list),
	.name = "Mic Volume Control",
	.type = UAC_FU_VOLUME,
	/* Todo: add real Volume control code */
	.set = generic_set_cmd,
	.get = generic_get_cmd,
};

static struct usb_audio_control_selector spk_feature_unit = {
	.list = LIST_HEAD_INIT(spk_feature_unit.list),
	.id = SPK_FEATURE_UNIT_ID,
	.name = "Mute & Volume Control",
	.type = UAC_FEATURE_UNIT,
	.desc = (struct usb_descriptor_header *)&spk_feature_unit_desc,
};
static struct usb_audio_control_selector mic_feature_unit = {
	.list = LIST_HEAD_INIT(mic_feature_unit.list),
	.id = MIC_FEATURE_UNIT_ID,
	.name = "Mute & Volume Control",
	.type = UAC_FEATURE_UNIT,
	.desc = (struct usb_descriptor_header *)&mic_feature_unit_desc,
};

static struct uac1_output_terminal_descriptor spk_output_terminal_desc = {
	.bLength		= UAC_DT_OUTPUT_TERMINAL_SIZE,
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubtype	= UAC_OUTPUT_TERMINAL,
	.bTerminalID		= SPK_OUTPUT_TERMINAL_ID,
	.wTerminalType		= UAC_OUTPUT_TERMINAL_SPEAKER,
	.bAssocTerminal		= 0/*SPK_FEATURE_UNIT_ID*/,
	.bSourceID		= SPK_FEATURE_UNIT_ID ,
};

static struct uac1_output_terminal_descriptor mic_output_terminal_desc = {
	.bLength		= UAC_DT_OUTPUT_TERMINAL_SIZE,
	.bDescriptorType	= USB_DT_CS_INTERFACE,
	.bDescriptorSubtype	= UAC_OUTPUT_TERMINAL,
	.bTerminalID		= MIC_OUTPUT_TERMINAL_ID,
	.wTerminalType		= UAC_TERMINAL_STREAMING,
	.bAssocTerminal		= 0/*MIC_FEATURE_UNIT_ID*/,
	.bSourceID		= MIC_FEATURE_UNIT_ID,
};

/* B.4.1  Standard AS Interface Descriptor */
static struct usb_interface_descriptor spk_as_interface_alt_0_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	.bAlternateSetting =	0,
	.bNumEndpoints =	0,
	.bInterfaceClass =	USB_CLASS_AUDIO,
	.bInterfaceSubClass =	USB_SUBCLASS_AUDIOSTREAMING,
};

static struct usb_interface_descriptor mic_as_interface_alt_0_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	.bAlternateSetting =	0,
	.bNumEndpoints =	0,
	.bInterfaceClass =	USB_CLASS_AUDIO,
	.bInterfaceSubClass =	USB_SUBCLASS_AUDIOSTREAMING,
};

static struct usb_interface_descriptor spk_as_interface_alt_1_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	.bAlternateSetting =	1,
	.bNumEndpoints =	1,
	.bInterfaceClass =	USB_CLASS_AUDIO,
	.bInterfaceSubClass =	USB_SUBCLASS_AUDIOSTREAMING,
};

static struct usb_interface_descriptor mic_as_interface_alt_1_desc = {
	.bLength =		USB_DT_INTERFACE_SIZE,
	.bDescriptorType =	USB_DT_INTERFACE,
	.bAlternateSetting =	1,
	.bNumEndpoints =	1,
	.bInterfaceClass =	USB_CLASS_AUDIO,
	.bInterfaceSubClass =	USB_SUBCLASS_AUDIOSTREAMING,
};

/* B.4.2  Class-Specific AS Interface Descriptor */
static struct uac1_as_header_descriptor spk_as_header_desc = {
	.bLength =		UAC_DT_AS_HEADER_SIZE,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubtype =	UAC_AS_GENERAL,
	.bTerminalLink =	SPK_INPUT_TERMINAL_ID,
	.bDelay =		1,
	.wFormatTag =		UAC_FORMAT_TYPE_I_PCM,
};

static struct uac1_as_header_descriptor mic_as_header_desc = {
	.bLength =		UAC_DT_AS_HEADER_SIZE,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubtype =	UAC_AS_GENERAL,
	.bTerminalLink =	MIC_OUTPUT_TERMINAL_ID,
	.bDelay =		1,
	.wFormatTag =		UAC_FORMAT_TYPE_I_PCM,
};

DECLARE_UAC_FORMAT_TYPE_I_DISCRETE_DESC(1);

static struct uac_format_type_i_discrete_descriptor_1 spk_as_type_i_desc = {
	.bLength =		UAC_FORMAT_TYPE_I_DISCRETE_DESC_SIZE(1),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubtype =	UAC_FORMAT_TYPE,
	.bFormatType =		UAC_FORMAT_TYPE_I,
	.bSubframeSize =	2,
	.bBitResolution =	16,
	.bSamFreqType =		1,
};

static struct uac_format_type_i_discrete_descriptor_1 mic_as_type_i_desc = {
	.bLength =		UAC_FORMAT_TYPE_I_DISCRETE_DESC_SIZE(1),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubtype =	UAC_FORMAT_TYPE,
	.bFormatType =		UAC_FORMAT_TYPE_I,
	.bSubframeSize =	2,
	.bBitResolution =	16,
	.bSamFreqType =		1,
};

/* Standard ISO OUT Endpoint Descriptor */
static struct usb_endpoint_descriptor as_out_ep_desc  = {
	.bLength =		USB_DT_ENDPOINT_AUDIO_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_SYNC_ADAPTIVE
				| USB_ENDPOINT_XFER_ISOC,
	.wMaxPacketSize =	__constant_cpu_to_le16(OUT_EP_MAX_PACKET_SIZE),
	.bInterval =		4,
};

/*Standard ISO IN Endpoint Descriptor*/
static struct usb_endpoint_descriptor as_in_ep_desc   = {
	.bLength =		USB_DT_ENDPOINT_AUDIO_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_SYNC_SYNC
				| USB_ENDPOINT_XFER_ISOC,
	.wMaxPacketSize =	__constant_cpu_to_le16(IN_EP_MAX_PACKET_SIZE),
	.bInterval =		4,

};

/* Class-specific AS ISO OUT Endpoint Descriptor */
static struct uac_iso_endpoint_descriptor as_iso_out_desc  = {
	.bLength =		UAC_ISO_ENDPOINT_DESC_SIZE,
	.bDescriptorType =	USB_DT_CS_ENDPOINT,
	.bDescriptorSubtype =	UAC_EP_GENERAL,
	.bmAttributes = 	1,
	.bLockDelayUnits =	1,
	.wLockDelay =		__constant_cpu_to_le16(1),
};

/* Class-specific AS ISO IN Endpoint Descriptor */
static struct uac_iso_endpoint_descriptor as_iso_in_desc = {
	.bLength =		UAC_ISO_ENDPOINT_DESC_SIZE,
	.bDescriptorType =	USB_DT_CS_ENDPOINT,
	.bDescriptorSubtype =	UAC_EP_GENERAL,
	.bmAttributes =		1,
	.bLockDelayUnits =	1,
	.wLockDelay =		__constant_cpu_to_le16(1),
};


static struct usb_interface_assoc_descriptor uac_iad  = {
	.bLength		= sizeof(uac_iad),
	.bDescriptorType	= USB_DT_INTERFACE_ASSOCIATION,
	/*.bFirstInterface	= 0,*/ /*dynamic*/
	/*.bInterfaceCount	= 3,*/ /*dynamic*/
	.bFunctionClass		= USB_CLASS_AUDIO,
	.bFunctionSubClass	= 0,
	.bFunctionProtocol	= UAC_VERSION_1,
	.iFunction		= 0,
};

#if 0
static struct usb_descriptor_header *f_audio_desc[]  = {
	(struct usb_descriptor_header *)&uac_iad,
	(struct usb_descriptor_header *)&ac_interface_desc,
	(struct usb_descriptor_header *)&ac_header_desc,

#ifdef CONFIG_GADGET_UAC1_PLAY
	(struct usb_descriptor_header *)&spk_input_terminal_desc,
#endif
#ifdef CONFIG_GADGET_UAC1_CAP
	(struct usb_descriptor_header *)&mic_input_terminal_desc,
#endif

#ifdef CONFIG_GADGET_UAC1_PLAY
	(struct usb_descriptor_header *)&spk_output_terminal_desc,
#endif
#ifdef CONFIG_GADGET_UAC1_CAP
	(struct usb_descriptor_header *)&mic_output_terminal_desc,
#endif

#ifdef CONFIG_GADGET_UAC1_PLAY
	(struct usb_descriptor_header *)&spk_feature_unit_desc,
#endif
#ifdef CONFIG_GADGET_UAC1_CAP
	(struct usb_descriptor_header *)&mic_feature_unit_desc,
#endif

#ifdef CONFIG_GADGET_UAC1_PLAY
	/* *************************spk********************/
	(struct usb_descriptor_header *)&spk_as_interface_alt_0_desc,
	(struct usb_descriptor_header *)&spk_as_interface_alt_1_desc,
	(struct usb_descriptor_header *)&spk_as_header_desc,

	(struct usb_descriptor_header *)&spk_as_type_i_desc,

	(struct usb_descriptor_header *)&as_out_ep_desc,
	(struct usb_descriptor_header *)&as_iso_out_desc,
#endif

#ifdef CONFIG_GADGET_UAC1_CAP
	/*****************************mic*******************/
	(struct usb_descriptor_header *)&mic_as_interface_alt_0_desc,
	(struct usb_descriptor_header *)&mic_as_interface_alt_1_desc,
	(struct usb_descriptor_header *)&mic_as_header_desc,

	(struct usb_descriptor_header *)&mic_as_type_i_desc,

	(struct usb_descriptor_header *)&as_in_ep_desc,
	(struct usb_descriptor_header *)&as_iso_in_desc,
#endif

	NULL,
};
#endif

/*
 * This function is an ALSA sound card following USB Audio Class Spec 1.0.
 */

/*-------------------------------------------------------------------------*/

static struct f_audio_buf *f_audio_buffer_alloc(int buf_size)
{
	struct f_audio		*audio = __audio;
	struct f_audio_buf *copy_buf;
	node_t *node = NULL;

	copy_buf = kzalloc(sizeof *copy_buf, GFP_ATOMIC);
	if (!copy_buf)
		return ERR_PTR(-ENOMEM);

	node = get_free_node(audio->play_frame_list);
	if (!node)
		return ERR_PTR(-ENOMEM);
	copy_buf->buf = node->data;
	copy_buf->node = node;

	if (!copy_buf->buf) {
		kfree(copy_buf);
		return ERR_PTR(-ENOMEM);
	}

	return copy_buf;
}

static void f_audio_buffer_free(struct f_audio_buf *audio_buf)
{
	//kfree(audio_buf->buf);
	struct f_audio		*audio = __audio;
	node_t *node = NULL;

	node = audio_buf->node;

	if (node != NULL) {
		node->size = audio_buf->actual;
		put_use_node(audio->play_frame_list, node);
	}

	kfree(audio_buf);
}
/*-------------------------------------------------------------------------*/


static inline struct f_audio *func_to_audio(struct usb_function *f)
{
	return container_of(f, struct f_audio, card.func);
}

/*-------------------------------------------------------------------------*/
#ifdef CONFIG_GADGET_UAC1_PLAY
static struct tasklet_struct putuse_work_t;

static void putuse_work(unsigned long data)
{
	struct f_audio *audio = (struct f_audio *)data;
	struct f_audio_buf *play_buf;

	spin_lock_irq(&audio->lock);
	if (list_empty(&audio->play_queue)) {
		spin_unlock_irq(&audio->lock);
		return;
	}
	play_buf = list_first_entry(&audio->play_queue,
			struct f_audio_buf, list);
	list_del(&play_buf->list);
	spin_unlock_irq(&audio->lock);

	f_audio_buffer_free(play_buf);

}

static void f_audio_playback_work(struct work_struct *data)
{
	struct f_audio *audio = container_of(data, struct f_audio,
					playback_work);
	struct f_audio_buf *play_buf;

	spin_lock_irq(&audio->lock);
	if (list_empty(&audio->play_queue)) {
		spin_unlock_irq(&audio->lock);
		return;
	}
	play_buf = list_first_entry(&audio->play_queue,
			struct f_audio_buf, list);
	list_del(&play_buf->list);
	spin_unlock_irq(&audio->lock);

	/* u_audio_playback(&audio->card, play_buf->buf, play_buf->actual); */
	f_audio_buffer_free(play_buf);
}

static int f_audio_out_ep_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_audio *audio = req->context;
	struct usb_composite_dev *cdev = audio->card.func.config->cdev;
	struct f_audio_buf *copy_buf = audio->copy_buf;
	/* static unsigned int time; */
	/* struct timeval tval; */
	/* u8 *ptr = NULL; */
	int err;

	if (!copy_buf)
		return -EINVAL;

	/* Copy buffer is full, add it to the play_queue */
	if (audio_buf_size - copy_buf->actual < req->actual) {
		list_add_tail(&copy_buf->list, &audio->play_queue);
		tasklet_hi_schedule(&putuse_work_t);
		//schedule_work(&audio->playback_work);
		audio->copy_buf = NULL;
		copy_buf = f_audio_buffer_alloc(audio_buf_size);
		if (IS_ERR(copy_buf))
			return -ENOMEM;
	}


	/* do_gettimeofday(&tval); */
	/* time = tval.tv_sec * 1000000 + tval.tv_usec; */
	/* ptr = req->buf; */
        /* *(u32*)ptr = cpu_to_le32(time); */

	memcpy(copy_buf->buf + copy_buf->actual, req->buf, req->actual);
	copy_buf->actual += req->actual;
	audio->copy_buf = copy_buf;

	/* pcm_count += req->actual; */
	err = usb_ep_queue(ep, req, GFP_ATOMIC);
	if (err)
		ERROR(cdev, "%s queue req: %d\n", ep->name, err);

	return 0;

}

static void f_audio_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_audio *audio = req->context;
	int status = req->status;
	u32 data = 0;
	struct usb_ep *out_ep = audio->out_ep;

	switch (status) {

	case 0:				/* normal completion? */
		if (ep == out_ep)
			f_audio_out_ep_complete(ep, req);
		else if (audio->set_con) {
			memcpy(&data, req->buf, req->length);
			audio->set_con->set(audio->set_con, audio->set_cmd,
					le16_to_cpu(data));
			audio->set_con = NULL;
		}
		break;
	default:
		break;
	}
}
#endif





static void f_audio_control_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_audio *audio = req->context;
	int status = req->status;
	u32 data = 0;
	struct usb_ep *out_ep = audio->out_ep;
	struct f_audio_event_list *e_list;
	unsigned long flags;

	switch (status) {

	case 0:				/* normal completion? */
		if (audio->set_con) {
			memcpy(&data, req->buf, req->length);
			audio->set_con->set(audio->set_con, audio->set_cmd,
					le16_to_cpu(data));
			e_list = kzalloc(sizeof(*e_list), GFP_ATOMIC);
			if (!e_list)
				return;
			e_list->event_con = audio->set_con;
			spin_lock_irqsave(&audio->event_lock, flags);
			list_add_tail(&e_list->list, &audio->event_list);
			spin_unlock_irqrestore(&audio->event_lock, flags);
			wake_up(&audio->event_queue);
			audio->set_con = NULL;
		}
		break;
	default:
		break;
	}
}


static int audio_set_intf_req(struct usb_function *f,
		const struct usb_ctrlrequest *ctrl)
{
	struct f_audio		*audio = func_to_audio(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request	*req = cdev->req;
	u8			id = ((le16_to_cpu(ctrl->wIndex) >> 8) & 0xFF);
	u16			len = le16_to_cpu(ctrl->wLength);
	u16			w_value = le16_to_cpu(ctrl->wValue);
	u8			con_sel = (w_value >> 8) & 0xFF;
	u8			cmd = (ctrl->bRequest & 0x0F);
	struct usb_audio_control_selector *cs;
	struct usb_audio_control *con;

	DBG(cdev, "set: bRequest 0x%x, w_value 0x%04x, len %d, entity %d\n",
			ctrl->bRequest, w_value, len, id);
	DBG(cdev, "%s %d brequest:0x%x\n", __func__, __LINE__, ctrl->bRequest);
	list_for_each_entry(cs, &audio->cs, list) {
		if (cs->id == id) {
			list_for_each_entry(con, &cs->control, list) {
				if (con->type == con_sel) {
					audio->set_con = con;
					break;
				}
			}
			break;
		}
	}

	audio->set_cmd = cmd;
	req->context = audio;
	req->complete = f_audio_control_complete;

	return len;
}

static int audio_get_intf_req(struct usb_function *f,
		const struct usb_ctrlrequest *ctrl)
{
	struct f_audio		*audio = func_to_audio(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request	*req = cdev->req;
	int			value = -EOPNOTSUPP;
	u8			id = ((le16_to_cpu(ctrl->wIndex) >> 8) & 0xFF);
	u16			len = le16_to_cpu(ctrl->wLength);
	u16			w_value = le16_to_cpu(ctrl->wValue);
	u8			con_sel = (w_value >> 8) & 0xFF;
	u8			cmd = (ctrl->bRequest & 0x0F);
	struct usb_audio_control_selector *cs;
	struct usb_audio_control *con;

	DBG(cdev, "get: bRequest 0x%x, w_value 0x%04x, len %d, entity %d\n",
			ctrl->bRequest, w_value, len, id);
	DBG(cdev, "%s %d brequest:0x%x\n", __func__, __LINE__, ctrl->bRequest);
	list_for_each_entry(cs, &audio->cs, list) {
		if (cs->id == id) {
			list_for_each_entry(con, &cs->control, list) {
				if (con->type == con_sel && con->get) {
					value = con->get(con, cmd);
					break;
				}
			}
			break;
		}
	}

	req->context = audio;
	req->complete = f_audio_control_complete;
	len = min_t(size_t, sizeof(value), len);
	memcpy(req->buf, &value, len);

	return len;
}




static int audio_set_endpoint_req(struct usb_function *f,
		const struct usb_ctrlrequest *ctrl)
{
	struct usb_composite_dev *cdev = f->config->cdev;
	int			value = -EOPNOTSUPP;
	u16			ep = le16_to_cpu(ctrl->wIndex);
	u16			len = le16_to_cpu(ctrl->wLength);
	u16			w_value = le16_to_cpu(ctrl->wValue);

	DBG(cdev, "bRequest 0x%x, w_value 0x%04x, len %d, endpoint %d\n",
			ctrl->bRequest, w_value, len, ep);
	DBG(cdev, "%s %d brequest:0x%x\n", __func__, __LINE__, ctrl->bRequest);
	switch (ctrl->bRequest) {
	case UAC_SET_CUR:
		value = 0;
		break;

	case UAC_SET_MIN:
		break;

	case UAC_SET_MAX:
		break;

	case UAC_SET_RES:
		break;

	case UAC_SET_MEM:
		break;

	default:
		break;
	}

	return value;
}

static int audio_get_endpoint_req(struct usb_function *f,
		const struct usb_ctrlrequest *ctrl)
{
	struct usb_composite_dev *cdev = f->config->cdev;
	int value = -EOPNOTSUPP;
	u8 ep = ((le16_to_cpu(ctrl->wIndex) >> 8) & 0xFF);
	u16 len = le16_to_cpu(ctrl->wLength);
	u16 w_value = le16_to_cpu(ctrl->wValue);

	u8 *buf = cdev->req->buf;

	DBG(cdev, "bRequest 0x%x, w_value 0x%04x, len %d, endpoint %d\n",
			ctrl->bRequest, w_value, len, ep);
	DBG(cdev, "%s %d brequest:0x%x\n", __func__, __LINE__, ctrl->bRequest);

	switch (ctrl->bRequest) {
	case UAC_GET_CUR:
	case UAC_GET_MIN:
	case UAC_GET_MAX:
	case UAC_GET_RES:
			buf[0] = (u8)IN_SAMPLE_RATE;
			buf[1] = (u8)(IN_SAMPLE_RATE >> 8);
			buf[2] = (u8)(IN_SAMPLE_RATE >> 16);
			value = 3;
		/* printk("%s %d, sample_rate:%d\n", __func__, __LINE__, IN_SAMPLE_RATE); */
		break;
	case UAC_GET_MEM:
		break;
	default:
		break;
	}

	return value;
}

static int
f_audio_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request	*req = cdev->req;
	int			value = -EOPNOTSUPP;
	u16			w_index = le16_to_cpu(ctrl->wIndex);
	u16			w_value = le16_to_cpu(ctrl->wValue);
	u16			w_length = le16_to_cpu(ctrl->wLength);

	/* composite driver infrastructure handles everything; interface
	 * activation uses set_alt().
	 */
	switch (ctrl->bRequestType) {

	case USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE:
		value = audio_set_intf_req(f, ctrl);
		break;

	case USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE:
		value = audio_get_intf_req(f, ctrl);
		break;

	case USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_ENDPOINT:
		value = audio_set_endpoint_req(f, ctrl);
		break;

	case USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_ENDPOINT:
		value = audio_get_endpoint_req(f, ctrl);
		break;

	default:
		ERROR(cdev, "invalid control req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
	}

	/* respond with data transfer or status phase? */
	if (value >= 0) {
		DBG(cdev, "audio req %02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
		req->zero = 0;
		req->length = value;
		value = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
		if (value < 0)
			ERROR(cdev, "audio response on err %d\n", value);
	}

	/* device either stalls (value < 0) or reports success */
	return value;
}



#ifdef CONFIG_GADGET_UAC1_PLAY
static int spk_interface_conn(struct usb_function *f)
{
	struct f_audio		*audio = func_to_audio(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_ep *out_ep = audio->out_ep;
	struct usb_request *req;
	struct usb_gadget *gadget = cdev->gadget;
	int i = 0, err = 0;

	usb_ep_enable(out_ep);
	config_ep_by_speed(gadget, f, out_ep);
	out_ep->driver_data = audio;
	audio->copy_buf = f_audio_buffer_alloc(audio_buf_size);
	if (IS_ERR(audio->copy_buf))
		return err;
		/* return -ENOMEM; */

	/*
	 * allocate a bunch of read buffers
	 * and queue them all at once.
	 */
	for (i = 0; i < req_count && err == 0; i++) {
		req = usb_ep_alloc_request(out_ep, GFP_ATOMIC);
		if (req) {
			req->buf = kzalloc(req_buf_size,
					GFP_ATOMIC);
			if (req->buf) {
				req->length = req_buf_size;
				req->context = audio;
				req->complete =
					f_audio_complete;
				err = usb_ep_queue(out_ep,
					req, GFP_ATOMIC);
				if (err)
					ERROR(cdev,
					"%s queue req: %d\n",
					out_ep->name, err);
			} else
				err = -ENOMEM;
		} else
			err = -ENOMEM;
	}
	return 0;
}

static void spk_interface_disconn(struct usb_function *f)
{
	struct f_audio		*audio = func_to_audio(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_ep *out_ep = audio->out_ep;

	struct f_audio_buf *copy_buf = audio->copy_buf;
	if (copy_buf) {
		list_add_tail(&copy_buf->list,
				&audio->play_queue);
		schedule_work(&audio->playback_work);
	}
	usb_ep_disable(out_ep);

}
#endif

#ifdef CONFIG_GADGET_UAC1_CAP
static int mic_interface_conn(struct usb_function * f)
{
	struct f_audio *audio = func_to_audio(f);

	int ret= usb_ep_enable(audio->in_ep);
	if(ret){
		usb_ep_disable(audio->in_ep);
		return ret;
	}
	audio->mic_disconn = 0;
	audio->in_online = 1;

#ifdef CONFIG_GADGET_UAC1_CAP_USER
	wake_up(&audio->online_wq);
#endif

#ifdef CONFIG_GADGET_UAC1_CAP_MIC
	wake_up_process(audio->capture_thread);
#endif
	return 0;

}

#ifdef CONFIG_GADGET_UAC1_CAP_MIC
#define RECORD_PERIOD_SIZE  256
struct cb_node {
        struct list_head node;
        struct list_head free;
        struct usb_request *req;
};
#endif
#endif

static struct usb_request * audio_req_get(struct f_audio * audio,struct list_head *head)
{
	unsigned long flags;
	struct usb_request *req;

	spin_lock_irqsave(&audio->lock, flags);
	if (list_empty(head)) {
		req = NULL;
	} else {
		req = list_first_entry(head, struct usb_request,
				list);
		list_del(&req->list);
	}
	spin_unlock_irqrestore(&audio->lock, flags);
	return req;

}

static void audio_req_put(struct f_audio *audio,struct list_head *head,struct usb_request *req)
{
	unsigned long flags;
	spin_lock_irqsave(&audio->lock, flags);
	list_add_tail(&req->list, head);
	spin_unlock_irqrestore(&audio->lock,flags);
	return;
}

static int audio_is_req_empty(struct f_audio *audio, struct list_head *head)
{
	unsigned long flags;
	int ret;
	spin_lock_irqsave(&audio->lock, flags);
	ret = list_empty(head);
	spin_unlock_irqrestore(&audio->lock, flags);
	return ret;
}
#ifdef CONFIG_GADGET_UAC1_CAP
static void mic_interface_disconn(struct usb_function * f)
{
	struct f_audio *audio = func_to_audio(f);
	struct usb_request *req = NULL;
#ifdef CONFIG_GADGET_UAC1_CAP_MIC
	struct cb_node *p;

	if(audio->in_online == 0)
	{
		printk ("%s, have disabled !!\n", __func__);
		return;
	}

	while (!list_empty(&audio->cb_list_queued))
	{

		spin_lock (&audio->cb_list_lock);
		p = list_first_entry(&audio->cb_list_queued, struct cb_node, node);
		list_del(&p->node);
		spin_unlock (&audio->cb_list_lock);

		usb_ep_dequeue(audio->in_ep, p->req);

		kfree(p->req->buf);
		usb_ep_free_request(audio->in_ep , p->req);
		kfree(p);
	}
#endif
/* #ifdef CONFIG_GADGET_UAC1_CAP_USER */
/* 	while(!audio_is_req_empty(audio,&audio->audio_data_list)) { */
/* 		req = audio_req_get(audio,&audio->audio_data_list); */
/* 		audio_req_put(audio, &audio->idle_reqs,req); */

/* 	} */
/* 	if (audio->cur_req != NULL) */
/* 		usb_ep_dequeue(audio->in_ep, audio->cur_req); */
/* 	audio->cur_req = NULL; */
/* 	printk("%s clean req queue!\n", __func__); */
/* #endif */

	audio->in_online = 0;
	audio->mic_disconn = 1;
	usb_ep_disable(audio->in_ep);

	return;
}
#endif

static int play_intf = 0;
static int cap_intf = 0;

static int f_audio_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	int event = 0;
	int ret = 0;
	unsigned long flags;
	struct f_audio_event_list *e_list = NULL;
	struct f_audio		*audio = func_to_audio(f);

	INFO(f->config->cdev, "uac_function_set_alt(%u, %u)\n", intf, alt);
#ifdef CONFIG_GADGET_UAC1_PLAY
	if (speak_enable) {
		if(intf == play_intf)
		{
			if (alt == 1) {
				event = UAC_EVENT_SPEAK_ON;
				ret = spk_interface_conn(f);
			} else {
				event = UAC_EVENT_SPEAK_OFF;
				spk_interface_disconn(f);
			}
		}

	}

#endif

#ifdef CONFIG_GADGET_UAC1_CAP

	if(intf == cap_intf)
	{
		if (alt == 1) {
			event = UAC_EVENT_RECORD_ON;
			ret =  mic_interface_conn(f);
		} else {
			event = UAC_EVENT_RECORD_OFF;
			mic_interface_disconn(f);
		}
	}
#endif

	e_list = kzalloc(sizeof(*e_list), GFP_ATOMIC);
	if (!e_list)
		return;
	e_list->event = event;
	e_list->event_con = NULL;
	spin_lock_irqsave(&audio->event_lock, flags);
	list_add_tail(&e_list->list, &audio->event_list);
	spin_unlock_irqrestore(&audio->event_lock, flags);
	wake_up(&audio->event_queue);

	return ret;
}


static void f_audio_disable(struct usb_function *f)
{
	struct f_audio *audio = func_to_audio(f);
#ifdef CONFIG_GADGET_UAC1_CAP
	if(audio->in_online == 1)
	{
		audio->in_online = 0;
		audio->mic_disconn = 1;
#ifdef CONFIG_GADGET_UAC1_CAP_USER
		wake_up(&audio->online_wq);
#endif
		mic_interface_disconn(f);
	}
#endif
	return;
}

/*-------------------------------------------------------------------------*/

static void f_audio_build_desc(struct f_audio *audio)
{
	struct gaudio *card = &audio->card;
	u8 *sam_freq;
	int rate;

#ifdef CONFIG_GADGET_UAC1_PLAY
	if (speak_enable) {
		spk_input_terminal_desc.bNrChannels = 1;
		spk_as_type_i_desc.bNrChannels = 1;

		/* Set sample rates */
		//rate = u_audio_get_playback_rate(card);
		rate = OUT_SAMPLE_RATE;
		sam_freq = spk_as_type_i_desc.tSamFreq[0];
		memcpy(sam_freq, &rate, 3);

	}
#endif


#ifdef CONFIG_GADGET_UAC1_CAP
	/* Set channel numbers */
	mic_input_terminal_desc.bNrChannels = 1;
	mic_as_type_i_desc.bNrChannels = 1;

	//printk("get capture channels:%d\n", u_audio_get_capture_channels(card));
	/* Set sample rates */
	//rate = u_audio_get_capture_rate(card);
	rate = IN_SAMPLE_RATE;
	sam_freq = mic_as_type_i_desc.tSamFreq[0];
	memcpy(sam_freq, &rate, 3);

#endif
	/* Todo: Set Sample bits and other parameters */

	return;
}

#ifdef CONFIG_GADGET_UAC1_CAP_USER
static int gaudio_lock(atomic_t *excl)
{
	if(atomic_inc_return(excl) == 1)
		return 0;
	else{
		atomic_dec(excl);
		return -1;
	}
}

static  void gaudio_unlock(atomic_t *excl)
{
	atomic_dec(excl);

}


static int gaudio_open(struct inode * ip,struct file *fp)
{
	if(!__audio)
		return -ENODEV;
	if(gaudio_lock(&__audio->open_excl))
	    return -EBUSY;
	fp->private_data = __audio;
	return 0;
}

static int gaudio_release(struct inode *ip,struct file *fp)
{
	gaudio_unlock(&((struct f_audio *)fp->private_data)->open_excl);
	return 0;
}


int req_null_cnt = 0;
/* static int g_test_audio_write_count = 0; */
static ssize_t gaudio_write(struct file *fp, const char __user *buf, size_t count, loff_t *ops)
{
	struct f_audio *audio = fp->private_data;
	int r = count;
	int offset=0;
	int ret=0;
	int len=0;
	unsigned long flags=0;
	struct usb_request *req;
//	pr_info("gaudio_write(%d)\n", count);
	if (gaudio_lock(&audio->write_excl))
		    return -EBUSY;
	ret = wait_event_interruptible(audio->online_wq, (audio->in_online == 1) || (audio->mic_disconn));
	if(ret < 0)
	{
		gaudio_unlock(&audio->write_excl);
		return -1;
	}
	while (r > 0) {
		len = 0;

		/* g_test_audio_write_count++; */
		/* if (0 == g_test_audio_write_count%100) */
		/* 	printk(KERN_DEBUG "g_test_audio_write_count = %d\n", g_test_audio_write_count); */

		req = NULL;
		ret = wait_event_interruptible_timeout(audio->write_wq, (req = audio_req_get(audio,&audio->idle_reqs)) || (audio->mic_disconn), usecs_to_jiffies(2000));
		if (audio->mic_disconn) {
			if(req)
				audio_req_put(audio, &audio->idle_reqs, req);
			break;
		}
		if (req) {
			req_null_cnt = 0;
			len = r;
			if(len > IN_EP_ACTUAL_SIZE)
				len = IN_EP_ACTUAL_SIZE;
			if(copy_from_user(req->buf, buf + offset, len)){
				audio_req_put(audio,&audio->idle_reqs,req);
				r = -EFAULT;
				break;
			}
			req->length = len;
			spin_lock_irqsave(&audio->audio_list_lock,flags);
			if (audio_is_req_empty(audio,&audio->audio_data_list) && audio->cur_req == NULL) {
				audio->cur_req = req;
				spin_unlock_irqrestore(&audio->audio_list_lock,flags);

				ret = usb_ep_queue(audio->in_ep,req,GFP_ATOMIC);
				if (ret < 0) {
					pr_err("usb_ep_queue failed ret : %d\n",ret);
					if (audio->mic_disconn) {
						req->length = 0;
						audio_req_put(audio, &audio->idle_reqs, req);

					}
					break;
				}
			} else {
				audio_req_put(audio,&audio->audio_data_list,req);
				spin_unlock_irqrestore(&audio->audio_list_lock,flags);
			}
		} else {
			req_null_cnt++;
			if (req_null_cnt > 5) {
				audio->cur_req = NULL;
				req = audio_req_get(audio, &audio->audio_data_list);
				while (req) {
					memset(req->buf, 0, req->length);
					req->length = 0;
					audio_req_put(audio, &audio->idle_reqs, req);
					req = audio_req_get(audio, &audio->audio_data_list);
				}
				usb_ep_disable(audio->in_ep);
				usb_ep_enable(audio->in_ep);
			}
		}
		r -= len;
		offset += len;
	}
	gaudio_unlock(&audio->write_excl);
	return count - r;
}

static ssize_t gaudio_read(struct file *fp, char __user *buf, size_t count, loff_t *ops)
{
	int len = 0;
	struct f_audio *audio = fp->private_data;

	node_t *node = NULL;
	node = get_use_node(audio->play_frame_list);
	if (!node)
		return 0;
	if (count < node->size)
		node->size = count;
	copy_to_user(buf, node->data, node->size);
	len = node->size;
	/* read_count += len; */
	put_free_node(audio->play_frame_list, node);
	return len;
}

static unsigned int gaudio_poll(struct file *file, poll_table *wait)
{
	struct f_audio * audio  = file->private_data;
	unsigned int	ret = 0;

	poll_wait(file, &audio->event_queue, wait);

	if (!list_empty(&audio->event_list))
		ret |= POLLIN | POLLRDNORM;

	return ret;
}

typedef struct _gaudio_fifo_list_info {
	unsigned int cnt;
	unsigned int size;
	void *base;
	unsigned int data;
} gaudio_fifo_list_info_t;


static int gaudio_dequeue_buf(node_list_t *list, unsigned long arg, int direction)
{

	int ret = 0;
	node_t *node = NULL;
	gaudio_fifo_list_info_t fifo_info = {0};

	node = get_use_node(list);
	if (!node)
		return 0;
	if (node->size <= 0) {
		put_free_node(list, node);
		return 0;
	}
	fifo_info.size = node->size;
	fifo_info.data = node->data_phy;
	fifo_info.base = node;
	/* printk("dequeue base:%x\n", node); */
	ret = copy_to_user((void *)arg, &fifo_info, sizeof(gaudio_fifo_list_info_t));
	if (ret) {
		printk("(%s) ERROR: copy to user error!\n", __func__);
		return -EINVAL;
	}
	/* put_free_node(list, node); */
	return node->size;
}

static int gaudio_queue_buf(node_list_t *list, unsigned long arg, int direction)
{

	int ret = 0;
	node_t *node = NULL;
	gaudio_fifo_list_info_t fifo_info = {0};

	ret = copy_from_user(&fifo_info, (void *)arg, sizeof(gaudio_fifo_list_info_t));
	if (ret) {
		printk("(%s) ERROR: copy from user failed!\n", __func__);
		return -EINVAL;
	}
	node = (node_t *)fifo_info.base;
	/* printk("queue base:%x\n", node); */
	put_free_node(list, node);
	return 0;
}

static int gaudio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int status = 0;
	struct f_audio * audio  = file->private_data;
	struct gaudio_event *g_event = (struct gaudio_event *) arg;
	struct f_audio_event_list *e_list;
	struct usb_audio_control *event_con;
	unsigned long flags;

	switch (cmd)
	{
	case GAUDIO_IOCTL_GET_EVENT:
		spin_lock_irqsave(&audio->event_lock, flags);
		if (list_empty(&audio->event_list)) {
			status = -1;
			break;
		}
		e_list = list_first_entry(&audio->event_list, struct f_audio_event_list, list);
		event_con = e_list->event_con;

		if (event_con) {
			if (event_con == &spk_mute_control) {
				g_event->type = UAC_EVENT_SET_SPK_MUTE;
			} else if (event_con == &mic_mute_control) {
				g_event->type = UAC_EVENT_SET_MIC_MUTE;
			} else if (event_con == &spk_volume_control) {
				g_event->type = UAC_EVENT_SET_SPK_VOLUME;
			} else if (event_con == &mic_volume_control) {
				g_event->type = UAC_EVENT_SET_MIC_VOLUME;
			} else
				status = -1;
			g_event->value = event_con->get(event_con, UAC__CUR);
		} else {
			/* status = -1; */
			g_event->type = e_list->event;
		}
		list_del(&e_list->list);
		kfree(e_list);
		spin_unlock_irqrestore(&audio->event_lock, flags);
		break;

	case GAUDIO_IOCTL_GET_MMAP_INFO: {
#ifdef CONFIG_GADGET_UAC1_PLAY
		if (speak_enable) {
			gaudio_fifo_list_info_t fifo_info = {
				.cnt	= AUDIO_NODE_NUM,
				.size 	= audio_buf_size,
				.base	= audio->play_frame_list->data_phy,
			};
			status = copy_to_user((void *)arg, (void *)&fifo_info, sizeof(gaudio_fifo_list_info_t));
			if (status) {
				printk("(%s) ERROR: copy tx video buf info to user!\n", __func__);
				status  = -EINVAL;
			}

		}
#endif
		break;

	 }
	case GAUDIO_IOCTL_DEQUEUE_BUF:
#ifdef CONFIG_GADGET_UAC1_PLAY
		if (speak_enable) {
			return gaudio_dequeue_buf(audio->play_frame_list, arg, 0);
		 }
#endif
		break;
	case GAUDIO_IOCTL_QUEUE_BUF:
#ifdef CONFIG_GADGET_UAC1_PLAY
		if (speak_enable) {
			return gaudio_queue_buf(audio->play_frame_list, arg, 0);
		}
#endif
		break;
	default:
		break;
	}
	return status;
}

static int gaudio_mmap(struct file *file, struct vm_area_struct *vma)
{
	vma->vm_flags |= VM_IO;
	pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
	pgprot_val(vma->vm_page_prot) |= _CACHE_CACHABLE_NONCOHERENT;

	if (io_remap_pfn_range(vma,vma->vm_start,
				   vma->vm_pgoff,
				   vma->vm_end - vma->vm_start,
				   vma->vm_page_prot))
		return -EAGAIN;
	return 0;
}

static struct file_operations gaudio_fops={
	.owner = THIS_MODULE,
	.write = gaudio_write,
	.read  = gaudio_read,
	.open = gaudio_open,
	.release = gaudio_release,
	.poll = gaudio_poll,
	.unlocked_ioctl = gaudio_ioctl,
	.mmap = gaudio_mmap,

};

static struct miscdevice gaudio_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gaudio",
	.fops = &gaudio_fops,
};

/* static int g_test_audio_complete_count = 0; */
static void audio_data_complete(struct usb_ep *ep, struct usb_request * req)
{
	struct f_audio		*audio = req->context;
	unsigned int flags;

	/* g_test_audio_complete_count++; */
	/* if (0 == g_test_audio_complete_count%100) */
	/* 	printk(KERN_DEBUG "g_test_audio_complete_count = %d\n", g_test_audio_complete_count); */
	spin_lock_irqsave(&audio->audio_list_lock,flags);
	audio_req_put(audio, &audio->idle_reqs,req);
	req = audio_req_get(audio,&audio->audio_data_list);
	audio->cur_req = req;
	spin_unlock_irqrestore(&audio->audio_list_lock,flags);
	if(req)
	{
		if(usb_ep_queue(audio->in_ep, req, GFP_ATOMIC) < 0)
		{
			pr_err("usb_ep_queue error audio->mic_disconn =%d!\n",audio->mic_disconn);
			if(audio->mic_disconn)
			{
				pr_err("usb ep disconnect!!!!");
				while(req)
				{
					memset(req->buf, 0, req->length);
					req->length = 0;
					audio_req_put(audio, &audio->idle_reqs, req);
					req = audio_req_get(audio, &audio->audio_data_list);
				}
				audio->cur_req = NULL;
			}
		}
	}
	audio->cur_req = NULL;
	wake_up(&audio->write_wq);

	return;

}

static struct usb_request *audio_request_new(struct usb_ep *ep, int buffer_size)
{
	struct usb_request *req = usb_ep_alloc_request(ep, GFP_KERNEL);
	if (!req)
		return NULL;

	req->buf = kmalloc(buffer_size, GFP_KERNEL);
	if (!req->buf) {
		usb_ep_free_request(ep, req);
		return NULL;
	}
	req->length = buffer_size;
	return req;
}
#endif



#ifdef CONFIG_GADGET_UAC1_CAP_MIC
#define RECORD_PERIOD_SIZE  256
static void capture_complete(struct usb_ep *ep,
		struct usb_request *req)
{
	struct f_audio *audio = req->context;


	if (req->status != -ESHUTDOWN)
	{
		spin_lock(&audio->cb_list_lock);

		if (!list_empty(&audio->cb_list_queued))
		{
			list_move_tail(audio->cb_list_queued.next, &audio->cb_list_free);
		}

		spin_unlock(&audio->cb_list_lock);
	}
	return 0;
}




static struct cb_node* alloc_cb_node(struct usb_ep *ep, size_t size, void *context)
{
	struct cb_node *cb_node;

	cb_node = kmalloc(sizeof(struct cb_node), GFP_ATOMIC);
	cb_node->req = usb_ep_alloc_request(ep, GFP_ATOMIC);


	cb_node->req->buf = kmalloc(size, GFP_ATOMIC);
	cb_node->req->length = size;
	cb_node->req->complete = capture_complete;
	cb_node->req->context = context;
	INIT_LIST_HEAD(&cb_node->node);

	return cb_node;
}



static int wait_for_capture(struct f_audio *audio)
{
	while(!kthread_should_stop()) {
		if (audio->in_online)
			return 0;

		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		__set_current_state(TASK_RUNNING);
	}

	return -1;
}



/* audio capture thread */

static int audio_capture_thread(void *data)
{

	struct f_audio *audio =  data;
	struct cb_node *cb_node;

	int ret = 0;

	unsigned int flag_cbnode = 0;
	unsigned long flags;
	mm_segment_t old_fs;


	old_fs = get_fs();
	set_fs(KERNEL_DS);
	while (!wait_for_capture(audio)) {


		flag_cbnode = 1;

		spin_lock_irqsave(&audio->cb_list_lock, flags);
		if (!list_empty(&audio->cb_list_free)) {

			cb_node = list_first_entry(&audio->cb_list_free , struct cb_node, node);
			list_del(&cb_node->node);
			spin_unlock_irqrestore (&audio->cb_list_lock, flags);
			flag_cbnode = 1;

		} else {
			spin_unlock_irqrestore (&audio->cb_list_lock, flags);
			cb_node = alloc_cb_node(audio->in_ep, RECORD_PERIOD_SIZE, audio);
			flag_cbnode = 0;


		}



		ret = u_audio_capture(&audio->card, cb_node->req->buf, cb_node->req->length);
		if (ret < 0) {
			int sleep_us = (cb_node->req->length * 1000000 /
					(u_audio_get_capture_channels(&audio->card) * 2) /
					u_audio_get_capture_rate(&audio->card));

			usleep_range(sleep_us - (sleep_us/10), sleep_us);

			if(!flag_cbnode) {
				kfree(cb_node->req->buf);
				usb_ep_free_request(audio->in_ep , cb_node->req);
				kfree(cb_node);
			}

		}else{

			spin_lock_irqsave (&audio->cb_list_lock, flags);
			usb_ep_queue(audio->in_ep, cb_node->req, GFP_KERNEL);
			list_add_tail(&cb_node->node, &audio->cb_list_queued);
			spin_unlock_irqrestore (&audio->cb_list_lock, flags);
		}
	}
	set_fs(old_fs);


}
#endif



/* audio function driver setup/binding */
static int
f_audio_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_audio		*audio = func_to_audio(f);
	int			status, i;
	struct usb_ep		*ep = NULL;

	usb_function_deactivate(f);
	f_audio_build_desc(audio);

	/* allocate instance-specific interface IDs, and patch descriptors */
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	ac_interface_desc.bInterfaceNumber = status;
	uac_iad.bFirstInterface = status;
	uac_iad.bInterfaceCount = 1;

#if defined(CONFIG_GADGET_UAC1_PLAY) && defined(CONFIG_GADGET_UAC1_CAP)
	if (speak_enable) {
		status = usb_interface_id(c, f);
		if (status < 0)
			goto fail;
		spk_as_interface_alt_0_desc.bInterfaceNumber = status;
		spk_as_interface_alt_1_desc.bInterfaceNumber = status;
		ac_header_desc.baInterfaceNr[0] = status;
		play_intf = status;
		uac_iad.bInterfaceCount += 1;

		status = usb_interface_id(c, f);
		if (status < 0)
			goto fail;
		mic_as_interface_alt_0_desc.bInterfaceNumber = status;
		mic_as_interface_alt_1_desc.bInterfaceNumber = status;
		ac_header_desc.baInterfaceNr[1] = status;
		cap_intf = status;
		uac_iad.bInterfaceCount += 1;
	} else {
		status = usb_interface_id(c, f);
		if (status < 0)
			goto fail;
		mic_as_interface_alt_0_desc.bInterfaceNumber = status;
		mic_as_interface_alt_1_desc.bInterfaceNumber = status;
		ac_header_desc.baInterfaceNr[0] = status;
		cap_intf = status;
		uac_iad.bInterfaceCount += 1;
	}


#elif defined(CONFIG_GADGET_UAC1_PLAY)
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	spk_as_interface_alt_0_desc.bInterfaceNumber = status;
	spk_as_interface_alt_1_desc.bInterfaceNumber = status;
	ac_header_desc.baInterfaceNr[0] = status;
	play_intf = status;
	uac_iad.bInterfaceCount += 1;
#elif defined(CONFIG_GADGET_UAC1_CAP)
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	mic_as_interface_alt_0_desc.bInterfaceNumber = status;
	mic_as_interface_alt_1_desc.bInterfaceNumber = status;
	ac_header_desc.baInterfaceNr[0] = status;
	cap_intf = status;
	uac_iad.bInterfaceCount += 1;
#else
#error cap or paly should be defined
#endif
	status = -ENODEV;

#ifdef CONFIG_GADGET_UAC1_PLAY
	/* allocate instance-specific endpoints */
	if (speak_enable) {
		ep = usb_ep_autoconfig(cdev->gadget, &as_out_ep_desc);
		if (!ep)
			goto fail;
		audio->out_ep = ep;
		audio->out_ep->desc = &as_out_ep_desc;
		ep->driver_data = cdev;	/* claim */
	}
#endif

#ifdef CONFIG_GADGET_UAC1_CAP
	ep = usb_ep_autoconfig(cdev->gadget, &as_in_ep_desc);
	if (!ep)
		goto fail;
	audio->in_ep = ep;
	audio->in_ep->desc = &as_in_ep_desc;
	ep->driver_data = cdev;	/* claim */
#endif


	status = -ENOMEM;

	/* copy descriptors, and track endpoint copies */
	status = usb_assign_descriptors(f, f_audio_desc, f_audio_desc, NULL);
	if (status)
		goto fail;
	usb_function_activate(f);

	atomic_set(&audio->interface_conn, 1);

	init_waitqueue_head(&audio->event_queue);
	INIT_LIST_HEAD(&audio->event_list);

	__audio = audio;
#ifdef CONFIG_GADGET_UAC1_CAP_USER
	init_waitqueue_head(&audio->read_wq);
	init_waitqueue_head(&audio->write_wq);
	init_waitqueue_head(&audio->online_wq);
	atomic_set(&audio->open_excl,0);
	atomic_set(&audio->read_excl,0);
	atomic_set(&audio->write_excl,0);
	INIT_LIST_HEAD(&audio->idle_reqs);
	INIT_LIST_HEAD(&audio->audio_data_list);
	spin_lock_init(&audio->audio_list_lock);

	status = misc_register(&gaudio_device);
	if(status < 0)
	    goto fail;

	for(i = 0,status = 0;i < IN_EP_REQ_COUNT && status == 0 ; i++)
	{
		struct usb_request *req = audio_request_new(audio->in_ep,IN_EP_MAX_PACKET_SIZE);
		if(req){
			req->context = audio;
			req->complete = audio_data_complete;
			audio_req_put(audio,&audio->idle_reqs,req);
		}else
		    status = -ENOMEM;
	}
	audio->cur_req = NULL;
	if(status == -ENOMEM)
		goto fail;
	//audio->in_online = 1;
#endif

#ifdef CONFIG_GADGET_UAC1_CAP_MIC
	spin_lock_init(&audio->cb_list_lock);
	audio->capture_thread = kthread_create(audio_capture_thread, audio, "usb-audio-capture");
	if (IS_ERR(audio->capture_thread))
	{
		return PTR_ERR(audio->capture_thread);
	}

	INIT_LIST_HEAD(&audio->cb_list_free);
	INIT_LIST_HEAD(&audio->cb_list_queued);
#endif
	return 0;

fail:
	if (ep)
		ep->driver_data = NULL;
	return status;
}

static void
f_audio_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_audio		*audio = func_to_audio(f);
	struct usb_request *req = NULL;
	int i = 0;
#ifdef CONFIG_GADGET_UAC1_CAP_USER
	/*******in****/
	misc_deregister(&gaudio_device);
	for(i = 0;i < IN_EP_REQ_COUNT; i++)
	{
		if(!list_empty(&audio->idle_reqs))
		{
			req = audio_req_get(audio,&audio->idle_reqs);
			kfree(req->buf);
			usb_ep_free_request(audio->out_ep,req);
		}
		else
			break;
	}
#endif
	usb_free_all_descriptors(f);
	kfree(audio);
}

/*-------------------------------------------------------------------------*/

static int generic_set_cmd(struct usb_audio_control *con, u8 cmd, int value)
{
	con->data[cmd] = value;
	return 0;
}

static int generic_get_cmd(struct usb_audio_control *con, u8 cmd)
{
	return con->data[cmd];
}

/* Todo: add more control selecotor dynamically */
int  control_selector_init(struct f_audio *audio)
{
	INIT_LIST_HEAD(&audio->cs);
#ifdef CONFIG_GADGET_UAC1_PLAY
	if (speak_enable) {
		list_add(&spk_feature_unit.list, &audio->cs);
		INIT_LIST_HEAD(&spk_feature_unit.control);
	}
#endif
#ifdef CONFIG_GADGET_UAC1_CAP
	list_add(&mic_feature_unit.list, &audio->cs);
	INIT_LIST_HEAD(&mic_feature_unit.control);
#endif

#ifdef CONFIG_GADGET_UAC1_PLAY
	if (speak_enable) {
		list_add(&spk_mute_control.list, &spk_feature_unit.control);
		list_add(&spk_volume_control.list, &spk_feature_unit.control);
	}
#endif

#ifdef CONFIG_GADGET_UAC1_CAP
	list_add(&mic_mute_control.list, &mic_feature_unit.control);
	list_add(&mic_volume_control.list, &mic_feature_unit.control);
#endif

#ifdef CONFIG_GADGET_UAC1_PLAY
	/* if (volume_is_set == 0) */
	if (speak_enable) {
		spk_volume_control.data[UAC__CUR] = 0x0000;
		spk_volume_control.data[UAC__MIN] = 0xE400;
		spk_volume_control.data[UAC__MAX] = 0x0C00;
		spk_volume_control.data[UAC__RES] = 0x0200;
	}
#endif

#ifdef CONFIG_GADGET_UAC1_CAP
	/* if (volume_is_set == 0) */
	/*mic_volume_control.data[UAC__CUR] = 0x0032; */
	mic_volume_control.data[UAC__MIN] = 0x0000;
	mic_volume_control.data[UAC__MAX] = 0x0064;
	mic_volume_control.data[UAC__RES] = 0x0001;
#endif
	return 0;
}

/**
 * audio_bind_config - add USB audio function to a configuration
 * @c: the configuration to supcard the USB audio function
 * Context: single threaded during gadget setup
 *
 * Returns zero on success, else negative errno.
 */
int audio_bind_config(struct usb_configuration *c)
{
	struct f_audio *audio;
	int status;
	struct usb_ep *ep;

	/* allocate and initialize one new instance */
	audio = kzalloc(sizeof *audio, GFP_KERNEL);
	if (!audio)
		return -ENOMEM;

	audio->card.func.name = "audio";
	audio->card.gadget = c->cdev->gadget;

#ifdef CONFIG_GADGET_UAC1_PLAY
	if (speak_enable) {
		tasklet_init(&putuse_work_t, putuse_work, 0);
		putuse_work_t.data = (unsigned long)audio;
		INIT_LIST_HEAD(&audio->play_queue);
		audio->play_frame_list = init_node_list(AUDIO_NODE_NUM, audio_buf_size, NL_NO_BLOCK, NL_NO_BLOCK, NL_KEEPUPDATE);
		if (audio->play_frame_list == NULL) {
			printk("UAC:Init node list Failed\n");
			return -ENOMEM;
		}

	}
#endif
	spin_lock_init(&audio->lock);
	spin_lock_init(&audio->event_lock);

#ifdef CONFIG_GADGET_UAC1_CAP_MIC
	/* set up ASLA audio devices */
	status = gaudio_setup(audio);
	if (status < 0)
		return status;
#endif
	if( audio_strings_def[0].id == 0 )
		status = usb_string_ids_tab(c->cdev, audio_strings_def);
	if(status) {
		printk("UAC:Allocating String ID Failed\n");
		return ;
	}
	uac_iad.iFunction = audio_strings_def[0].id;
	ac_interface_desc.iInterface = audio_strings_def[0].id;
	/* status = audio_strings_def[1].id; */
	/* spk_as_interface_alt_0_desc.iInterface = status; */
	/* spk_as_interface_alt_1_desc.iInterface = status; */
	/* status = audio_strings_def[2].id; */
	/* mic_as_interface_alt_0_desc.iInterface = status; */
	/* mic_as_interface_alt_1_desc.iInterface = status; */

	audio->card.func.strings = uac_strings;
	audio->card.func.bind = f_audio_bind;
	audio->card.func.unbind = f_audio_unbind;
	audio->card.func.set_alt = f_audio_set_alt;
	audio->card.func.setup = f_audio_setup;
	audio->card.func.disable = f_audio_disable;

	control_selector_init(audio);

#ifdef CONFIG_GADGET_UAC1_PLAY
	if (speak_enable)
		INIT_WORK(&audio->playback_work, f_audio_playback_work);
#endif

	status = usb_add_function(c, &audio->card.func);
	if (status)
		return status;

	/* INFO(c->cdev, "audio_buf_size %d, req_buf_size %d, req_count %d\n", */
	/* 	audio_buf_size, req_buf_size, req_count); */

	return 0;

}

int uac_set_audio_param(struct audio_param *a_param)
{
	int cnt, ret = 0;
	int ndesc = 12;

	mic_volume_control.data[UAC__CUR] = a_param->mic_volume;
	spk_volume_control.data[UAC__CUR] = a_param->spk_volume;
	/* printk("mic_volume:0x%x spk_volume:0x%x\n", mic_volume_control.data[UAC__CUR], spk_volume_control.data[UAC__CUR]); */
	/* volume_is_set = 1; */
	speak_enable = a_param->speak_en;
	if (speak_enable)
		F_AUDIO_NUM_INTERFACES = 2;
	UAC_DT_AC_HEADER_LENGTH = UAC_DT_AC_HEADER_SIZE(F_AUDIO_NUM_INTERFACES);
	UAC_DT_TOTAL_LENGTH = UAC_DT_AC_HEADER_LENGTH +
				UAC_DT_INPUT_TERMINAL_SIZE * F_AUDIO_NUM_INTERFACES +
				UAC_DT_OUTPUT_TERMINAL_SIZE * F_AUDIO_NUM_INTERFACES +
				UAC_DT_FEATURE_UNIT_SIZE(0) * F_AUDIO_NUM_INTERFACES;

	ac_header_desc.bLength = UAC_DT_AC_HEADER_LENGTH;
	ac_header_desc.wTotalLength = UAC_DT_TOTAL_LENGTH;
	ac_header_desc.bInCollection = F_AUDIO_NUM_INTERFACES;
	if (speak_enable) {
		mic_input_terminal_desc.wTerminalType = 0x405;
		spk_output_terminal_desc.wTerminalType = 0x405;
		ndesc += 9;
	}
	f_audio_desc = kmalloc((ndesc+1)*sizeof(struct usb_descriptor_header *), GFP_KERNEL);
	if (!f_audio_desc) {
		printk("error(%s, %d): alloc audio descriptors buf failed!\n", __func__, __LINE__);
		return -1;
	}
	cnt = 0;
	f_audio_desc[cnt] = &uac_iad;
	cnt++;
	f_audio_desc[cnt] = &ac_interface_desc;
	cnt++;
	f_audio_desc[cnt] = &ac_header_desc;
	if (speak_enable) {
		cnt++;
		f_audio_desc[cnt] = &spk_input_terminal_desc;
	}
	cnt++;
	f_audio_desc[cnt] = &mic_input_terminal_desc;
	if (speak_enable) {
		cnt++;
		f_audio_desc[cnt] = &spk_output_terminal_desc;
	}
	cnt++;
	f_audio_desc[cnt] = &mic_output_terminal_desc;
	if (speak_enable) {
		cnt++;
		f_audio_desc[cnt] = &spk_feature_unit_desc;
	}
	cnt++;
	f_audio_desc[cnt] = &mic_feature_unit_desc;
	if (speak_enable) {
		cnt++;
		f_audio_desc[cnt] = &spk_as_interface_alt_0_desc;
		cnt++;
		f_audio_desc[cnt] = &spk_as_interface_alt_1_desc;
		cnt++;
		f_audio_desc[cnt] = &spk_as_header_desc;
		cnt++;
		f_audio_desc[cnt] = &spk_as_type_i_desc;
		cnt++;
		f_audio_desc[cnt] = &as_out_ep_desc;
		cnt++;
		f_audio_desc[cnt] = &as_iso_out_desc;
	}
	cnt++;
	f_audio_desc[cnt] = &mic_as_interface_alt_0_desc;
	cnt++;
	f_audio_desc[cnt] = &mic_as_interface_alt_1_desc;
	cnt++;
	f_audio_desc[cnt] = &mic_as_header_desc;
	cnt++;
	f_audio_desc[cnt] = &mic_as_type_i_desc;
	cnt++;
	f_audio_desc[cnt] = &as_in_ep_desc;
	cnt++;
	f_audio_desc[cnt] = &as_iso_in_desc;
	cnt++;
	f_audio_desc[cnt] = NULL;



	return 0;
}

int uac_name_set(char *name)
{
	audio_strings_def[0].s = (const char *)name;
	return 0;
}
EXPORT_SYMBOL(uac_name_set);

int ucam_bind_uac(struct usb_configuration *c)
{
	return audio_bind_config(c);
}
EXPORT_SYMBOL(ucam_bind_uac);

int uac_set_descriptor(unsigned long arg)
{
	int ret = 0;
	struct audio_param *a_param = NULL;

	a_param = kmalloc(sizeof(struct audio_param), GFP_KERNEL);
	ret = copy_from_user(a_param, (void __user*)arg, sizeof(struct audio_param));
	if (ret) {
		printk("error(%s %d):failed to copy audio param!\n", __func__, __LINE__);
		return -EIO;
	}

	uac_set_audio_param(a_param);
	return 0;
}
EXPORT_SYMBOL(uac_set_descriptor);
