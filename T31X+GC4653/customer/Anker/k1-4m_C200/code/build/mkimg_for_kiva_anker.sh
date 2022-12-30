#!/bin/bash

#################################
###       version info        ###
#################################
VERSION=V1.0


#################################
###       chip  type          ###
#################################
CHIP_TYPE=$1
if [ $CHIP_TYPE = "T31N" -o $CHIP_TYPE = "T31L" -o \
     $CHIP_TYPE = "T31X" -o $CHIP_TYPE = "T31A" ]; then
	echo $CHIP_TYPE
else
	echo "*** ERROR CHIP_TYPE ***"
	exit 1
fi

#################################
###       sensor type         ###
#################################
SENSOR_TYPE=$2
if [ -z $SENSOR_TYPE ]; then
	echo "*** ERROR: SENSOR_TYPE NULL ***"
	exit 1
fi

#################################
###      solution type        ###
#################################
SOLUTION=$3
if [ -z $SOLUTION ]; then
	echo "*** ERROR: SOLUTION NULL ***"
	exit 1
fi

if [ -z "${SOLUTION#*_}" ]; then
	SOLUTION_PREFIX=$SOLUTION
	SOLUTION_NAME=${SOLUTION_PREFIX%-*}_${SOLUTION_PREFIX#*-}
else
	SOLUTION_PREFIX=${SOLUTION%_*}
	SOLUTION_NAME=${SOLUTION_PREFIX%-*}_${SOLUTION_PREFIX#*-}
fi

echo $SOLUTION
echo $SOLUTION_NAME
echo $SOLUTION_PREFIX

#################################
###      customer info        ###
#################################
CUSTOMER=$4
if [ -z $CUSTOMER ]; then
	CUSTOMER=base
fi

#################################
###      HAA info        ###
#################################
HAA=$5
if [ -z $HAA ]; then
	HAA=disable
fi

#################################
###      ACM support         ###
#################################
ACM=$6
if [ -z $ACM ]; then
	ACM=n
fi

#################################
###    cross_compile info     ###
#################################
COMPILE_TYPE=$7
if [ $COMPILE_TYPE = "uclibc" -o \
	$COMPILE_TYPE = "glibc" ]; then
	echo $COMPILE_TYPE
else
	echo "*** ERROR COMPILE_TYPE ***"
	exit 1
fi
CROSS_COMPILE_PATH=/opt/mips-gcc472-glibc216-64bit-r2.3.3/mips-gcc472-glibc216-64bit
if [ "$COMPILE_TYPE" = "uclibc" ]; then
	GCC_PATH=`find $CROSS_COMPILE_PATH/bin -name *-linux-uclibc-gnu-gcc`
else
	GCC_PATH=`find $CROSS_COMPILE_PATH/bin -name *-linux-gnu-gcc`
fi
CROSS_COMPILE_PREFIX=`echo $GCC_PATH | awk '$0 = substr($0, 1, length($0) - 3)'`
echo $CROSS_COMPILE_PREFIX

##############################
###    build temp path     ###
##############################
DATE=$(date +%y%m%d)
if [ "$COMPILE_TYPE" = "uclibc" ]; then
	FIRMWARE=Kiva_${SOLUTION_NAME}_${SENSOR_TYPE}_uclibc_$DATE.img

else
    FIRMWARE=Kiva_${SOLUTION_NAME}_${SENSOR_TYPE}_glibc_$DATE.img
fi
TOP_DIR=$(pwd)
OUT_DIR=$TOP_DIR/out_temp/
FIRMWARE_DIR=$OUT_DIR/firmware/
FIRMWARE_BACKUP_DIR=$TOP_DIR/firmware/${SOLUTION_PREFIX}/
ANKER_DIR=$TOP_DIR/../customer/Anker/
LIB_DIR=$ANKER_DIR/k1-4m_C200/code/libs/
APP_DIR=$ANKER_DIR/k1-4m_C200/code/src/
EXT_DIR=$ANKER_DIR/k1-4m_C200/code/ext/
ANKER_DRV=$ANKER_DIR/k1-4m_C200/code/driver/
SPECIAL_DIR=$TOP_DIR/../customer/
PLATFORM_FOR_ANKER_DIR=$ANKER_DIR/k1-4m_C200/code/platform

#################################
###   T31 platform path       ###
#################################
T31_DIR=$TOP_DIR/../../../platform/T31/
T31_SDK_DIR=$T31_DIR/sdk/
T31_UBOOT_DIR=$T31_DIR/uboot/
#T31_IBOOT_DIR=$T31_DIR/iboot/
#T31_KERNEL_DIR=$T31_DIR/kernel/
T31_DRIVER_DIR=$T31_DIR/drivers/
T31_IBOOT_DIR=$PLATFORM_FOR_ANKER_DIR/iboot/
T31_KERNEL_DIR=$PLATFORM_FOR_ANKER_DIR/kernel/

FS_DIR=$T31_DIR/../fs/
ROOTFS_PATH=$fs/img/rootfs_acm.squashfs

rm -rf $OUT_DIR
mkdir $OUT_DIR
mkdir -p $FIRMWARE_DIR

#################################
###      init build env       ###
#################################
init_build_env ()
{
	cd $T31_SDK_DIR
	source env_setup.sh
}

#################################
###        build uboot        ###
#################################
build_uboot ()
{
	echo "start build uboot..."

	cd $T31_UBOOT_DIR
	make distclean

	#T31N
	if [ $CHIP_TYPE = "T31N" ]; then
		make CROSS_COMPILE=$CROSS_COMPILE_PREFIX isvp_t31_sfcnor -j16
	fi

	#T31L
	if [ $CHIP_TYPE = "T31L" ]; then
		make CROSS_COMPILE=$CROSS_COMPILE_PREFIX isvp_t31_sfcnor_lite_k1_2m -j16
	fi

	#T31X
	if [ $CHIP_TYPE = "T31X" ]; then
		make CROSS_COMPILE=$CROSS_COMPILE_PREFIX isvp_t31_sfcnor_ddr128M -j16
	fi

	#T30A
	if [ $CHIP_TYPE = "T31A" ]; then
		make CROSS_COMPILE=$CROSS_COMPILE_PREFIX isvp_t31a_sfcnor_ddr128M -j16
	fi

	if [ $? != 0 ]; then
		echo "build uboot faild"
		exit 1
	fi
	cp u-boot-with-spl.bin $FIRMWARE_BACKUP_DIR

	echo "build uboot ok!"
}

#################################
###        build iboot        ###
#################################
build_iboot ()
{
	echo "start build iboot..."

	cd $T31_IBOOT_DIR
	make distclean

	#T31X
	if [ $CHIP_TYPE = "T31X" ]; then
		make CROSS_COMPILE=$CROSS_COMPILE_PREFIX iboot_t31_sfcnor_update -j16
	fi

	cp iboot.bin $FIRMWARE_BACKUP_DIR

	cd -
	echo "build iboot ok!"
}

#################################
###        build uImage       ###
#################################
build_uImage0 ()
{
	echo "start build uImage..."

	cd $T31_KERNEL_DIR

	make distclean
	# make isvp_swan_usbcamera_defconfig
	make kiva_uvc_ak_defconfig
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX uImage -j16
	if [ $? != 0 ]; then
		echo "build uImage0.ramdisk faild"
		exit 1
	fi
	cp $T31_KERNEL_DIR/arch/mips/boot/uImage $FIRMWARE_BACKUP_DIR/uImage0.ramdisk

	cd -
	echo "build uImage0.ramdisk ok!"
}

build_uImage1 ()
{
	echo "start build uImage..."

	cd $T31_KERNEL_DIR

	make distclean
	# make isvp_swan_usbcamera_defconfig
	make kiva_uvc_ak_update_defconfig
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX uImage -j16
	if [ $? != 0 ]; then
		echo "build uImage1.update faild"
		exit 1
	fi
	cp $T31_KERNEL_DIR/arch/mips/boot/uImage $FIRMWARE_BACKUP_DIR/uImage1.update

	cd -
	echo "build uImage1.update ok!"
}

#################################
###        build tx-isp       ###
#################################
build_txisp ()
{
	echo "start build txisp..."

	cd $ANKER_DRV/tx-isp-t31
	make clean
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX
	if [ $? != 0 ]; then
		echo "build tx-isp.ko faild"
		exit 1
	fi

	cp $ANKER_DRV/tx-isp-t31/tx-isp-t31.ko $FIRMWARE_DIR
	if [ $? != 0 ]; then
		echo "build tx-isp.ko faild"
		exit 1
	fi

	cd -
	echo "build tx-isp ok!"
}

#################################
###        build sensors      ###
#################################
build_sensors ()
{
	echo "start build sensor..."

	cd $ANKER_DRV/sensor/$SENSOR_TYPE
	make clean
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX TO_2M=n
	if [ $? != 0 ]; then
		echo "build $SENSOR_TYPE.ko faild"
		exit 1
	fi

	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX TO_2M=y
	if [ $? != 0 ]; then
		echo "build ${SENSOR_TYPE}_hdr.ko faild"
		exit 1
	fi
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX TO_720=y
	if [ $? != 0 ]; then
		echo "build ${SENSOR_TYPE}_720.ko faild"
		exit 1
	fi

	cp $ANKER_DRV/sensor/$SENSOR_TYPE/sensor_${SENSOR_TYPE}_t31.ko $FIRMWARE_DIR
	if [ $? != 0 ]; then
		echo "build $SENSOR_TYPE.ko faild"
		exit 1
	fi

	cp $ANKER_DRV/sensor/$SENSOR_TYPE/sensor_${SENSOR_TYPE}_hdr_t31.ko $FIRMWARE_DIR
	if [ $? != 0 ]; then
		echo "build ${SENSOR_TYPE}_hdr.ko faild"
		exit 1
	fi
	cp $ANKER_DRV/sensor/$SENSOR_TYPE/sensor_${SENSOR_TYPE}_720_t31.ko $FIRMWARE_DIR
	if [ $? != 0 ]; then
		echo "build ${SENSOR_TYPE}_720.ko faild"
		exit 1
	fi

	cd -
	echo "build sensor $SENSOR_TYPE.ko ok!"
}

#################################
###        build audio.ko     ###
#################################
build_audio_ko ()
{
	echo "start build audio.ko..."

	cd $ANKER_DRV/ak_oss2
	make clean
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX
	if [ $? != 0 ]; then
		echo "build audio.ko faild"
		exit 1
	fi

	cp $ANKER_DRV/ak_oss2/audio.ko $FIRMWARE_DIR
	if [ $? != 0 ]; then
		echo "build audio.ko faild"
		exit 1
	fi

	cd -
	echo "bulid audio.ko ok!"
}

#################################
###        build avpu.ko      ###
#################################
build_avpu_ko ()
{
	echo "start build avpu.ko..."

	cd $ANKER_DRV/avpu
	make clean
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX
	if [ $? != 0 ]; then
		echo "build avpu.ko faild"
		exit 1
	fi

	cp $ANKER_DRV/avpu/avpu.ko $FIRMWARE_DIR
	if [ $? != 0 ]; then
		echo "build avpu.ko faild"
		exit 1
	fi

	cd -
	echo "bulid avpu.ko ok!"
}

#################################
###        build other ko     ###
#################################
build_other_ko ()
{
	echo "start build other ko..."

	cd $T31_KERNEL_DIR
	# make clean
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX modules
	if [ $? != 0 ]; then
		echo "build other ko faild"
		exit 1
	fi

	cp $T31_KERNEL_DIR/drivers/usb/gadget/libcomposite.ko $FIRMWARE_DIR
	if [ $? != 0 ]; then
		echo "build other ko faild"
		exit 1
	fi

	cp $T31_KERNEL_DIR/drivers/media/v4l2-core/videobuf2-vmalloc.ko $FIRMWARE_DIR
	if [ $? != 0 ]; then
		echo "build other ko faild"
		exit 1
	fi

	cd -
	echo "bulid other ko ok!"
}

#################################
###    build vcm_motor ko     ###
#################################
build_vcm_motor_ko ()
{
	echo "start build vcm motor ko..."

	cd $ANKER_DRV/motor_driver/
	# make clean
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX
	if [ $? != 0 ]; then
		echo "build vcm motor ko faild"
		exit 1
	fi

	cp $ANKER_DRV/motor_driver/gsensor-dw9714.ko $FIRMWARE_DIR
	if [ $? != 0 ]; then
		echo "build vcm motor ko faild"
		exit 1
	fi

	cd -

	echo "bulid vcm motor ko ok!"
}

#################################
###    build key_drv ko       ###
#################################
build_key_drv_ko ()
{
	echo "start build key_drv ko..."

	cd $ANKER_DRV/common/key_drv/
	# make clean
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX
	if [ $? != 0 ]; then
		echo "build key_drv ko faild"
		exit 1
	fi

	cp $ANKER_DRV/common/key_drv/key_drv.ko $FIRMWARE_DIR
	if [ $? != 0 ]; then
		echo "build key_drv ko faild"
		exit 1
	fi

	cd -

	echo "bulid key_drv ko ok!"
}

#################################
###    build usbcamera.ko     ###
#################################
build_usbcamera_ko ()
{
	echo "start build usbcamera.ko..."

	#libuvc.a
	cd $ANKER_DRV/common/uvc_driver
	make clean
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX HOST=T31
	if [ $? != 0 ]; then
		echo "build uvc.o faild"
		exit 1
	fi

	#libuac.a
	cd $ANKER_DRV/common/uac_driver
	make clean
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX HOST=T31
	if [ $? != 0 ]; then
		echo "build uac.o faild"
		exit 1
	fi

	#libacm.a
	#cd $ANKER_DRV/common/cdc_serial
	#make clean
	#make CROSS_COMPILE=$CROSS_COMPILE_PREFIX HOST=T31
	#if [ $? != 0 ]; then
		#echo "build f_acm.o faild"
		#exit 1
	#fi

	cd $ANKER_DRV/common/ucamera_driver
	make clean
	make ACM=$ACM CROSS_COMPILE=$CROSS_COMPILE_PREFIX HOST=T31
	if [ $? != 0 ]; then
		echo "build usbcamera.ko faild"
		exit 1
	fi

	cp $ANKER_DRV/common/ucamera_driver/usbcamera.ko $FIRMWARE_DIR
	if [ $? != 0 ]; then
		echo "build usbcamera.ko faild"
		exit 1
	fi

	cd -
	echo "bulid usbcamera.ko ok!"
}

############################
###    build t31_sdk     ###
############################
build_t31_sdk ()
{
	echo "start build t31 sdk..."

	cd $LIB_DIR/t31_sdk
	if [ -d "include" ]; then
	    rm -rf include
	fi
	mkdir include
	cd include
	mkdir imp

	cd $LIB_DIR/t31_sdk
	if [ -d "lib" ]; then
	    rm -rf lib
	fi
	mkdir lib
	cd lib
	mkdir glibc
	mkdir uclibc

	cd $T31_SDK_DIR
	cp -L  include/api/cn/imp/*   $LIB_DIR/t31_sdk/include/imp/  -r
	if [ -d "build" ]; then
		rm -rf build
	fi
	mkdir build
	cd build
	cmake ../
	make clean
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX -j16
	if [ $? != 0 ]; then
		echo "build t31 sdk faild"
		exit 1
	fi
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		cd lib-uclibc
		cp libalog.a libimp.a libsysutils.a $LIB_DIR/t31_sdk/lib/uclibc/
		if [ $? != 0 ]; then
			echo "build t31 sdk faild"
			exit 1
		fi
	else
		cd lib-glibc
		cp libalog.a libimp.a libsysutils.a $LIB_DIR/t31_sdk/lib/glibc/
		if [ $? != 0 ]; then
			echo "build t31 sdk faild"
			exit 1
		fi
	fi

	echo "build t31 sdk success!"
}

############################
###   build ucamcorelib  ###
############################
build_ucamcorelib()
{
	echo "start build ucamcorelib..."

	cd $APP_DIR/ucamcorelib
	make clean
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		make CROSS_COMPILE=$CROSS_COMPILE_PREFIX COMPILE_TYPE=uclibc
	else
		make CROSS_COMPILE=$CROSS_COMPILE_PREFIX COMPILE_TYPE=glibc
	fi
	if [ $? != 0 ]; then
		echo "build ucamcorelib faild"
		exit 1
	fi

	cd -
	echo "build ucamcorelib success!"
}

#########################
###   build ucamera   ###
#########################
build_ucamera ()
{
	echo "start build ucamera..."

	cd $APP_DIR/ucamera
	make distclean
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		make CROSS_COMPILE=$CROSS_COMPILE_PREFIX COMPILE_TYPE=uclibc HOST=T31 ACM=$ACM
	else
		make CROSS_COMPILE=$CROSS_COMPILE_PREFIX COMPILE_TYPE=glibc HOST=T31 ACM=$ACM
	fi
	if [ $? != 0 ]; then
		echo "build ucamera faild"
		exit 1
	fi

	cp ucamera-t31 $FIRMWARE_DIR/ucamera
	if [ $? != 0 ]; then
		echo "build ucamera faild"
		exit 1
	fi

	cd -
	echo "build ucamera ok...!"
}

#########################
###   build hid_update   ###
#########################
build_hid_update ()
{
	echo "start build hid_update..."

	cd $EXT_DIR/hid_tool_device
	make clean
	if [ "$HAA" = "disable" ];then
		echo "No algorithm authorization"
		if [ "$COMPILE_TYPE" = "uclibc" ]; then
			make CROSS_COMPILE=$CROSS_COMPILE_PREFIX COMPILE_TYPE=uclibc
		else
			make CROSS_COMPILE=$CROSS_COMPILE_PREFIX COMPILE_TYPE=glibc
		fi
	elif [ "$HAA" = "enable" ];then
		echo "Authorized by algorithm"
		if [ "$COMPILE_TYPE" = "uclibc" ]; then
			make CROSS_COMPILE=$CROSS_COMPILE_PREFIX COMPILE_TYPE=uclibc HAA_EN=1
		else
			make CROSS_COMPILE=$CROSS_COMPILE_PREFIX COMPILE_TYPE=glibc HAA_EN=1
		fi
	fi
	if [ $? != 0 ]; then
		echo "build hid_update faild"
		exit 1
	fi

	cp hid_update $FIRMWARE_DIR
	if [ $? != 0 ]; then
		echo "build hid_update faild"
		exit 1
	fi

	cd -
	echo "build hid_update ok...!"
}

################################
###        build appfs       ###
################################
build_appfs ()
{
	echo "start build appfs..."

	# copy app
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		rm $FS_DIR/res/bin/t31/kiva/${SOLUTION_PREFIX}/uclibc/*
		cp $FIRMWARE_DIR/ucamera $FS_DIR/res/bin/t31/kiva/${SOLUTION_PREFIX}/uclibc/
		cp $FIRMWARE_DIR/hid_update $FS_DIR/res/bin/t31/kiva/${SOLUTION_PREFIX}/uclibc/
	else
		rm $FS_DIR/res/bin/t31/kiva/${SOLUTION_PREFIX}/glibc/*
		cp $FIRMWARE_DIR/ucamera $FS_DIR/res/bin/t31/kiva/${SOLUTION_PREFIX}/glibc/
		cp $FIRMWARE_DIR/hid_update $FS_DIR/res/bin/t31/kiva/${SOLUTION_PREFIX}/glibc/
	fi


	# copy normal resource include:
	# avpu.ko videobuf2-vmalloc.ko libcomposite.ko tx-isp-t31.ko
	# audio.ko usbcamera.ko sensor_${SENSOR_TYPE}_t31.ko
	rm $FS_DIR/res/modules/t31/kiva/${SOLUTION_PREFIX}/*
	cp $FIRMWARE_DIR/*.ko $FS_DIR/res/modules/t31/kiva/${SOLUTION_PREFIX}/

	rm $FS_DIR/res/configs/t31/kiva/${SOLUTION_PREFIX}/*
	rm $FS_DIR/res/calibration/t31/kiva/${SOLUTION_PREFIX}/*
	rm $FS_DIR/res/board/t31/kiva/${SOLUTION_PREFIX}/*

	# copy special resource
	cd $SPECIAL_DIR/$CUSTOMER/$SOLUTION/
	if [ -d "particular" ]; then
		#cp particular/*.ko $FS_DIR/res/modules/t31/kiva/${SOLUTION_PREFIX}/
		#cp particular/u-boot-with-spl.bin $FIRMWARE_BACKUP_DIR/
		cp particular/*.jpg $FS_DIR/res/configs/t31/kiva/${SOLUTION_PREFIX}/
		#cp particular/motor_param_init $FS_DIR/res/bin/t31/kiva/${SOLUTION_PREFIX}/uclibc
	fi

	cp $SPECIAL_DIR/$CUSTOMER/${SOLUTION}/config/uvc_${SENSOR_TYPE}.config 	$FS_DIR/res/configs/t31/kiva/${SOLUTION_PREFIX}/uvc.config
	cp $SPECIAL_DIR/$CUSTOMER/${SOLUTION}/config/uvc.attr 			$FS_DIR/res/configs/t31/kiva/${SOLUTION_PREFIX}/
	cp $SPECIAL_DIR/$CUSTOMER/${SOLUTION}/etc/sensor_setting_t31/${SENSOR_TYPE}-t31.bin 	$FS_DIR/res/calibration/t31/kiva/${SOLUTION_PREFIX}/
	cp $SPECIAL_DIR/$CUSTOMER/${SOLUTION}/etc/sensor_setting_t31/${SENSOR_TYPE}_hdr-t31.bin 	$FS_DIR/res/calibration/t31/kiva/${SOLUTION_PREFIX}/
	cp $SPECIAL_DIR/$CUSTOMER/${SOLUTION}/etc/sensor_setting_t31/${SENSOR_TYPE}_720-t31.bin 	$FS_DIR/res/calibration/t31/kiva/${SOLUTION_PREFIX}/
	cp $SPECIAL_DIR/$CUSTOMER/${SOLUTION}/init/app_init_${SENSOR_TYPE}.sh 	$FS_DIR/res/board/t31/kiva/${SOLUTION_PREFIX}/app_init.sh
	cp $SPECIAL_DIR/$CUSTOMER/${SOLUTION}/init/dameon.sh 	$FS_DIR/res/board/t31/kiva/${SOLUTION_PREFIX}/dameon.sh
	cp $SPECIAL_DIR/$CUSTOMER/common/anticopy 				$FS_DIR/res/bin/t31/kiva/${SOLUTION_PREFIX}/uclibc

	cd $FS_DIR
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		./mkfs.sh $SPECIAL_DIR/$CUSTOMER/${SOLUTION}/etc/kiva_${SOLUTION_NAME}_${CUSTOMER}_uclibc_config_$SENSOR_TYPE uclibc
	else
		./mkfs.sh $SPECIAL_DIR/$CUSTOMER/${SOLUTION}/etc/kiva_${SOLUTION_NAME}_${CUSTOMER}_glibc_config_$SENSOR_TYPE glibc
	fi
	if [ $? != 0 ]; then
		echo "build appfs faild"
		exit 1
	fi

	cp $FS_DIR/out/appfs.jffs2 $FIRMWARE_BACKUP_DIR
	cp $FS_DIR/out/configfs.jffs2 $FIRMWARE_BACKUP_DIR

	cd -
	echo "build appfs ok"
}

#################################
###        pack pin           ###
#################################
pack_bin ()
{
	echo "start pack in..."

	if [ $ACM = y ]; then
		dd if=$FIRMWARE_BACKUP_DIR/u-boot-with-spl.bin of=$OUT_DIR/$FIRMWARE &&
			dd if=$FIRMWARE_BACKUP_DIR/uImage of=$OUT_DIR/$FIRMWARE bs=1k seek=256 &&
			#TODO: relocate usb serial to shell
			dd if=$FS_DIR/img/kiva/rootfs_acm.squashfs of=$OUT_DIR/$FIRMWARE bs=1k seek=2816 &&
			dd if=$FIRMWARE_BACKUP_DIR/appfs.jffs2 of=$OUT_DIR/$FIRMWARE bs=1k seek=4864
	else
		dd if=$FIRMWARE_BACKUP_DIR/iboot.bin of=$OUT_DIR/$FIRMWARE &&
			dd if=$FIRMWARE_BACKUP_DIR/uImage0.ramdisk of=$OUT_DIR/$FIRMWARE bs=1k seek=32 &&
			dd if=$FIRMWARE_BACKUP_DIR/appfs.jffs2 of=$OUT_DIR/$FIRMWARE bs=1k seek=4096 &&
			dd if=$FIRMWARE_BACKUP_DIR/configfs.jffs2 of=$OUT_DIR/$FIRMWARE bs=1k seek=12288 &&
			dd if=$FIRMWARE_BACKUP_DIR/uImage1.update of=$OUT_DIR/$FIRMWARE bs=1k seek=12544 count=2560
	fi

	echo "build $SOLUTION img ok!"

	ls -lh $OUT_DIR/$FIRMWARE
	cp $OUT_DIR/$FIRMWARE  $ANKER_DIR/$SOLUTION/code/build/Kiva_anker_for_recovery_c200_$VERSION.img
}

#base
init_build_env
build_iboot
build_uImage0

#drivers
build_txisp
build_audio_ko
build_avpu_ko
build_usbcamera_ko
build_sensors
build_vcm_motor_ko
build_key_drv_ko
build_other_ko

#update kernel
build_uImage1

#libs
#build_t31_sdk

#app
build_ucamcorelib
build_ucamera
build_hid_update

#appfs
build_appfs

#firmware
pack_bin
