#!/bin/sh
mount -t jffs2 /dev/mtdblock3 /media

MODULE_DIR=$(uname -r)
mkdir -p /tmp/modules/${MODULE_DIR}
mkdir -p /lib/modules
cd /lib/modules/
ln -s /tmp/modules/*

insmod /system/lib/modules/key_drv.ko
insmod /system/lib/modules/videobuf2-vmalloc.ko
insmod /system/lib/modules/libcomposite.ko
insmod /system/lib/modules/usbcamera.ko af_en=1 dmic_channel=2 in_sample_rate=48000 streaming_maxpacket=2048
insmod /system/lib/modules/gsensor-dw9714.ko

insmod /system/lib/modules/avpu.ko
insmod /system/lib/modules/audio.ko codec_type=1 dmic_channel=2
insmod /system/lib/modules/tx-isp-t31.ko isp_clk=240000000
insmod /system/lib/modules/sensor_sc500ai_t31.ko
insmod /system/lib/modules/sensor_sc500ai_hdr_t31.ko
insmod /system/lib/modules/sensor_sc500ai_720_t31.ko
#killall -USR1 ucamera

#cd /system/bin/
#anticopy ucamera
ucamera &

sleep 5

hid_update &

sleep 3

adbd &

#motor_param_init
