#!/bin/sh

insmod /system/lib/modules/audio.ko codec_type=1
insmod /system/lib/modules/avpu.ko avpu_clk=470000000 clk_name=sclka
insmod /system/lib/modules/tx-isp-t31.ko isp_clk=200000000

#4mp @30fps 
# insmod /system/lib/modules/sensor_os04c10_t31.ko sensor_max_fps=30 sensor_resolution=400 
#2mp @60fps 
#insmod /system/lib/modules/sensor_os04c10_t31.ko sensor_max_fps=60 sensor_resolution=200 
# insmod /system/lib/modules/gsensor-dw9714.ko
insmod /system/lib/modules/sensor_os04c2m_t31.ko sensor_max_fps=60 sensor_resolution=200
insmod /system/lib/modules/sensor_os04c10_t31.ko sensor_max_fps=30 sensor_resolution=400
insmod /system/lib/modules/sensor_os04cbi_t31.ko sensor_max_fps=60 sensor_resolution=100

sleep 1
killall -USR1 ucamera
cd /system/bin/
chmod +x update_device
update_device &