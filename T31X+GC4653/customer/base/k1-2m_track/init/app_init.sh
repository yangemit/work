#!/bin/sh

insmod /system/lib/modules/step_motor.ko hmaxstep=4096
insmod /system/lib/modules/videobuf2-vmalloc.ko
insmod /system/lib/modules/libcomposite.ko
insmod /system/lib/modules/usbcamera.ko

insmod /system/lib/modules/audio.ko
insmod /system/lib/modules/avpu.ko
insmod /system/lib/modules/tx-isp-t31.ko isp_clk=200000000
insmod /system/lib/modules/sensor_jxf37_t31.ko data_interface=1

cd /system/bin/
anticopy ucamera
ucamera &
# killall -USR1 ucamera
sleep 1
hid_update &

# sleep 3

# adbd &
