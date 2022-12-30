
# build Kiva for T31
# $1: chip type (T31N / T31A / T31X / T31L)
# $2: sensor name,such as jxf37,jxf23,sc4335...
# $3: solution,such as k1-2m(e/d),k1-4m(_xxx)...
# $4: custorm name,such as base,Emeet,WaSheng...
# $5: HAA (disable / enable)
# $6: ACM (y / n)
# $7: uclibc or glibc

#./mkimg_for_kiva.sh T31X jxf37 k1-2m base disable n uclibc
#./mkimg_for_kiva.sh T31A sc4335 k1-4m_Jupiter Emeet disable n uclibc

#build for recovery
cp mkimg_for_kiva_anker.sh ../../../../../build/
cd ../../../../../build/
./mkimg_for_kiva_anker.sh T31X sc500ai k1-4m_C200 Anker disable n uclibc
rm mkimg_for_kiva_anker.sh
cd -

#build for update
cp mkimg_for_kiva_anker_ota.sh ../../../../../build/
cd ../../../../../build/
./mkimg_for_kiva_anker_ota.sh T31X sc500ai k1-4m_C200 Anker disable n uclibc
rm mkimg_for_kiva_anker_ota.sh
cd -
