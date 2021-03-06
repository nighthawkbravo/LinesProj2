#!/bin/bash
BR_NAME=buildroot-2021.02
BR_FILE=${BR_NAME}.tar.bz2
BR_DL=${BR_FILE}
set -e
if [ ! -f ${BR_DL} ] || ! ( bzip2 -q -t ${BR_DL}); then
  (  
     rm -f ${BR_FILE}
     wget https://buildroot.org/downloads/${BR_FILE}
  )
fi
tar -xjf ${BR_DL}
cp BR_config ${BR_NAME}/.config
cp -vr package/* ${BR_NAME}/package/
cd ${BR_NAME}

for i in ../patches/* ; do
   patch -p1 < $i
done
time make -j$(nproc)
