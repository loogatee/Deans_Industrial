#!/bin/bash
gvim -p Makefile \
include/cdc_cmds.h \
include/proj_common.h \
Canned_Packets.c \
packet_recv.c \
serial_recv.c \
serial_send.c \
utils.c \
main.c \
lutils.lua

