##! /bin/sh

##################################
# $1 : chip_type   (T31N/T31L/T31X/T31A)
# $2 : sensor type (jxf23/jxf37/sc4335)
# $3 : customer	   (base)
# $4 : solution    (k1-2m/k1-2me/k1-4m)
# $5 : compiler    (glibc/uclibc)
# $6 : af	   (af/null)
# $7 : advanced	   (quickStart/autoCenter/PTZ/update/null)
##################################

CURRENT_DIR=$(pwd)


echo build firmware for kiva base
#Three Base Project: k1-2m/k1-2me/k1-4m
# ./mkimg_t31_for_kiva.sh T31L jxf23 base k1-2m uclibc null null
# ./mkimg_t31_for_kiva.sh T31L jxf37 base k1-2me uclibc af null
# ./mkimg_t31_for_kiva.sh T31X sc4335 base k1-4m uclibc af null

#Advanced project: quickstart/autocenter
# ./mkimg_t31_for_kiva.sh T31L jxf23 base k1-2m uclibc null quickStart
# ./mkimg_t31_for_kiva.sh T31X jxf23 base k1-2m uclibc null autoCenter
echo build firmware for kiva ok

