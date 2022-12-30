#!/bin/sh
echo 1 > /proc/sys/vm/overcommit_memory

insmod /system/lib/modules/audio.ko
insmod /system/lib/modules/tx-isp-t21.ko isp_clk=150000000
insmod /system/lib/modules/sensor_jxf37_t21.ko
insmod /system/lib/modules/libcomposite.ko
insmod /system/lib/modules/videobuf2-vmalloc.ko
insmod /system/lib/modules/usbcamera_t21.ko

cd /system/bin
./anticopy ucamera
./ucamera &
