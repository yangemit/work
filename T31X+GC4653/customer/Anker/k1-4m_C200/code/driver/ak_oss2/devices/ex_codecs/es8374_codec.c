#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/seq_file.h>
#include <linux/miscdevice.h>
#include <linux/input-polldev.h>
#include <linux/input.h>
#include <linux/gfp.h>
#include <linux/proc_fs.h>
#include <linux/gpio.h>
#include <soc/gpio.h>
#include "common_codec.h"

#include "../xb47xx_i2s_v12.h"
#include "es8374_codec.h"

static struct snd_codec_data es8374_codec_data = {
	.codec_sys_clk = 0,
	.codec_dmic_clk = 0,
	/* volume */
	.replay_volume_base = 0,
	.record_volume_base = 0,
	.record_digital_volume_base = 23,
	.replay_digital_volume_base = 0,
	/* default route */
	.replay_def_route = {.route = SND_ROUTE_REPLAY_DACRL_TO_ALL,
					.gpio_hp_mute_stat = STATE_DISABLE,
					.gpio_spk_en_stat = STATE_ENABLE},
	.record_def_route = {.route = SND_ROUTE_RECORD_MIC1_AN1,
					.gpio_hp_mute_stat = STATE_DISABLE,
					.gpio_spk_en_stat = STATE_ENABLE},
	/* device <-> route map */
	.record_headset_mic_route = {.route = SND_ROUTE_RECORD_MIC1_SIN_AN2,
					.gpio_hp_mute_stat = STATE_DISABLE,
					.gpio_spk_en_stat = STATE_ENABLE},
	.record_buildin_mic_route = {.route = SND_ROUTE_RECORD_MIC1_AN1,
					.gpio_hp_mute_stat = STATE_DISABLE,
					.gpio_spk_en_stat = STATE_ENABLE},
	.replay_headset_route = {.route = SND_ROUTE_REPLAY_DACRL_TO_HPRL,
					.gpio_hp_mute_stat = STATE_DISABLE,
					.gpio_spk_en_stat = STATE_DISABLE},
	.replay_speaker_route = {.route = SND_ROUTE_REPLAY_DACRL_TO_ALL,
					.gpio_hp_mute_stat = STATE_DISABLE,
					.gpio_spk_en_stat = STATE_ENABLE},
	.replay_headset_and_speaker_route = {.route = SND_ROUTE_REPLAY_DACRL_TO_ALL,
						.gpio_hp_mute_stat = STATE_DISABLE,
						.gpio_spk_en_stat = STATE_ENABLE},
	/* linein route */
	.record_linein_route = {.route = SND_ROUTE_RECORD_LINEIN1_AN2_SIN_TO_ADCL_AND_LINEIN2_AN3_SIN_TO_ADCR,
						.gpio_hp_mute_stat = STATE_DISABLE,
						.gpio_spk_en_stat = STATE_ENABLE,
	},

	.record_linein1_route = {.route = SND_ROUTE_RECORD_LINEIN1_DIFF_AN2,
						.gpio_hp_mute_stat = STATE_DISABLE,
						.gpio_spk_en_stat = STATE_ENABLE,
	},
	.record_linein2_route = {.route = SND_ROUTE_RECORD_LINEIN2_SIN_AN3,
						.gpio_hp_mute_stat = STATE_DISABLE,
						.gpio_spk_en_stat = STATE_ENABLE,
	},
};

static struct snd_codec_data *es8374_platform_data = &es8374_codec_data;
struct codec_operation *es8374_codec_ope = NULL;
extern int i2s_register_codec_2(struct codec_info * codec_dev);

struct es8374_data {
	struct i2c_client *client;
	struct snd_codec_data *pdata;
	struct mutex lock_rw;
};

struct es8374_data *es8374_save = NULL;

int es8374_i2c_read(struct es8374_data *es, unsigned char reg)
{
	unsigned char value;
	struct i2c_client *client = es->client;
	struct i2c_msg msg[2] = {
		[0] = {
			.addr = client->addr,
			.flags = 0,
			.len = 1,
			.buf = &reg,
		},
		[1] = {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = 1,
			.buf = &value,
		},
	};
	int err;
	err = i2c_transfer(client->adapter, msg, 2);
	if (err < 0) {
		printk("error:(%s,%d), Read msg error\n", __func__, __LINE__);
		return err;
	}
	return value;

}

int es8374_i2c_write(struct es8374_data *es, unsigned char reg, unsigned char value)
{
	struct i2c_client *client = es->client;
	unsigned char buf[2] = {reg, value};
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.len = 2,
		.buf = buf,
	};
	int err;
	err = i2c_transfer(client->adapter, &msg, 1);
	if (err < 0) {
		printk("error:(%s,%d), Write msg error.\n",__func__,__LINE__);
		return err;
	}

	return 0;
}

int es8374_reg_set(unsigned char reg, int start, int end, int val)
{
	int ret;
	int i = 0, mask = 0;
	unsigned char oldv = 0, new = 0;
	for(i = 0; i < (end-start + 1); i++) {
		mask += (1 << (start + i));
	}
	oldv = es8374_i2c_read(es8374_save, reg);
	new = oldv & (~mask);
	new |= val << start;
	ret = es8374_i2c_write(es8374_save, reg, new);
	if(ret < 0) {
#ifdef ES8374_DEBUG
		printk("fun:%s,ES8374 I2C Write error.\n",__func__);
#endif
	}
	if(new != es8374_i2c_read(es8374_save, reg)) {
#ifdef ES8374_DEBUG
		printk("es8374 write error, reg = 0x%x, start = %d, end = %d, val = 0x%x\n", reg, start, end, val);
#endif
	}

	return ret;
}

