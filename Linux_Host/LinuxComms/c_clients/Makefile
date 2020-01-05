CFLAGS   = -Wall
IFLAGS   = -I/usr/include/lua5.1 -I../include
LFLAGS   = -L/usr/lib/arm-linux-gnueabihf
PREFIX   =  /usr
GCC      = gcc
GCCARM   = arm-linux-gnueabihf-gcc

progs    = healthcheck Dloop AD_configonly gpioonly GetAD

all : $(progs)

distclean clean :
	rm -f $(progs)



healthcheck: healthcheck.c ../include/proj_common.h ../include/cdc_cmds.h
	$(GCC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $< -o $@ -lzmq -lpthread

GetAD: GetAD.c ../include/proj_common.h ../include/cdc_cmds.h
	$(GCC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $< -o $@ -lzmq

AD_configonly: AD_configonly.c ../include/proj_common.h ../include/cdc_cmds.h
	$(GCC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $< -o $@ -llua5.1

gpioonly: gpioonly.c ../include/proj_common.h ../include/cdc_cmds.h
	$(GCC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $< -o $@

Dloop: Dloop.c ../include/proj_common.h ../include/cdc_cmds.h
	$(GCC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $< -o $@ -lzmq -lpthread


%.o:  %.c
	$(GCC) $(CFLAGS) -c $(IFLAGS) $< -o $@















