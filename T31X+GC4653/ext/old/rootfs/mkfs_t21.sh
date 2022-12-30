#!/bin/sh

ROOTFS_DIR=./rootfs
SCRITPS_DIR=./scripts
OUT_DTR=./out
RES_DIR=./res

if [ "$2" = "uclibc" ]; then
	echo --- mips-linux-uclibc-gun-strip ---
	if [ "$3" = "t02" ]; then
		COMPLIE_STRIP=/opt/mips-gcc540-glibc222-for-t02/bin/mips-linux-uclibc-gnu-strip
	else
		COMPLIE_STRIP=/home_a/ljtong/mips-gcc472/mips-gcc472-glibc216-64bit/bin/mips-linux-uclibc-gnu-strip
	fi
#else
#	echo --- mips-linux-gun-strip ---
#	if [ "$3" = "t02" ]; then
#		COMPLIE_STRIP=/opt/mips-gcc540-glibc222-for-t02/bin/mips-linux-gnu-strip
#	else
#		COMPLIE_STRIP=mips-linux-gnu-strip
#	fi
fi

help() {
    echo "Useage:"
    echo "    $0 <config>"
    echo "    exp:"
    echo "        $0 configs/demo_spi_nor_16M_config glibc"
}

ERASE_SIZE=
PAGE_SIZE=
NAME=
FS_TYPE=
LIBC_TYPE=
SIZE=
IS_ROOT=
BUSYBOX=
STRIPPED=
READY_COPY=no
PREPARE_FS=no
prepare_fs() {
    echo "prepare fs ... $1, $2"
    DST_DIR=$OUT_DTR/$1
    mkdir $DST_DIR
    if [ $IS_ROOT = yes ]; then
	ROOT_SRC=$ROOTFS_DIR/$2
	echo "BUSYBOX = $BUSYBOX"
	if [ "$BUSYBOX" = "" ]; then
	    echo "ERROR: rootfs not specified busybox"
	    echo "       please set BUSYBOX=xxx in config"
	    exit 1
	fi
	BUSYBOX_SRC=$ROOTFS_DIR/base/$BUSYBOX
	cp -rf $ROOT_SRC/* $DST_DIR
	cp -rf $BUSYBOX_SRC/* $DST_DIR
	if [ $STRIPPED = yes ]; then
	    $COMPLIE_STRIP $DST_DIR/bin/*
		#echo "$COMPLIE_STRIP $DST_DIR/bin/*"
	fi
    fi
}

res_copy() {
    echo "resouse copy ... $1"
    SRC=$RES_DIR/`echo "$1" | awk -F '>' '{gsub(/[[:blank:]]*/,"",$1); print $1}'`
    DST=$OUT_DTR/$2/`echo "$1" | awk -F '>' '{gsub(/[[:blank:]]*/,"",$2); print $2}'`
    if [ ! -d $DST ]; then
	mkdir -p $DST
    fi
    cp -rf $SRC $DST
}

create_fs_img() {
    echo "create fs img ... $1, $2, $3"
    SRC_DIR=$OUT_DTR/$1
    IMG_NAME=$OUT_DTR/$1.$2
    IMG_SIZE=$3
    SCRITPS=$SCRITPS_DIR/mk$2.sh
	if [ $STRIPPED = yes ]; then
		find $SRC_DIR -name "*.so*" | xargs $COMPLIE_STRIP
		#echo "find $SRC_DIR -name "*.so*" | xargs $COMPLIE_STRIP"
	fi
    echo "$SCRITPS $SRC_DIR $IMG_NAME $IMG_SIZE $ERASE_SIZE $PAGE_SIZE"
    $SCRITPS $SRC_DIR $IMG_NAME $IMG_SIZE $ERASE_SIZE $PAGE_SIZE
	if [ $? != 0 ]; then
		echo "------- ERROR ------"
		exit 1
	fi
}

