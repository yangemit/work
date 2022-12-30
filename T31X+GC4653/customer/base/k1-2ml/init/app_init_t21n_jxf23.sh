#!/bin/sh

insmod /system/lib/modules/videobuf2-vmalloc.ko
insmod /system/lib/modules/libcomposite.ko
insmod /system/lib/modules/usbcamera.ko

cd /system/bin/
anticopy ucamera
ucamera &

insmod /system/lib/modules/audio.ko
insmod /system/lib/modules/tx-isp-t21.ko isp_clk=150000000
insmod /system/lib/modules/sensor_jxf23_t21.ko

killall -USR1 ucamera
hid_update &

# sleep 3

# adbd &
