#!/bin/bash


dpkg -i /root/debs/sudo_1.8.16-0ubuntu1_armhf.deb
apt-get install -f

dpkg -i /root/debs/libgmp10_6.1.0+dfsg-2_armhf.deb
apt-get install -f

dpkg -i /root/debs/libidn11_1.32-3ubuntu1_armhf.deb
apt-get install -f

dpkg -i /root/debs/libmnl0_1.0.3-5_armhf.deb
apt-get install -f

dpkg -i /root/debs/libnettle6_3.2-1_armhf.deb
apt-get install -f

dpkg -i /root/debs/libffi6_3.2.1-4_armhf.deb
apt-get install -f

dpkg -i /root/debs/libhogweed4_3.2-1_armhf.deb
apt-get install -f

dpkg -i /root/debs/libp11-kit0_0.23.2-3_armhf.deb
apt-get install -f

dpkg -i /root/debs/libtasn1-6_4.7-3_armhf.deb
apt-get install -f

dpkg -i /root/debs/libgnutls30_3.4.10-4ubuntu1_armhf.deb
apt-get install -f

dpkg -i /root/debs/libgnutls-openssl27_3.4.10-4ubuntu1_armhf.deb
apt-get install -f

dpkg -i /root/debs/net-tools_1.60-26ubuntu1_armhf.deb
apt-get install -f

dpkg -i /root/debs/iproute2_4.3.0-1ubuntu3_armhf.deb
apt-get install -f

dpkg -i /root/debs/libisc-export160_9.10.3.dfsg.P4-8_armhf.deb
apt-get install -f

dpkg -i /root/debs/libdns-export162_9.10.3.dfsg.P4-8_armhf.deb
apt-get install -f


dpkg -i /root/debs/ifupdown_0.8.10ubuntu1_armhf.deb
apt-get install -f

#
#  This creates rootfs/etc/network/interfaces.d/lo:
#
/bin/bash -c "echo auto lo >       /etc/network/interfaces.d/lo"
sed -i "\$aiface lo inet loopback" /etc/network/interfaces.d/lo

#
#  This creates rootfs/etc/network/interfaces.d/eth0:
#
/bin/bash -c "echo auto eth0 >   /etc/network/interfaces.d/eth0"
sed -i "\$aiface eth0 inet dhcp" /etc/network/interfaces.d/eth0

dpkg -i /root/debs/isc-dhcp-client_4.3.3-5ubuntu12_armhf.deb
apt-get install -f

ifup -v eth0

sudo dpkg -i /root/debs/iputils-ping_20121221-5ubuntu2_armhf.deb
apt-get install -f


