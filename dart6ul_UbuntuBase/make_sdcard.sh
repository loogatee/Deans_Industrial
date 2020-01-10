#!/bin/bash
set -x

sudo umount /dev/sdb1
sudo umount /dev/sdb2

sudo rm -rf /opt/var_mx6ul_dart_debian/rootfs/*

pushd .
cd /opt/var_mx6ul_dart_debian

sudo ./make_var_mx6ul_dart_debian.sh -c modules

popd

sudo rsync -avD rootfs /opt/var_mx6ul_dart_debian


pushd .
cd /opt/var_mx6ul_dart_debian


sudo ./make_var_mx6ul_dart_debian.sh -c rtar
sudo ./make_var_mx6ul_dart_debian.sh -c rubi


sudo ./make_var_mx6ul_dart_debian.sh -c sdcard -d /dev/sdb

popd

sleep 1

[ ! -d "/media/johnr/rootfs" ] && sudo mkdir "/media/johnr/rootfs"

sudo mount -t ext4 -o rw,nosuid,nodev,relatime,data=ordered,uhelper=udisks2 /dev/sdb2 /media/johnr/rootfs

[ ! -d "/media/johnr/BOOT-VARSOM" ] && sudo mkdir "/media/johnr/BOOT-VARSOM"

sudo mount -t vfat -o rw,nosuid,nodev,relatime,uid=1000,gid=1000,fmask=0022,dmask=0022,codepage=437,iocharset=iso8859-1,shortname=mixed,showexec,utf8,flush,errors=remount-ro,uhelper=udisks2 /dev/sdb1 /media/johnr/BOOT-VARSOM

sleep 1

rm /media/johnr/BOOT-VARSOM/*concerto*

cp Custom_Files/Deans_dtb/* /media/johnr/BOOT-VARSOM

sudo cp Custom_Files/Deans_dtb/* /media/johnr/rootfs/opt/images/Debian

sudo umount /dev/sdb1
sudo umount /dev/sdb2
sudo rmdir /media/johnr/rootfs
sudo rmdir /media/johnr/BOOT-VARSOM

sudo eject /dev/sdb


