
# build Kiva for T31
# $1: chip type (T31N / T31A / T31X / T31L)
# $2: sensor name,such as jxf37,jxf23,sc4335...
# $3: solution,such as k1-2m(e/d),k1-4m(_xxx)...
# $4: custorm name,such as base,Emeet,WaSheng...
# $5: HAA (disable / enable)
# $6: ACM (y / n)
# $7: uclibc or glibc
# $8: QUICK_START (y/n)
#build common uvc firmware:
# ./mkimg_for_kiva.sh T31X sc4335 k1-4m base disable n uclibc
# ./mkimg_for_kiva.sh T31L jxf23 k1-2m base disable n uclibc
# ./mkimg_for_kiva.sh T31L jxf37 k1-2me base disable n uclibc

#build update firmware:
	#1.flash capacity must >= 16M
	#2.insmod usbcamera.ko uvc_extension_en=1
	#3.modify the configration (pid,vid,serial number) as same as Anker
# ./mkimg_for_kiva_recovery.sh T31X sc4335 k1-4m base disable n uclibc n

#build k1-2m_track:
# ./mkimg_for_kiva.sh T31N jxf37 k1-2m_track base disable n uclibc
