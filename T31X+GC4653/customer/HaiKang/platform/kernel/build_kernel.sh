#! /bin/bash

usage()
{
	echo "***********************************************************"
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
	str=$(str_to_lower $1)
	if [ "$str" == "clean" ] || [ "$str" == "release" ]  || [ "$str" == "distclean" ] || [ "$str" == "factory" ] ; then
		OBJ=$str
	elif [ "$str" == "kernel" ] || [ "$str" == "rootfs" ] || [ "$str" == "uboot" ] || [ "$str" == "modules" ] \
		|| [ "$str" == "all" ] ; then
		TARGET=$str
	elif [ "$str" == "ipc" ] || [ "$str" == "ipd" ] || [ "$str" == "ipcr" ] ; then
		DEV=$(str_to_upper $str)
	elif [ "$str" == "smp" ] || [ "$str" == "fh8856" ] || [ "$str" == "fh8852" ]; then
		CPU=$str
    elif [ "$str" == "g2" ]  || [ "$str" == "e6" ] || [ "$str" == "e7" ] || [ "$str" == "h7" ] || [ "$str" == "g3" ] || [ "$str" == "g4" ] || [ "$str" == "c2" ] \
	 || [ "$str" == "h8" ] || [ "$str" == "t31x" ] || [ "$str" == "t31l" ]; then
        PLAT=$str
	elif [ "$str" == "dbg" ] || [ "$str" == "debug" ]; then
		DBG=$str
	elif [ "$str" == "host" ] || [ "$str" == "slave" ]; then
		 MSC=$str
	else
		usage
		exit 1
	fi
}

#shell param parse
if [ $# -eq 0 ] ; then
	echo "Default setting:OBJ=${OBJ} PLAT=${PLAT} DEV=${DEV} DBG=${DBG}"
else	
	for param in $@
	do
	  parse_param $param
	done
fi

clean_kernel()
{
	make distclean
	if [ -f uImage ] ; then
		rm -fr uImage
	fi
	if [ -f arch/mips/boot/vmlinux.bin.lzma ] ; then
		rm -fr arch/mips/boot/vmlinux.bin.lzma
	fi
	if [ -f arch/mips/boot/uImage.lzma ] ; then
		rm -fr arch/mips/boot/uImage.lzma
	fi
}

build_kernel()
{
	make ARCH=mips CROSS_COMPILE=mips-linux-gnu- -j16 uImage
	if [ $? -ne 0 ]; then
		return 1
	fi
}

if [ "$OBJ" == "clean" ] || [ "$OBJ" == "distclean" ]; then
	clean_kernel
else
	build_kernel
	if [ "$?" != "0" ];then
		echo "***build $PLAT kernel failed!***"
		exit 1
	else
		echo "###build $PLAT kernel success###"
		cp ./arch/mips/boot/uImage.lzma ./uImage
	fi
fi

exit 0
