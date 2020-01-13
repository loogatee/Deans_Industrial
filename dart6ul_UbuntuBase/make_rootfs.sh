#!/bin/sh
set -x

# Additions:
#    - can remove some ttyX conf's in /etc.   Remove them all.
#

HOSTNAME="dartsy"


#UBUNTU_NAME="ubuntu-base-16.04.4-base-armhf.tar.gz"
UBUNTU_NAME="ubuntu-base-16.04.6-base-armhf.tar.gz"


sudo rm -rf rootfs
mkdir rootfs

#
#  If it's there in /tmp, then grab it
#  Else get if from the repo
#
if test -f /tmp/$UBUNTU_NAME
then
    cp /tmp/$UBUNTU_NAME .
    TMP1=0
else
    wget http://cdimage.ubuntu.com/ubuntu-base/releases/16.04.6/release/$UBUNTU_NAME
    TMP1=1
fi


cd rootfs; sudo tar --numeric-owner -xzvf ../$UBUNTU_NAME; cd ..


sudo cp Custom_Files/1stboot_config.sh               rootfs/root
sudo cp Custom_Files/install_openssh.sh              rootfs/root
sudo cp Custom_Files/more_packages.sh                rootfs/root
sudo cp Custom_Files/markdown                        rootfs/usr/bin

sudo cp Custom_Files/debian-emmc.sh                  rootfs/usr/sbin
sudo cp Custom_Files/mkfs.fat                        rootfs/sbin
sudo ln -s /sbin/mkfs.fat                            rootfs/sbin/mkfs.vfat
sudo ln -s /lib/systemd/system/getty@.service        rootfs/etc/systemd/system/getty.target.wants/getty@ttymxc0.service

sudo rm -f                                           rootfs/lib/systemd/system/getty-static.service


#sudo cp Custom_Files/sudoers                         rootfs/etc
#sudo chown root:root rootfs/etc/sudoers
#sudo chmod 440       rootfs/etc/sudoers


sudo mkdir -p                                        rootfs/usr/share/lua/5.1



#---------------  luafcgid
sudo mkdir                                           rootfs/etc/luafcgid
sudo mkdir                                           rootfs/var/log/luafcgid

sudo cp Custom_Files/LuaFCGI/luafcgidWDEBUG          rootfs/usr/bin
sudo cp Custom_Files/LuaFCGI/luafcgid.init           rootfs/etc/init.d/luafcgid
sudo cp Custom_Files/LuaFCGI/luafcgid.lua            rootfs/usr/share/lua/5.1
sudo cp Custom_Files/LuaFCGI/config.lua              rootfs/etc/luafcgid
sudo ln -s /usr/bin/luafcgidWDEBUG                   rootfs/usr/bin/luafcgid

sudo /bin/bash -c "echo '  ' >  rootfs/var/log/luafcgid/luafcgid.log"
sudo chmod 777                                       rootfs/var/log/luafcgid/luafcgid.log



#---------------  lzmq
sudo mkdir -p                                        rootfs/usr/local/lib/lua/5.1
sudo mkdir -p                                        rootfs/usr/local/share/lua/5.1/lzmq

sudo rsync -avD Custom_Files/lzmq/lzmq1/*            rootfs/usr/local/lib/lua/5.1
sudo rsync -avD Custom_Files/lzmq/lzmq2/*            rootfs/usr/local/share/lua/5.1/lzmq


#---------------  Monkey
sudo mkdir                                           rootfs/var/log/monkey
sudo mkdir                                           rootfs/etc/monkey
sudo mkdir                                           rootfs/usr/lib/monkey
sudo mkdir                                           rootfs/usr/share/monkey

sudo /bin/bash -c "echo '  ' >  rootfs/var/log/monkey/access.log"
sudo /bin/bash -c "echo '  ' >  rootfs/var/log/monkey/error.log"
sudo /bin/bash -c "echo '  ' >  rootfs/var/log/monkey/master.log"

sudo chmod 777                                       rootfs/var/log/monkey/access.log
sudo chmod 777                                       rootfs/var/log/monkey/error.log
sudo chmod 777                                       rootfs/var/log/monkey/master.log


sudo rsync -avD Custom_Files/Monkey/etc_monkey/*     rootfs/etc/monkey
sudo rsync -avD Custom_Files/Monkey/www/*            rootfs/usr/share/monkey

sudo cp Custom_Files/Monkey/monkey.init              rootfs/etc/init.d/monkey
sudo cp Custom_Files/Monkey/monkey.service           rootfs/lib/systemd/system
sudo cp Custom_Files/Monkey/sbin/*                   rootfs/usr/sbin
sudo cp Custom_Files/Monkey/so/*                     rootfs/usr/lib/monkey

#---------------  cdc3

sudo cp Custom_Files/cdc3/*                          rootfs/usr/local/bin
sudo cp Custom_Files/cdc3/cdc3_initd                 rootfs/etc/init.d/cdc3


#sudo rm -f                                          rootfs/etc/init/whatever*


sudo cp Custom_Files/udev/hid2hci                    rootfs/lib/udev
sudo cp Custom_Files/udev/libin*                     rootfs/lib/udev
sudo rsync -avD Custom_Files/udev/rules.d            rootfs/lib/udev
sudo rsync -avD Custom_Files/udev/hwdb.d             rootfs/lib/udev

sudo rsync -avD Custom_Files/firmware                rootfs/lib
sudo rsync -avD Custom_Files/debs                    rootfs/root


#
#   Removes passwd from root
#
sudo sed -i "s/root:x:0/root::0/g" rootfs/etc/passwd

#
#   Creates /etc/resolv.conf
#
#sudo rm -f                                          rootfs/etc/resolv.conf
#sudo /bin/bash -c "echo nameserver $NAMESERVER >   rootfs/etc/resolv.conf"
#sudo ln -s /var/run/resolvconf/resolv.conf          rootfs/etc/resolv.conf

#
#   Creates /etc/hostname
#
sudo /bin/bash -c "echo $HOSTNAME >   rootfs/etc/hostname"

#
#   Creates /etc/fstab
#
sudo sed -i "\$aproc     /proc      proc      defaults     0   0" rootfs/etc/fstab

#
#    An example of a simple addition to an existing file
#    Add to the skel file also
#    
sudo sed -i "\$aalias dir='ls -CF'" rootfs/root/.bashrc
sudo sed -i "\$aalias dir='ls -CF'" rootfs/etc/skel/.bashrc



#
#    Can remove the original tarball,
#    or copy to /tmp if it's not there yet
#
if test $TMP1 = 1
then
    mv ./$UBUNTU_NAME /tmp
else
    rm ./$UBUNTU_NAME
fi
