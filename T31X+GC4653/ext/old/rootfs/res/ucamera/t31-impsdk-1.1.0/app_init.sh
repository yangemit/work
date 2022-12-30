#!/bin/sh
insmod /system/lib/modules/avpu.ko
insmod /system/lib/modules/audio.ko
insmod /system/lib/modules/tx-isp-t31.ko isp_clk=120000000
# insmod /system/lib/modules/tx-isp-t31.ko isp_clk=200000000
# insmod /system/lib/modules/sensor_sc4335_t31.ko
insmod /system/lib/modules/sensor_jxf37_t31.ko
insmod /system/lib/modules/videobuf2-vmalloc.ko
insmod /system/lib/modules/libcomposite.ko
insmod /system/lib/modules/usbcamera.ko

cd /system/bin/
/system/bin/anticopy ucamera
/system/bin/ucamera &
# sleep 3

#/system/bin/adbd &
