#!/bin/sh
if [ $# -ge 3 ]; then

SRCDIR=$1
imagename=$2
imagesize=`echo $3 | awk -F 'K' '{print $1}'`
echo 8888888888 $SRCDIR;
echo 8888888888 $imagename;
echo 8888888888 $imagesize;
echo $(pwd);

GENEXT2FS=./tools/genext2fs
OUTPUT=$PWD/$imagename.img
num_blocks=`du -sk $SRCDIR | tail -n1 | awk '{print $1;}'`
num_inodes=`find $SRCDIR | wc -l`
if [ $num_blocks -gt $imagesize ];then
    echo "Image limit $3 is too small, at least [$num_blocks]KB needed"
    exit 1
fi
reserve_blocks=`expr $imagesize - $num_blocks`
reserve_inodes=$num_inodes
num_blocks=`expr $num_blocks + $reserve_blocks`
num_inodes=`expr $num_inodes + $reserve_inodes`

# NOTE: default blocksize = 1KB
echo "$GENEXT2FS -d $SRCDIR -b $num_blocks -I $num_inodes $OUTPUT"
$GENEXT2FS -d $SRCDIR -b $num_blocks -I $num_inodes $OUTPUT
e2fsck -fy $OUTPUT
tune2fs -j $OUTPUT
tune2fs -O extents,uninit_bg,dir_index $OUTPUT
e2fsck -fy $OUTPUT
fsck.ext4 -fy $OUTPUT
echo "$OUTPUT"
else
echo "./mkext4.sh <srcdir> <image_name> <image_size> [ ... ]"
fi
