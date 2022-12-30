#!/bin/sh

insmod /system/lib/modules/videobuf2-vmalloc.ko
insmod /system/lib/modules/libcomposite.ko
insmod /system/lib/modules/usbcamera.ko
insmod /system/lib/modules/gsensor-dw9714.ko

cd /system/bin/
anticopy ucamera
ucamera &

insmod /system/lib/modules/audio.ko
insmod /system/lib/modules/avpu.ko clk_name="vpll" avpu_clk=440000000
insmod /system/lib/modules/tx-isp-t31.ko isp_clk=200000000
insmod /system/lib/modules/sensor_gc2093_t31.ko

killall -USR1 ucamera
hid_update &

# sleep 3

# adbd &
