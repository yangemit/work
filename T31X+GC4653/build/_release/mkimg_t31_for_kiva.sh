#!/bin/bash

#
# pack all of t31 firmware
#

#################################
###       version info        ###
#################################
VERSION=V2.0.0

#################################
###       chip type         ###
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
	echo "*** ERROR: SENSOR_TYPE IS NULL ***"
	exit 1
fi

#################################
###      customer info        ###
#################################
CUSTOMER=$3
if [ -z $CUSTOMER ]; then
	echo "*** ERROR: CUSTOMER IS NULL ***"
	exit 1
fi

#################################
###      solution type        ###
#################################
SOLUTION=$4
if [ -z $SOLUTION ]; then
	echo "*** ERROR: SOLUTION IS NULL ***"
	exit 1
fi

echo "$SOLUTION" | grep -q "_"
if [ $? -eq 0 ]; then
	SOLUTION_PREFIX=${SOLUTION%_*}
	SOLUTION_NAME=${SOLUTION_PREFIX%-*}_${SOLUTION_PREFIX#*-}
else
	SOLUTION_PREFIX=$SOLUTION
	SOLUTION_NAME=${SOLUTION_PREFIX%-*}_${SOLUTION_PREFIX#*-}
fi

echo "SOLUTION        = $SOLUTION"
echo "SOLUTION_NAME   = $SOLUTION_NAME"
echo "SOLUTION_PREFIX = $SOLUTION_PREFIX"

#################################
###    cross_compile info     ###
#################################
COMPILE_TYPE=$5
if [ $COMPILE_TYPE = "uclibc" -o \
	$COMPILE_TYPE = "glibc" ]; then
echo $COMPILE_TYPE
else
	echo "*** ERROR COMPILE_TYPE ***"
	exit 1
fi

#################################
###        af function        ###
#################################
AF=$6
if [ $AF = "af" -o $AF = "null" ]; then
	echo $AF
else
	AF=null
	echo "*** WARNING: ERROR PARAMETER OF AF ***"
	echo "*** Default Disable AF ***"
fi

#################################
###     advanced function     ###
#################################
ADVANCED=$7
if [ -z $ADVANCED ]; then
	echo "*** ERROR:  IS NULL ***"
	exit 1
fi
if [ $ADVANCED = "quickStart" -o \
	$ADVANCED = "autoCenter" -o \
	$ADVANCED = "PTZ" -o \
	$ADVANCED = "cdcUpdate" ]; then
	echo $ADVANCED
else
	ADVANCED=null
	echo "*** WARNING: ERROR PARAMETER OF ADVANCED ***"
	echo "*** Default Disable ADVANCED ***"
fi

######################################
###             path               ###
######################################
DATA=$(date +%y%m%d)
CURRENT_DIR=$(pwd)
TOP_DIR=$CURRENT_DIR/..

#base path
BUILD_DIR=$CURRENT_DIR
OPENSOURCE_DIR=$TOP_DIR/opensource
KIVA_DIR=$TOP_DIR/kiva

#opensource path
IBOOT_DIR=$OPENSOURCE_DIR/iboot
UBOOT_DIR=$OPENSOURCE_DIR/uboot
KERNEL_DIR=$OPENSOURCE_DIR/kernel
DRIVER_DIR=$OPENSOURCE_DIR/drivers

#kiva source path
KIVA_DRIVER_DIR=$KIVA_DIR/driver
HID_UPDATE_DIR=$KIVA_DIR/ext/hid_tool_device
SOLUTION_DIR=$KIVA_DIR/customer/$SOLUTION

#fs path
FS_DIR=$CURRENT_DIR/fs
APPFS_DIR=$FS_DIR/appfs

#resource path
RESOURCE_DIR=$TOP_DIR/../resource

#rootfs path
ROOTFS=$RESOURCE_DIR/rootfs_t31/root-${COMPILE_TYPE}-toolchain4.7.2-1.1.squashfs

#cross_compiler path
CROSS_COMPILER_DIR=$RESOURCE_DIR/toolchain/gcc_472/mips-gcc472-glibc216-64bit

#target path
OUT_DIR=$CURRENT_DIR/out_temp
FIRMWARE_TMP_DIR=$OUT_DIR/firmware_tmp/
T31_UPDATE_FIRMWARE=$OUT_DIR/t31_update_firmware/
TARGET=${CHIP_TYPE}_firmware_${SOLUTION}_${SENSOR_TYPE}_${COMPILE_TYPE}_${VERSION}_${DATA}.bin

#driver modules path
TX_ISP_DIR=$DRIVER_DIR/tx-isp-t31
SENSOR_DIR=$DRIVER_DIR/sensors-t31/$SENSOR_TYPE
AUDIO_DIR=$DRIVER_DIR/oss2
AVPU_DIR=$DRIVER_DIR/avpu
AF_DRIVER_DIR=$KIVA_DRIVER_DIR/common/motor_driver
UCAMERA_DRIVER_DIR=$KIVA_DRIVER_DIR/common/ucamera_driver
STEP_MOTOR_DRIVER_DIR=$KIVA_DRIVER_DIR/motor

#################################
###    prepare config file    ###
#################################
if [[ "$SENSOR_TYPE" =~ ^jx ]]; then
	SENSOR_ADDR=0x40
elif [[ "$SENSOR_TYPE" =~ ^gc ]]; then
	SENSOR_ADDR=0x37
elif [[ "$SENSOR_TYPE" =~ ^sc ]]; then
	SENSOR_ADDR=0x30
elif [[ "$SENSOR_TYPE" =~ ^os ]]; then
	SENSOR_ADDR=0x3c
elif [[ "$SENSOR_TYPE" =~ ^ov ]]; then
	SENSOR_ADDR=0x36
elif [[ "$SENSOR_TYPE" =~ ^imx ]]; then
	SENSOR_ADDR=0x1a
else
	echo "*** ERROR SENSOR_TYPE ***"
	exit 1
fi

UVC_CONFIG_FILE=$KIVA_DIR/config/uvc.config
APP_INIT_FILE=$KIVA_DIR/config/app_init.sh
IMAGE_BIN_FILE=$RESOURCE_DIR/sensor_setting/${SENSOR_TYPE}-t31.bin

#set uvc_config_file
sed -ri 's/(sensor_name[[:blank:]]*:)[^[:blank:]]*/\1'"$SENSOR_TYPE"'/' $UVC_CONFIG_FILE
sed -ri 's/(i2c_addr[[:blank:]]*:)[^[:blank:]]*/\1'"$SENSOR_ADDR"'/' $UVC_CONFIG_FILE

#set app_init_file
sed -ri 's/(sensor_)[^_]*/\1'"$SENSOR_TYPE"'/' $APP_INIT_FILE

#set af parameter
sed -i '/^af/d' $UVC_CONFIG_FILE
sed -i '/^motor/d' $UVC_CONFIG_FILE
sed -i 's/ af_en=1/ /' $APP_INIT_FILE
sed -i '/dw9714/d' $APP_INIT_FILE

if [ $AF = "af" ]; then
sed -i '/^rcmode/a\
af_en           :1\
motor_pixel     :400\
motor_step      :25\
motor_sleep     :80000\
motor_step_max  :600\
motor_step_min  :300'	$UVC_CONFIG_FILE

sed -i '/usbcamera.ko/s/$/ af_en=1/' $APP_INIT_FILE
sed -i '/usbcamera.ko/a\insmod /lib/modules/gsensor-dw9714.ko' $APP_INIT_FILE

#if [ $AF = "null" ]; then
#	sed -i '/^af/d' $UVC_CONFIG_FILE
#	sed -i '/^motor/d' $UVC_CONFIG_FILE
#fi
fi

#set resolution parameter
sed -i '/^{2560/d' $UVC_CONFIG_FILE
sed -i '/960}$/d' $UVC_CONFIG_FILE
if [ $SOLUTION = "k1-2m" -o $SOLUTION = "k1-2me" ]; then
sed -i '/1080}/a\{1280, 960}' $UVC_CONFIG_FILE
sed -ri 's/(width[[:blank:]]*:)[^[:blank:]]*/\1'"1920"'/' $UVC_CONFIG_FILE
sed -ri 's/(height[[:blank:]]*:)[^[:blank:]]*/\1'"1080"'/' $UVC_CONFIG_FILE
fi
if [ $SOLUTION = "k1-4m" ]; then
sed -i '/360}/a\{2560, 1440}' $UVC_CONFIG_FILE
sed -ri 's/(width[[:blank:]]*:)[^[:blank:]]*/\1'"2560"'/' $UVC_CONFIG_FILE
sed -ri 's/(height[[:blank:]]*:)[^[:blank:]]*/\1'"1440"'/' $UVC_CONFIG_FILE
fi

#xxx.bin
if [ ! -f $IMAGE_BIN_FILE ]; then
	echo "NO IMAGE BIN FILE :${SENSOR_TYPE}-t31.bin !!!"
	echo "PLEASE CHECK THE FILE EXIST OR NOT:"
	echo "		$IMAGE_BIN_FILE !!!"
	exit 1
fi

########################################
###       prepare tmp directroy      ###
########################################
#rm fs dir
if [ -d $FS_DIR ]; then
	rm -rf $FS_DIR
fi

#rm out dir
if [ -d $OUT_DIR ]; then
    rm -rf $OUT_DIR
fi

#create appfs dir
mkdir -p $APPFS_DIR/{bin,config,etc/sensor,init,lib/modules}
touch $APPFS_DIR/.system
echo mkdir appfs dir ok

#create out dir
mkdir -p $FIRMWARE_TMP_DIR
mkdir -p $T31_UPDATE_FIRMWARE
echo mkdir firmware dir ok

########################################
###       set cross_compileir        ###
########################################
echo $CROSS_COMPILER_DIR
if [ -d $CROSS_COMPILER_DIR ]; then
	export PATH=$CROSS_COMPILER_DIR/bin:$PATH
	echo $PATH
else

	#tar -jxvf ${CROSS_COMPILER_DIR}.tar.bz2 -C $RESOURCE_DIR/toolchain/gcc_472/
	7z x ${CROSS_COMPILER_DIR}-r2.3.3.7z -o$RESOURCE_DIR/toolchain/gcc_472/
	export PATH=$CROSS_COMPILER_DIR/bin:$PATH
fi

if [ "$COMPILE_TYPE" = "uclibc" ]; then
	echo $COMPILE_TYPE
	GCC_PATH=`find $CROSS_COMPILER_DIR/bin -name *-linux-uclibc-gnu-gcc`
	echo $GCC_PATH
else
	GCC_PATH=`find $CROSS_COMPILER_DIR/bin -name *-linux-gnu-gcc`
	echo $GCC_PATH
fi
CROSS_COMPILE_PREFIX=`echo $GCC_PATH | awk '$0 = substr($0, 1, length($0) - 3)'`
echo $CROSS_COMPILE_PREFIX

########################################

#build iboot
build_iboot ()
{
	cd $IBOOT_DIR
	make distclean
	if [ $ADVANCED = "quickStart" ]; then
		if [ $CHIP_TYPE = "T31N" -o $CHIP_TYPE = "T31L" ]; then
			IBOOT_DEFCONFIG=iboot_t31_sfcnor_lite_lzo
		elif [ $CHIP_TYPE = "T31X" ]; then
			IBOOT_DEFCONFIG=iboot_t31_sfcnor_lzo
		elif [ $CHIP_TYPE = "T31A" ]; then
			IBOOT_DEFCONFIG=isvp_t31a_sfcnor_lzo
		fi
	elif [ $ADVANCED = "cdcUpdate" ]; then
		IBOOT_DEFCONFIG=iboot_t31_sfcnor_update
	else
		IBOOT_DEFCONFIG=iboot_t31_sfcnor
	fi
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX $IBOOT_DEFCONFIG -j16
	if [ $? != 0 ]; then
		echo "build iboot faild"
		exit 1
	fi
	cd -

	########################################
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		echo uclibc------build_iboot
	else
		echo glibc------build_iboot
	fi
	cp $IBOOT_DIR/iboot.bin $FIRMWARE_TMP_DIR/
	########################################

	echo "build iboot ok!"
}

# build uboot
build_uboot ()
{
	cd $UBOOT_DIR
	make distclean
	if [ $CHIP_TYPE = "T31N" ]; then
		make CROSS_COMPILE=$CROSS_COMPILE_PREFIX isvp_t31_sfcnor -j16
	fi

	if [ $CHIP_TYPE = "T31X" -o $CHIP_TYPE = "T31A" ]; then
		make CROSS_COMPILE=$CROSS_COMPILE_PREFIX isvp_t31_sfcnor_ddr128M -j16
	fi

	if [ $CHIP_TYPE = "T31L" ]; then
		make CROSS_COMPILE=$CROSS_COMPILE_PREFIX isvp_t31_sfcnor_lite_k1_2m -j16
	fi

	if [ $? != 0 ]; then
		echo "build uboot faild"
		exit 1
	fi
	cd -

	########################################
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		echo uclibc------build_uboot
	else
		echo glibc------build_uboot
	fi
	cp $UBOOT_DIR/u-boot-with-spl.bin $FIRMWARE_TMP_DIR/
	########################################

	echo "build uboot ok!"
}

# build uImage
build_uImage ()
{
	cd $KERNEL_DIR
	make distclean
	if [ $ADVANCED = "quickStart" ]; then
	KERNEL_DEFCONFIG=kiva_uvc_quick_start_defconfig
	elif [ $ADVANCED = "cdcUpdate" ]; then
		KERNEL_DEFCONFIG=kiva_uvc_ramdisk_defconfig
	else
		KERNEL_DEFCONFIG=kiva_uvc_defconfig
	fi

	make $KERNEL_DEFCONFIG
	#make isvp_k1_4k_usb_net_defconfig
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX uImage -j16
	if [ $? != 0 ]; then
		echo "build uImage faild"
		exit 1
	fi
	cd -

	########################################
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		echo uclibc------build_uImage
	else
		echo glibc------build_uImage
	fi
	if [ $ADVANCED = "quickStart" ]; then
		cp $KERNEL_DIR/arch/mips/boot/uImage.lzo $FIRMWARE_TMP_DIR/uImage
	else
		cp $KERNEL_DIR/arch/mips/boot/uImage $FIRMWARE_TMP_DIR/uImage
	fi
	########################################
	echo "build uImage ok!"
}

#build update kernel
build_uImage_update ()
{
	cd $KERNEL_DIR
	make distclean
	make kiva_uvc_cdc_update_defconfig
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX uImage -j16
	if [ $? != 0 ]; then
		echo "build uImage faild"
		exit 1
	fi
	cd -

	########################################
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		echo uclibc------build_uImage
	else
		echo glibc------build_uImage
	fi

	cp $KERNEL_DIR/arch/mips/boot/uImage.lzo $FIRMWARE_TMP_DIR/uImage.update
	########################################
	echo "build uImage.update ok!"
}


# build tx-isp
build_txisp ()
{
	cd $TX_ISP_DIR
	make clean;make CROSS_COMPILE=$CROSS_COMPILE_PREFIX
	if [ $? != 0 ]; then
		echo "build tx-isp faild"
		exit 1
	fi
	cd -

	########################################
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		echo uclibc------build_txips
	else
		echo glibc------build_txisp
	fi
	cp $TX_ISP_DIR/tx-isp-t31.ko $APPFS_DIR/lib/modules/
	if [ $? != 0 ]; then
		echo "cp isp driver faild"
		exit 1
	fi
	########################################
	echo "build tx-isp ok!"
}

# build sensors
build_sensors ()
{
	########################################
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		echo uclibc------build_sensors
	else
		echo glibc------build_sensors
	fi
	########################################

	cd $SENSOR_DIR
	make clean;make CROSS_COMPILE=$CROSS_COMPILE_PREFIX
	if [ $? != 0 ]; then
		echo "build sensor faild"
		exit 1
	fi
	cd -

	cp $SENSOR_DIR/sensor_${SENSOR_TYPE}_t31.ko $APPFS_DIR/lib/modules/
	if [ $? != 0 ]; then
		echo "cp sensor driver faild"
		exit 1
	fi
	echo "build sensor ko ok!"
}

# build audio
build_audio ()
{
	########################################
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		echo uclibc------build_audio
	else
		echo glibc------build_audio
	fi
	########################################

	cd $AUDIO_DIR
	make clean;make CROSS_COMPILE=$CROSS_COMPILE_PREFIX
	if [ $? != 0 ]; then
		echo "build audio faild"
		exit 1
	fi
	cd -

	cp $AUDIO_DIR/audio.ko $APPFS_DIR/lib/modules/
	echo "build audio.ko ok!"
}

# build avpu
build_avpu ()
{
	########################################
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		echo uclibc------build_avpu
	else
		echo glibc------build_avpu
	fi
	########################################

	cd $AVPU_DIR
	make clean;make CROSS_COMPILE=$CROSS_COMPILE_PREFIX
	if [ $? != 0 ]; then
		echo "build audio faild"
		exit 1
	fi
	cd -

	cp $AVPU_DIR/avpu.ko $APPFS_DIR/lib/modules/
	echo "build avpu.ko ok!"
}

#################################
###    build usbcamera.ko     ###
#################################
build_usbcamera_ko ()
{

	echo "start build usbcamera.ko..."

	cd $UCAMERA_DRIVER_DIR
	make clean;make HOST_SOC=t31 CROSS_COMPILE=$CROSS_COMPILE_PREFIX ISVP_ENV_KERNEL_DIR=$KERNEL_DIR
	if [ $? != 0 ]; then
		echo "build usbcamera.ko faild"
		exit 1
	fi

	cp $UCAMERA_DRIVER_DIR/usbcamera.ko $APPFS_DIR/lib/modules/

	echo "bulid usbcamera.ko ok!"
}
#################################
###    build other ko         ###
#################################
build_other_ko()
{
	echo "start build other ok..."
	cd $KERNEL_DIR
	make CROSS_COMPILE=$CROSS_COMPILE_PREFIX modules
	if [ $? != 0 ];then
		echo "build othe ko faild"
		exit 1
	fi
	cp $KERNEL_DIR/drivers/usb/gadget/libcomposite.ko $APPFS_DIR/lib/modules/
	if [ $? != 0 ];then
		echo "build othe ko faild"
		exit 1
	fi
	cp $KERNEL_DIR/drivers/media/v4l2-core/videobuf2-vmalloc.ko $APPFS_DIR/lib/modules/
	if [ $? != 0 ];then
		echo "build othe ko faild"
		exit 1
	fi

	echo "build other ko ok!"
}

#################################
###  build gsensor-dw9714.ko  ###
#################################
build_gsensor-dw9714_ko ()
{
    #AF
    echo "start build gsensor-dw9714.ko..."

    cd $AF_DRIVER_DIR
    make clean
    make CROSS_COMPILE=$CROSS_COMPILE_PREFIX HOST=T31 KERNEL_DIR=$KERNEL_DIR
    if [ $? != 0 ]; then
	echo "build gsensor-dw9714.ko faild"
	exit 1
    fi
    cd -
    cp $AF_DRIVER_DIR/gsensor-dw9714.ko $APPFS_DIR/lib/modules/
    echo "bulid gsensor-dw9714.ko ok!"
}

#################################
###    build step_motor.ko    ###
#################################
build_step_motor_ko ()
{
    #step_motor
    echo "start build steap_motor.ko..."

    cd $STEP_MOTOR_DRIVER_DIR
    make clean
    make
    if [ $? != 0 ]; then
	echo "build step_motor.ko faild"
	exit 1
    fi
    cd -
    cp $STEP_MOTOR_DRIVER_DIR/steap_motor.ko $APPFS_DIR/lib/modules/
    echo "bulid step_motor.ko ok!"
}

#build app
build_app ()
{
	_MODULE_FACEAE=MODULE_FACEAE=disbale
	_MODULE_FACEZOOM=MODULE_FACEZOOM=disbale
	_MODULE_PERSONDET=MODULE_PERSONDET=disbale
	_MODULE_AUTOFOCUS=MODULE_AUTOFOCUS=disbale
	if [ $AF = "af" ]; then
		_MODULE_AUTOFOCUS=MODULE_AUTOFOCUS=enable
	fi

	if [ $ADVANCED = "autoCenter" ]; then
		_MODULE_FACEZOOM=MODULE_FACEZOOM=enable
	elif [ $ADVANCED = "PTZ" ]; then
		_MODULE_PERSONDET=MODULE_PERSONDET=enable
	fi

	cd $KIVA_DIR
	########################################
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		make HOST_SOC=t31 LIBTYPE=uclibc clean;make HOST_SOC=t31 LIBTYPE=uclibc CROSS_COMPILE=$CROSS_COMPILE_PREFIX \
			$_MODULE_FACEAE $_MODULE_FACEZOOM $_MODULE_PERSONDET $_MODULE_AUTOFOCUS
		echo uclibc------build_app
	else
		make HOST_SOC=t31 LIBTYPE=glibc clean;make HOST_SOC=t31 LIBTYPE=glibc CROSS_COMPILE=$CROSS_COMPILE_PREFIX \
			$_MODULE_FACEAE $_MODULE_FACEZOOM $_MODULE_PERSONDET $_MODULE_AUTOFOCUS
		echo glibc------build_app
	fi
	########################################

	if [ $? != 0 ]; then
		echo "build app faild"
		exit 1
	fi

	cd -
	cp $KIVA_DIR/ucamera-t31 $APPFS_DIR/bin/ucamera
	echo "build app ucamera ok!"
}

#build hid_update
build_hid_update ()
{
	cd $HID_UPDATE_DIR
	########################################
	if [ "$COMPILE_TYPE" = "uclibc" ]; then
		make clean;make LIBTYPE=uclibc CROSS_COMPILE=$CROSS_COMPILE_PREFIX
		echo uclibc------build_hid_update
	else
		make clean;make LIBTYPE=glibc CROSS_COMPILE=$CROSS_COMPILE_PREFIX
		echo glibc------build_hid_update
	fi
	########################################

	if [ $? != 0 ]; then
		echo "build hid update faild"
		exit 1
	fi
	cd -

	cp $HID_UPDATE_DIR/hid_update $APPFS_DIR/bin/
	echo "build app hid_update ok!"
}

# build appfs
build_appfs ()
{
	cp $UVC_CONFIG_FILE 	$APPFS_DIR/config/
	cp $APP_INIT_FILE 	$APPFS_DIR/init/
	cp $IMAGE_BIN_FILE 	$APPFS_DIR/etc/sensor/
	cp $RESOURCE_DIR/tools_t31/bin/$COMPILE_TYPE/lrz		$APPFS_DIR/bin/
	cp $RESOURCE_DIR/tools_t31/bin/$COMPILE_TYPE/lsz		$APPFS_DIR/bin/
	cp $RESOURCE_DIR/tools_t31/bin/$COMPILE_TYPE/adbd		$APPFS_DIR/bin/
	cp $RESOURCE_DIR/tools_t31/bin/$COMPILE_TYPE/anticopy		$APPFS_DIR/bin/
	cp $KIVA_DIR/config/uvc.attr		$APPFS_DIR/config/

	if [ $ADVANCED = "autoCenter" ]; then
		cp $KIVA_DIR/libs/mxu/lib/$COMPILE_TYPE/* $APPFS_DIR/lib/
		cp $KIVA_DIR/libs/faceDet/lib/$COMPILE_TYPE/* $APPFS_DIR/lib/
	elif [ $ADVANCED = "PTZ" ]; then
		cp $KIVA_DIR/libs/persondet/models/* $APPFS_DIR/bin/
		cp $KIVA_DIR/libs/persondet/Ingenic-IIAL/IVS/lib/$COMPILE_TYPE/*.so $APPFS_DIR/lib/
		cp $KIVA_DIR/libs/persondet/Ingenic-IIAL/MXU/lib/$COMPILE_TYPE/*.so $APPFS_DIR/lib/
	fi

	cp $KIVA_DIR/libs/lib_audio/lib/$COMPILE_TYPE/*.so $APPFS_DIR/lib/

	cd $CURRENT_DIR
	APPFS_NAME=appfs.jffs2
	if [ $ADVANCED = "autoCenter" -o \
		$ADVANCED = "PTZ" ]; then
		APPFS_SIZE=11520K
	else
		APPFS_SIZE=3328K
	fi
	ERASE_SIZE=32K
	PAGE_SIZE=256K

	echo " mkjffs2 start ..."
	./mkjffs2.sh $APPFS_DIR $APPFS_NAME $APPFS_SIZE $ERASE_SIZE $PAGE_SIZE
	if [ $? != 0 ]; then
		echo "mkjffs2 appfs faild"
		exit 1
	fi
	echo " mkjffs2 success !"
	mv $APPFS_NAME $FIRMWARE_TMP_DIR

	if [ $? != 0 ]; then
		echo "build appfs.jffs2 faild"
		exit 1
	fi
	cd -
	echo "build appfs ok!"
}


# pack all bin
pack_bin ()
{
	cp $ROOTFS $FIRMWARE_TMP_DIR

	########################################
	if [ $ADVANCED = "null" -o \
		$ADVANCED = "autoCenter" -o \
		$ADVANCED = "PTZ" ]; then
		echo uclibc------pack_bin
		dd if=$FIRMWARE_TMP_DIR/u-boot-with-spl.bin of=$OUT_DIR/$TARGET &&
		dd if=$FIRMWARE_TMP_DIR/uImage of=$OUT_DIR/$TARGET bs=1k seek=256 &&
		dd if=$FIRMWARE_TMP_DIR/root-${COMPILE_TYPE}-toolchain4.7.2-1.1.squashfs of=$OUT_DIR/$TARGET bs=1k seek=2816 &&
		dd if=$FIRMWARE_TMP_DIR/appfs.jffs2 of=$OUT_DIR/$TARGET bs=1k seek=4864
	elif [ $ADVANCED = "quickStart" ]; then
		echo glibc------pack_bin
		dd if=$FIRMWARE_TMP_DIR/iboot.bin of=$OUT_DIR/$TARGET &&
		dd if=$FIRMWARE_TMP_DIR/uImage of=$OUT_DIR/$TARGET bs=1k seek=32 &&
		dd if=$FIRMWARE_TMP_DIR/appfs.jffs2 of=$OUT_DIR/$TARGET bs=1k seek=4128
	elif [ $ADVANCED = "cdcUpdate" ]; then
		dd if=$FIRMWARE_TMP_DIR/iboot.bin of=$OUT_DIR/$TARGET &&
		dd if=$FIRMWARE_TMP_DIR/uImage of=$OUT_DIR/$TARGET bs=1k seek=32 &&
		dd if=$FIRMWARE_TMP_DIR/appfs.jffs2 of=$OUT_DIR/$TARGET bs=1k seek=6432
		dd if=$FIRMWARE_TMP_DIR/uImage.update of=$OUT_DIR/$TARGET bs=1k seek=9760 count=2816
	fi
	########################################

	echo "build t31 img ok!"

	echo "back up"
	cp $FIRMWARE_TMP_DIR/* $T31_UPDATE_FIRMWARE/

	echo
	ls -lh $OUT_DIR/$TARGET
}

#main
main ()
{
	if [ $ADVANCED = "quickStart" -o \
		$ADVANCED = "cdcUpdate" ]; then
		build_iboot
	else
		build_uboot
	fi

	build_uImage

	if [ $ADVANCED = "cdcUpdate" ]; then
		build_uImage_update
	fi

	build_txisp
	build_sensors
	build_audio
	build_avpu
	build_usbcamera_ko
	build_other_ko

	if [ $AF = "af" ]; then
		build_gsensor-dw9714_ko
	fi

	if [ $ADVANCED = "PTZ" ]; then
		build_step_motor_ko
	fi

	build_app
	build_hid_update
	build_appfs
	pack_bin
}

main

