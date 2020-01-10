#!/bin/bash

./debs/install.sh

apt-get update

apt-get -y  install apt-utils
apt-get -y  install dialog
apt-get -y  install locales

locale-gen en_US.UTF-8
dpkg-reconfigure locales


apt-get -y  install lua5.1
apt-get -y  install liblua5.1-0-dev
apt-get -y  install libfcgi-dev
apt-get -y  install rsync
apt-get -y  install libffi6
apt-get -y  install libffi-dev
apt-get -y  install lua-socket
apt-get -y  install lua-bitop
apt-get -y  install luarocks
apt-get -y  install libzmq5
apt-get -y  install libzmq3-dev


#
#    For getting luafcgid started up properly
#
ln -s /usr/bin/luafcgidWDEBUG /usr/bin/luafcgid
ln -s /etc/init.d/luafcgid /etc/rc0.d/K01luafcgid
ln -s /etc/init.d/luafcgid /etc/rc1.d/K01luafcgid
ln -s /etc/init.d/luafcgid /etc/rc2.d/S01luafcgid
ln -s /etc/init.d/luafcgid /etc/rc3.d/S01luafcgid
ln -s /etc/init.d/luafcgid /etc/rc4.d/S01luafcgid
ln -s /etc/init.d/luafcgid /etc/rc5.d/S01luafcgid
ln -s /etc/init.d/luafcgid /etc/rc6.d/K01luafcgid

#
#   cdc3 hostif stuff
#
ln -s /etc/init.d/cdc3 /etc/rc0.d/K01cdc3
ln -s /etc/init.d/cdc3 /etc/rc1.d/K01cdc3
ln -s /etc/init.d/cdc3 /etc/rc2.d/S01cdc3
ln -s /etc/init.d/cdc3 /etc/rc3.d/S01cdc3
ln -s /etc/init.d/cdc3 /etc/rc4.d/S01cdc3
ln -s /etc/init.d/cdc3 /etc/rc5.d/S01cdc3
ln -s /etc/init.d/cdc3 /etc/rc6.d/K01cdc3

#
#   monkey
#
ln -s /etc/init.d/monkey /etc/rc0.d/K01monkey
ln -s /etc/init.d/monkey /etc/rc1.d/K01monkey
ln -s /etc/init.d/monkey /etc/rc2.d/S01monkey
ln -s /etc/init.d/monkey /etc/rc3.d/S01monkey
ln -s /etc/init.d/monkey /etc/rc4.d/S01monkey
ln -s /etc/init.d/monkey /etc/rc5.d/S01monkey
ln -s /etc/init.d/monkey /etc/rc6.d/K01monkey

update-rc.d monkey defaults
update-rc.d monkey enable
















