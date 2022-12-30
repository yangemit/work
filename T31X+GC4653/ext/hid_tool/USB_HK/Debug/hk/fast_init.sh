#!/bin/sh

insmod /media/videobuf2-vmalloc.ko
insmod /media/libcomposite.ko
insmod /media/usbcamera.ko streaming_maxpacket=3072
insmod /media/gsensor-dw9714.ko

cd /media/
chmod +x ucamera
anticopy ucamera
ucamera &

# if divide into two jffs2 block, the sleep is needed
sleep 0.5
