#!/bin/sh

DRIVERS_KO_DIR=../drivers/release
SRC_DIR=./
CFG_PATH=../project_cfg
SCRITPS_DIR=../appfs/scripts
sync

OBJ=release
PRO=''
MODEL=''

usage()
{
	echo "***********************************************************"
	echo "USAGE:"
	echo "***********************************************************"
}

str_to_lower()
{
	echo $1 | tr '[A-Z]' '[a-z]'
}

str_to_upper()
{
	echo $1 | tr '[a-z]' '[A-Z]'
}

parse_param()
{
	echo $1
	str=$(str_to_lower $1)
	if [ "$str" == "clean" ] || [ "$str" == "release" ]  || [ "$str" == "distclean" ] || [ "$str" == "factory" ] ; then
		OBJ=$str
	elif [ "$str" == "e_2m" ] || [ "$str" == "e_4m" ] \
         || [ "$str" == "lbc_2m" ] || [ "$str" == "lbc_4m" ] \
         || [ "$str" == "lbc_2m_dtof" ] || [ "$str" == "lbc_4m_dtof" ] \
         || [ "$str" == "pro_2m" ] || [ "$str" == "pro_4m" ]  ; then
        PRO=$str
    elif [ "$str" == "overseas" ] || [ "$str" == "sell" ] || [ "$str" == "trade" ] ; then
        MODEL=$str
	else
		usage
		exit 1
	fi
}

#shell param parse
if [ $# -eq 0 ] ; then
	echo "Default setting:OBJ=${OBJ} CPU=${CPU} DEV=${DEV} DBG=${DBG}"
else	
	for param in $@
	do
	  parse_param $param
	done
fi

clean_ucamera()
{
    make clean
}

build_ucamera()
{
    if [ -e ucamera ] ; then
        clean_ucamera
    fi
	# 拷贝相关文件到该目录下
    # 拷贝机芯的库、头文件和libusbcamera.a
    cp -rf $CFG_PATH/$PRO/jx/libaf.a $SRC_DIR/lib/jx/libaf.a
	if [ "$?" != "0" ];then
		return 1
	fi
    cp -rf $CFG_PATH/$PRO/jx/Af_src.h $SRC_DIR/include/jx/Af_src.h
	if [ "$?" != "0" ];then
		return 1
	fi
    cp -rf $CFG_PATH/$PRO/model/$MODEL/libusbcamera.a $SRC_DIR/lib/ucam/libusbcamera.a
	if [ "$?" != "0" ];then
		return 1
	fi

    make  ARCH=mips CROSS_COMPILE=mips-linux-gnu-
    if [ $? -ne 0 ] ; then
        return 1
    else
        return 0
    fi
}

if [ "$OBJ" == "clean" ] || [ "$OBJ" == "distclean" ] ; then
	clean_ucamera	
else
	if [ "$PRO" == "" ] ; then
		echo "PRO is NULL"
		exit 1
	fi
    if [ "$MODEL" == "" ] ; then
		echo "MODEL is NULL"
		exit 1
	fi
    build_ucamera
    if [ "$?" != "0" ];then
        echo "***build ucamera failed!***"
        exit 1
    else
        echo "###build ucamera success###"
    fi
fi

exit 0