#!/bin/sh

SRCDIR=$1
DST_ROOTFSNAME=$2
SIZE=$3
NOR_ERASE_SIZE=$4
NOR_PAGE_SIZE=$5
OUT=$PWD/$DST_ROOTFSNAME

if [ $# -ge 3 ]; then

	if [ $# -eq 5 ]; then
		ERASE_SIZE=`echo $NOR_ERASE_SIZE | awk -F 'K' '{print $1}'`
		ERASE_SIZE_10=`expr $ERASE_SIZE \* 1024`
		ERASE_SIZE_16=`echo "obase=16;$ERASE_SIZE_10"|bc`

		PAGE_SIZE=`echo $NOR_PAGE_SIZE | awk -F 'K' '{print $1}'`
		PAGE_SIZE_10=$PAGE_SIZE
		PAGE_SIZE_16=`echo "obase=16;$PAGE_SIZE_10"|bc`
		if [ $PAGE_SIZE_16 -le 1000 ]; then
			PAGE_SIZE_16=1000
		fi
	else
		ERASE_SIZE_16=8000
		PAGE_SIZE_16=1000
	fi

	if [ -f $OUT ]; then
		rm $OUT
	fi

	JFFS2_SIZE=`echo $SIZE | awk -F 'K' '{print $1}'`
	JFFS2_SIZE_10=`expr $JFFS2_SIZE \* 1024`
	JFFS2_SIZE_16=`echo "obase=16;$JFFS2_SIZE_10"|bc`

	find $SRCDIR -name .gitignore | xargs rm -vf

	mkfs.jffs2 -o $OUT -r $SRCDIR -e 0x$ERASE_SIZE_16 -s 0x$PAGE_SIZE_16 -n -l -X zlib
	USED=`ls -l $OUT | awk -F ' ' '{print $5}'`

	if [ $JFFS2_SIZE_10 -gt $USED ]; then

		mkfs.jffs2 -o $OUT -r $SRCDIR -e 0x$ERASE_SIZE_16 -s 0x$PAGE_SIZE_16 -n -l -X zlib --pad=0x$JFFS2_SIZE_16
		TOTAL=`ls -l $OUT | awk -F ' ' '{print $5}'`
		FREE=`expr $TOTAL - $USED`
		USED_KB=`expr $USED / 1024`
		TOTAL_KB=`expr $TOTAL / 1024`
		FREE_KB=`expr $FREE / 1024`
		echo " ---------  Size: $TOTAL_KB KB; used: $USED_KB KB; free:$FREE_KB KB  --------"
		ls -lh $OUT
		exit 0
	else
		rm $OUT
		echo "----------- the inputed size of jffs2 is too little ---------"
		exit 1
	fi
else
	echo $#
	echo "./mkjffs2.sh srcdir dstdir/imgname img_size [nor_erase_size] [nor_page_size]"
	exit 1
fi
