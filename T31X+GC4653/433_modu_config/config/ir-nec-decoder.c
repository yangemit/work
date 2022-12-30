/* ir-nec-decoder.c - handle NEC IR Pulse/Space protocol
 *
 * Copyright (C) 2010 by Mauro Carvalho Chehab <mchehab@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <linux/bitrev.h>
#include <linux/module.h>
#include "rc-core-priv.h"

#define NEC_NBITS		32
#define NEC_UNIT		18750             /* 18.75us  *2 = 37.5 us  /2 = 9.3 */
#define NEC_HEADER_PULSE	(16 * NEC_UNIT)   /* 300us 数据头开始高电平的标准时间 */
//#define NECX_HEADER_PULSE	(8  * NEC_UNIT) /* 4.5 ms Less common NEC variant */
#define NEC_HEADER_SPACE	(210  * NEC_UNIT) /* 3.93 ms 数据头开始低电平的标准时间 */
//#define NEC_REPEAT_SPACE	(4  * NEC_UNIT) /* 2.25ms */
#define NEC_BIT_PULSE_300	(16  * NEC_UNIT)  /* 300 us bit-0 数据高电平标准时间 */
#define NEC_BIT_PULSE_900	(48  * NEC_UNIT)  /* 900 us bit-1 数据高电平标准时间 */
#define NEC_BIT_0_SPACE		(48  * NEC_UNIT)  /* 900 us bit-0 数据低电平标准时间 */
#define NEC_BIT_1_SPACE		(16  * NEC_UNIT)  /* 300 us bit-1 数据低电平标准时间 */
//#define	NEC_TRAILER_PULSE	(1  * NEC_UNIT)  
#define	NEC_TRAILER_SPACE	(10 * NEC_UNIT) /* even longer in reality 后期配置 */
#define NECX_REPEAT_BITS	1

enum nec_state {
	STATE_INACTIVE,
	STATE_HEADER_SPACE,
	STATE_BIT_PULSE,
	STATE_BIT_SPACE,
	STATE_TRAILER_PULSE,
	STATE_TRAILER_SPACE,
};

static int bit_flag = 0; 
 
/**
 * ir_nec_decode() - Decode one NEC pulse or space
 * @dev:	the struct rc_dev descriptor of the device
 * @duration:	the struct ir_raw_event descriptor of the pulse/space
 *
 * This function returns -EINVAL if the pulse violates the state machine
 */
