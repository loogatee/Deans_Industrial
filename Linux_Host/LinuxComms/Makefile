CFLAGS   = -Wall
IFLAGS   = -I/usr/include/lua5.1 -I./include
LFLAGS   = -L/usr/lib/arm-linux-gnueabihf
PREFIX   =  /usr
GCC      = gcc
GCCARM   = arm-linux-gnueabihf-gcc

progs    = cdc3 healthcheck
OBJS     = main.o serial_recv.o serial_send.o packet_recv.o PacketCentral.o

all : $(progs)

distclean clean :
	rm -f $(progs) $(OBJS) AD_configonly gpioonly



cdc3: $(OBJS)
	$(GCC) $(LFLAGS) $^ -o $@ -lzmq -lpthread -llua5.1

healthcheck: healthcheck.c include/proj_common.h include/cdc_cmds.h
	$(GCC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $< -o $@ -lzmq -lpthread

AD_configonly: AD_configonly.c include/proj_common.h include/cdc_cmds.h
	$(GCC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $< -o $@ -llua5.1

gpioonly: gpioonly.c include/proj_common.h include/cdc_cmds.h
	$(GCC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $< -o $@

Dloop: Dloop.c include/proj_common.h include/cdc_cmds.h
	$(GCC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $< -o $@ -lzmq -lpthread


%.o:  %.c
	$(GCC) $(CFLAGS) -c $(IFLAGS) $< -o $@


serial_recv.o:   serial_recv.c   include/proj_common.h include/cdc_cmds.h
serial_send.o:   serial_send.c   include/proj_common.h
packet_recv.o:   packet_recv.c   include/proj_common.h include/cdc_cmds.h include/utils.h
main.o:          main.c          include/proj_common.h
utils.o:         utils.c         include/proj_common.h include/utils.h
PacketCentral.o: PacketCentral.c include/proj_common.h include/cdc_cmds.h