void es8374_dac_mute(int mute)
{
	return 0;
}

#ifdef ES8374_DEBUG
int dump_es8374_codec_regs(void)
{
	return 0;
}
#endif

int es8374_codec_init(void)
{
	//printk("%s",__func__);
	return 0;
}

static int es8374_codec_turn_off(int mode)
{
	//printk("%s",__func__);
	return 0;
}

static int es8374_set_sample_rate(unsigned int rate)
{
	//printk("%s",__func__);
	return 0;
}

static int es8374_set_speaker(void)
{
	//printk("%s",__func__);
	return 0;
}

static int es8374_set_buildin_mic(void)
{
	//printk("%s",__func__);
	return 0;
}

void es8374_codec_set_play_volume(int * vol)
{
	//printk("%s",__func__);
	return;
}

static unsigned int cur_out_device = -1;
static int es8374_set_device(struct es8374_data *es, enum snd_device_t device)
{
	//printk("%s",__func__);
	return 0;
}

void es8374_codec_set_record_volume(int * vol)
{
	//printk("%s",__func__);
	return;
}

static int es8374_codec_ctl(struct codec_info *codec_dev, unsigned int cmd, unsigned long arg)
{
	//printk("%s",__func__);
	return 0;
}

static const struct i2c_device_id es8374_id[] = {
	{"es8374", 0},
	{ }
};

static int es8374_codec_register(struct platform_device *pdev)
{
	if(es8374_codec_ope) return 0;

	es8374_codec_ope = (struct codec_operation*) kzalloc(sizeof(struct codec_operation), GFP_KERNEL);
	if(!es8374_codec_ope) {
		printk("fun:%s,alloc es8374 codec mem failed,\n",__func__);
		return -ENOMEM;
	}
	pdev->dev.platform_data = es8374_platform_data;
	platform_set_drvdata(pdev, &es8374_codec_ope);

	printk("%s, probe() successful!\n",__func__);
	return 0;
}

static int es8374_codec_release(void)
{
	return 0;
}

static int codec_match(struct device *dev, void *data)
{
	struct platform_device *pdev = to_platform_device(dev);
	int ret = 0;

	if(!get_device(dev))
		return -ENODEV;
	if(!strncmp(pdev->name, "es8374-codec", sizeof("es8374-codec"))) {
		ret = es8374_codec_register(pdev);
	}

	put_device(dev);
	return ret;
}

#define T30_EXTERNAL_CODEC_CLOCK 12000000
int jz_es8374_init(void)
{
	int retval;
	struct codec_info *codec_dev;

	codec_dev = kzalloc(sizeof(struct codec_info), GFP_KERNEL);
	if(!codec_dev) {
		printk("error:(%s,%d),alloc codec device error\n",__func__,__LINE__);
		return -1;
	}

	sprintf(codec_dev->name, "i2s_external_codec");
	codec_dev->codec_ctl_2 = es8374_codec_ctl;
	codec_dev->codec_clk = T30_EXTERNAL_CODEC_CLOCK;
	codec_dev->codec_mode = CODEC_SLAVE;

	i2s_register_codec_2(codec_dev);

	retval = bus_for_each_dev(&platform_bus_type, NULL, NULL, codec_match);
	if(retval) {
		 printk("%s[%d]: Failed to register codec driver!\n",__func__,__LINE__);
		 return retval;
	}

	if(es8374_codec_ope) {
		es8374_codec_ope->priv = (void*)codec_dev;
	}

//	es8374_codec_init();
	return retval;
}

void jz_es8374_exit(void)
{
	struct codec_info *codec_dev;

	es8374_codec_release();
	codec_dev = es8374_codec_ope->priv;
	i2s_release_codec_2(codec_dev);

	kfree(codec_dev);
	kfree(es8374_codec_ope);
	es8374_codec_ope = NULL;
}

int es8374_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	struct es8374_data *es;

	if (!i2c_check_functionality(client->adapter,
		I2C_FUNC_SMBUS_BYTE | I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev, "client not i2c capable\n");
		ret = -ENODEV;
		goto err_dev;
	}

	es = kzalloc(sizeof(struct es8374_data), GFP_KERNEL);
	if (NULL == es) {
		dev_err(&client->dev, "failed to allocate memery for module data\n");
		ret = -ENOMEM;
		goto err_klloc;
	}

	es->client = client;
	i2c_set_clientdata(client, es);

	es8374_save = es;

	return 0;
err_klloc:
	kfree(es);
err_dev:
	return ret;
}

static int es8374_remove(struct i2c_client *client)
{
	if (es8374_save) {
		kfree(es8374_save);
		es8374_save = NULL;
	}
	return 0;
}


MODULE_DEVICE_TABLE(i2c, es8374_id);
static struct i2c_driver es8374_driver = {
	.driver = {
		.name = "es8374",
		.owner = THIS_MODULE,
	},

	.probe = es8374_probe,
	.remove = es8374_remove,
	.id_table = es8374_id,
};


int es8374_i2c_drv_init(void)
{
	i2c_add_driver(&es8374_driver);
	return 0;
}

void es8374_i2c_drv_exit(void)
{
	i2c_del_driver(&es8374_driver);
}

