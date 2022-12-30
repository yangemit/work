#!/bin/sh

insmod /system/lib/modules/videobuf2-vmalloc.ko
insmod /system/lib/modules/libcomposite.ko
insmod /system/lib/modules/usbcamera.ko

cd /system/bin/
anticopy ucamera
ucamera &

insmod /system/lib/modules/c980ai_gpio.ko
insmod /system/lib/modules/audio.ko dmic_amic_sync=1
insmod /system/lib/modules/avpu.ko
insmod /system/lib/modules/tx-isp-t31.ko isp_clk=240000000
insmod /system/lib/modules/sensor_sc4335_t31.ko data_interface=1
insmod /system/lib/modules/gsensor-dw9714.ko

killall -USR1 ucamera
hid_update &

#sleep 3

#adbd &
