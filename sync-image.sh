#!/bin/sh

kpartx -a ${MESON_SOURCE_ROOT}/tools/image.hdd 

mkdir -p /tmp/wyrmos_root
mount /dev/mapper/loop0p2 /tmp/wyrmos_root 

mkdir -p /tmp/wyrmos_boot
mount /dev/mapper/loop0p1 /tmp/wyrmos_boot
mkdir -p /tmp/wyrmos_boot/boot

cp -r ${MESON_SOURCE_ROOT}/run/boot    /tmp/wyrmos_boot/
cp -r ${MESON_SOURCE_ROOT}/run/include /tmp/wyrmos_root/
cp -r ${MESON_SOURCE_ROOT}/run/lib     /tmp/wyrmos_root/

umount /tmp/wyrmos_boot
umount /tmp/wyrmos_root

kpartx -d ${MESON_SOURCE_ROOT}/tools/image.hdd

rm -rf /tmp/wyrmos_boot
rm -rf /tmp/wyrmos_root

sync