pares_partition() {
    KEY=`echo "$1" | awk -F '=' '{gsub(/[[:blank:]]*/,"",$1); print $1}'`
    if [ "$KEY" = "NAME" ]; then
	NAME=`echo "$1" | awk -F '=' '{gsub(/[[:blank:]]*/,"",$2); print $2}'`
    elif [ "$KEY" = "FS_TYPE" ]; then
	FS_TYPE=`echo "$1" | awk -F '=' '{gsub(/[[:blank:]]*/,"",$2); print $2}'`
    elif [ "$KEY" = "LIBC_TYPE" ]; then
	LIBC_TYPE=`echo "$1" | awk -F '=' '{gsub(/[[:blank:]]*/,"",$2); print $2}'`
    elif [ "$KEY" = "SIZE" ]; then
	SIZE=`echo "$1" | awk -F '=' '{gsub(/[[:blank:]]*/,"",$2); print $2}'`
    elif [ "$KEY" = "IS_ROOT" ]; then
	IS_ROOT=`echo "$1" | awk -F '=' '{gsub(/[[:blank:]]*/,"",$2); print $2}'`
    elif [ "$IS_ROOT" = "yes" ] && [ "$KEY" = "BUSYBOX" ]; then
	BUSYBOX=`echo "$1" | awk -F '=' '{gsub(/[[:blank:]]*/,"",$2); print $2}'`
    elif [ "$KEY" = "STRIPPED" ]; then
	STRIPPED=`echo "$1" | awk -F '=' '{gsub(/[[:blank:]]*/,"",$2); print $2}'`
    elif [ "$KEY" = "ERASE_SIZE" ]; then
	ERASE_SIZE=`echo "$1" | awk -F '=' '{gsub(/[[:blank:]]*/,"",$2); print $2}'`
    elif [ "$KEY" = "PAGE_SIZE" ]; then
	PAGE_SIZE=`echo "$1" | awk -F '=' '{gsub(/[[:blank:]]*/,"",$2); print $2}'`
    fi

    if [ "$NAME" != "" ] && [ "$FS_TYPE" != "" ] && [ "$SIZE" != "" ] && [ "$IS_ROOT" != "" ] && [ "$STRIPPED" != "" ]; then
	if [ $PREPARE_FS = no ]; then
	    prepare_fs $NAME $LIBC_TYPE
	    PREPARE_FS=yes
	fi
	if [ "$KEY" = "COPYLIST:" ]; then
	    READY_COPY=yes
	elif [ $READY_COPY = yes ]; then
	    res_copy "$1" $NAME
	fi
    fi
}

parse_config() {
    echo "pares config file $1"
    while read LINE
    do
	NOTES_LINE=`echo $LINE | sed -n '/^#/p'`
	if [ "$NOTES_LINE" = "" ]; then
	    if [ "$LINE" = "PARTITION:" ]; then
		if [ "$NAME" != "" ] &&
		    [ "$FS_TYPE" != "" ] &&
		    [ "$SIZE" != "" ]; then
		    create_fs_img $NAME $FS_TYPE $SIZE
		fi
		NAME=
		FS_TYPE=
		LIBC_TYPE=
		SIZE=
		IS_ROOT=
		STRIPPED=
		READY_COPY=no
		PREPARE_FS=no
	    elif [ "$LINE" = "SPI_NOR_INFO" ]; then
		ERASE_SIZE=
		PAGE_SIZE=
	    elif [ "$LINE" != "" ]; then
		pares_partition "$LINE"
	    fi
	fi
    done < $1
    if [ "$NAME" != "" ] &&
	[ "$FS_TYPE" != "" ] &&
	[ "$SIZE" != "" ]; then
	create_fs_img $NAME $FS_TYPE $SIZE
    fi
}

CONFIG_FILE=$1
if [ -f $CONFIG_FILE ]; then
	if [ -d $OUT_DTR ]; then
		rm -rf $OUT_DTR/*
		echo "rm $OUT_DTR -rf"
	else
		mkdir $OUT_DTR
		echo "mkdir $OUT_DTR"
	fi
	parse_config $CONFIG_FILE
else
	echo "ERROR: can't find config file $CONFIG_FILE"
	help
fi

TOP_DIR=$(pwd)
OUT=$TOP_DIR/out
DATA=$(date +%y%m%d)
T21_FIRMWARE_DIR=$TOP_DIR/firmware/t21/
RELEASE_DIR=$(pwd)/release/t21/
if [ "$2" != "" ]; then
	TARGET=T21N_firmware_for_USB_Camera_$2_$DATA.bin
else
	TARGET=T21N_firmware_for_USB_Camera_$DATA.bin

fi

dd if=$T21_FIRMWARE_DIR/uboot of=$OUT/$TARGET &&
dd if=$T21_FIRMWARE_DIR/uImage of=$OUT/$TARGET bs=1k seek=256 &&
dd if=$T21_FIRMWARE_DIR/rootfs of=$OUT/$TARGET bs=1k seek=2816 &&
dd if=$OUT/appfs.jffs2 of=$OUT/$TARGET bs=1k seek=4864
cp $OUT/$TARGET $RELEASE_DIR
