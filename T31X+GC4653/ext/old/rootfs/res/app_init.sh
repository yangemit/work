#!/bin/sh

insmod /system/lib/modules/videobuf2-vmalloc.ko
insmod /system/lib/modules/libcomposite.ko
insmod /system/lib/modules/usbcamera.ko

cd /system/bin/
anticopy ucamera
ucamera &

insmod /system/lib/modules/audio.ko
insmod /system/lib/modules/avpu.ko
insmod /system/lib/modules/tx-isp-t31.ko isp_clk=120000000
insmod /system/lib/modules/sensor_jxf23_t31.ko
killall -USR1 ucamera

# sleep 3

# adbd &
