#!/bin/bash

#################################
###      QUICK_START support         ###
#################################
SOLUTION=$1
if [ -z $SOLUTION ]; then
	SOLUTION=2M_CAMERA
fi

CUR_DIR=$(pwd)
TOP_DIR=$(pwd)/../
DATE=$(date +%Y%m%d)
RELEASE_SDK_NAME=KIVA-For-${SOLUTION}-${DATE}
RELEASE_SDK_DIR=${CUR_DIR}/${RELEASE_SDK_NAME}/
cd $CUR_DIR
mkdir -p ${RELEASE_SDK_NAME}
cp -rf ../ucamcorelib/libucam ${RELEASE_SDK_DIR}
cp -rf ../driver/usbcamera/libdriver ${RELEASE_SDK_DIR}
cp ./VERSION ${RELEASE_SDK_DIR}

#Finally tree it.
#tree
#compress
7z a ${RELEASE_SDK_NAME}.7z ${RELEASE_SDK_NAME}
