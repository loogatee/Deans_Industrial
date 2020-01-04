#!/bin/bash
gvim -p Makefile \
include/cdc_cmds.h \
include/proj_common.h \
healthcheck.c \
Canned_Packets.c \
packet_recv.c \
serial_recv.c \
serial_send.c \
utils.c \
main.c \
lutils.lua

