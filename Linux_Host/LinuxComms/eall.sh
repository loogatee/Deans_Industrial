#!/bin/bash
gvim -p Makefile \
cdc_cmds.h \
proj_common.h \
healthcheck.c \
PacketCentral.c \
packet_recv.c \
serial_recv.c \
serial_send.c \
utils.c \
main.c \
lutils.lua

