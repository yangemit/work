
# build Kiva for T31_TC
# $1: chip type (T31N / T31A / T31X / T31L)
# $2: sensor name,such as gc4653,...(must append suffix: _2m/_4m)
# $3: solution,such as k1-2m(e/d),k1-4m(_xxx)...
# $4: custorm name,such as base,Emeet,WaSheng...
# $5: HAA (disable / enable)
# $6: ACM (y / n)
# $7: uclibc or glibc

CURRENT_DIR=$(pwd)
OUT_TEMP=$CURRENT_DIR/out_temp
FIRMWARE=$CURRENT_DIR/firmware
BUILD_DIR=$CURRENT_DIR/../../../../../build
BUILD_SH=mkimg_for_kiva_tc.sh

ARGUMENT='T31X gc4653 k1-4m_W100 TianCai disable n uclibc'

############# Prepare ###############
if [ -d "$OUT_TEMP" -a -d "$FIRMWARE" ]; then
	rm $OUT_TEMP  $FIRMWARE -rf
fi
mkdir -p $OUT_TEMP/firmware
mkdir -p $FIRMWARE

########### Start Build #############
cp $BUILD_SH $BUILD_DIR
cd $BUILD_DIR
source $BUILD_DIR/$BUILD_SH $ARGUMENT
rm $BUILD_DIR/$BUILD_SH
cd -
############## End ##################