static int ir_nec_decode(struct rc_dev *dev, struct ir_raw_event ev)
{
	struct nec_dec *data = &dev->raw->nec;
	u32 scancode;
	u8 address, not_address, command, not_command;
	bool send_32bits = false;

	if (!(dev->enabled_protocols & RC_BIT_NEC))
		return 0;

	if (!is_timing_event(ev)) {
		if (ev.reset)
			data->state = STATE_INACTIVE;
		return 0;
	}

	IR_dprintk(2, "NEC decode started at state %d (%uus %s)\n",
		   data->state, TO_US(ev.duration), TO_STR(ev.pulse));

	//printk("[INFO] switch %d \n",data->state);
	switch (data->state) {

	case STATE_INACTIVE:
		/*
			gpio值为1代表高脉冲
		*/
		if (!ev.pulse)
			break;
		/*
			300 - 18.75 * 5 < 头脉冲时间-高 < 300 + 18.75 * 5
		*/
		if (eq_margin(ev.duration, NEC_HEADER_PULSE, NEC_UNIT * 5)) {
			/*data->is_nec_x = false;*/
			/*data->necx_repeat = false;*/
			//printk("[INFO] 300 us  %u \n",ev.duration);
		} else
			break;

		data->count = 0;
		data->state = STATE_HEADER_SPACE;
		return 0;

	case STATE_HEADER_SPACE:
		/*
			gpio 值为0代表低电平
		*/
		if (ev.pulse)
			break;
		/*
			3.93 + 0.468 ms < 头脉冲时间-低 < 3.93 + 0.468 ms
		*/
		//printk("[INFO] 4ms %d \n",ev.duration);
		if (eq_margin(ev.duration, NEC_HEADER_SPACE, NEC_UNIT * 25)) {
			data->state = STATE_BIT_PULSE;
			dev->keypressed = true;
			//printk("[INFO] 4ms %d \n",ev.duration);
			return 0;
			//} else {
			// 	bit_5--;
			//	break;
			//}
		} 
		/*else if (eq_margin(ev.duration, NEC_REPEAT_SPACE, NEC_UNIT / 2)) {*/
			/*if (!dev->keypressed) {*/
				/*IR_dprintk(1, "Discarding last key repeat: event after key up\n");*/
			/*} else {*/
				/*rc_repeat(dev);*/
				/*IR_dprintk(1, "Repeat last key\n");*/
				/*data->state = STATE_TRAILER_PULSE;*/
			/*}*/
			/*return 0;*/
		/*}*/

		break;

	case STATE_BIT_PULSE:
		/*
			bit-高
		*/
		if (!ev.pulse)
			break;

		/*
			bit0-头：300 - 93.7 ~ 300 + 93.7
		*/
		if (eq_margin(ev.duration, NEC_BIT_PULSE_300, NEC_UNIT * 5))
		{
			bit_flag = 0;
		} else if (eq_margin(ev.duration,NEC_BIT_PULSE_900,NEC_UNIT * 5)) {
			/*
				bit1-头：900 - 93.7 ~ 900 + 93.7
			*/
			bit_flag = 1;
		} else {
			//printk("[INFO] bit head failed %d \n",ev.duration);
			break;
		}
			//printk("[INFO] bit head Success %d \n",ev.duration);

		data->state = STATE_BIT_SPACE;
		return 0;

	case STATE_BIT_SPACE:
		/*
			bit-低
		*/
		if (ev.pulse)
			break;

		if (data->necx_repeat && data->count == NECX_REPEAT_BITS &&
			geq_margin(ev.duration,
			NEC_TRAILER_SPACE, NEC_UNIT / 2)) {
				IR_dprintk(1, "Repeat last key\n");
				rc_repeat(dev);
				data->state = STATE_INACTIVE;
				return 0;

		} else if (data->count > NECX_REPEAT_BITS)
			data->necx_repeat = false;
		/*
			data->bits 每调一次左移位
		*/
		data->bits <<= 1;
		/*
			bit1-尾 ：900 - 93.7 ~ 900 + 93.7 && bit_flag=1
		*/
		if (eq_margin(ev.duration, NEC_BIT_1_SPACE, NEC_UNIT * 5 ) && bit_flag)
			data->bits |= 1;
		else if (!eq_margin(ev.duration, NEC_BIT_0_SPACE, NEC_UNIT * 5))
			break;
		data->count++;
		
		
		//printk("[INFO %d] bit wei %d \n",data->count,ev.duration );
		
		if (data->count == NEC_NBITS) {
			data->state = STATE_TRAILER_PULSE;
			//printk("[INFO] bit 24 %x \n",bitrev8((data->bits >> 24) & 0xff));
			//printk("[INFO] bit 16 %x \n",bitrev8((data->bits >> 16) & 0xff));
			//printk("[INFO] bit 8 %x \n",bitrev8((data->bits >> 8) & 0xff));
			//printk("[INFO] bit 0 %x \n",bitrev8((data->bits >> 0) & 0xff));
		}
		else
			data->state = STATE_BIT_PULSE;

		return 0;

	case STATE_TRAILER_PULSE:
		if (!ev.pulse)
			break;
		//printk("[INFO] TRAILER %d \n",ev.duration);
		/*if (!eq_margin(ev.duration, NEC_TRAILER_PULSE, NEC_UNIT / 2))*/
			/*break;*/

		data->state = STATE_TRAILER_SPACE;
		return 0;

	case STATE_TRAILER_SPACE:
		if (ev.pulse)
			break;

		if (!geq_margin(ev.duration, NEC_TRAILER_SPACE, NEC_UNIT / 2))
			break;

		/*
			将得到的32bit 编码进行分解
			433 模块
			command = not_command = 按键值
			not_address << 8 | address = 设备ID = 0xdf01
		*/
		command     = bitrev8((data->bits >> 24) & 0xff);
		not_command = bitrev8((data->bits >> 16) & 0xff);
		address	    = bitrev8((data->bits >>  8) & 0xff);
		not_address = bitrev8((data->bits >>  0) & 0xff);

		if ((command != not_command)) {
			IR_dprintk(1, "NEC checksum error: received 0x%08x\n",
				   data->bits);
			send_32bits = true;
		}

		if (send_32bits) {
			/* NEC transport, but modified protocol, used by at
			 * least Apple and TiVo remotes */
			scancode = data->bits;
			IR_dprintk(1, "NEC (modified) scancode 0x%08x\n", scancode);
		} else if ((address == not_address) ) {
			/* Extended NEC */
			scancode = address     << 16 |
				   not_address <<  8 |
				   command;
			IR_dprintk(1, "NEC (Ext) scancode 0x%06x\n", scancode);
		} else {
			/* Normal NEC */
			scancode = command;
			/*
				当按键按下时，会多申报一个input event事件----且value=0
				与我设置的 0 键值中途，所有将其改变为 0xa0 与 rc-map 里的
				文件键值映射对应
			*/
			if( scancode == 0) {
				scancode = 0xa0;
			}
			IR_dprintk(1, "NEC scancode 0x%04x\n", scancode);
		}
		
		//printk("[INFO] scancode %x \n",scancode);

		if (data->is_nec_x)
			data->necx_repeat = true;

		/*
			按键值的映射----将得到的command 与 映射文件进行匹配-----来上报按键事件
		*/
		rc_keydown(dev, scancode, 0);
		data->state = STATE_INACTIVE;
		return 0;
	}

	IR_dprintk(1, "NEC decode failed at count %d state %d (%uus %s)\n",
		   data->count, data->state, TO_US(ev.duration), TO_STR(ev.pulse));
	data->state = STATE_INACTIVE;
	return -EINVAL;
}

static struct ir_raw_handler nec_handler = {
	.protocols	= RC_BIT_NEC,
	.decode		= ir_nec_decode,
};

static int __init ir_nec_decode_init(void)
{
	ir_raw_handler_register(&nec_handler);

	printk(KERN_INFO "IR NEC protocol handler initialized\n");
	return 0;
}

static void __exit ir_nec_decode_exit(void)
{
	ir_raw_handler_unregister(&nec_handler);
}

module_init(ir_nec_decode_init);
module_exit(ir_nec_decode_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mauro Carvalho Chehab <mchehab@redhat.com>");
MODULE_AUTHOR("Red Hat Inc. (http://www.redhat.com)");
MODULE_DESCRIPTION("NEC IR protocol decoder");
