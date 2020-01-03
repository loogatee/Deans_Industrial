/**
 * Copyright 2019  john reed
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not  use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations  under the License.
 */

#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include "proj_common.h"


static void setup_the_tty( int ttyfd );

int main(void)
{
    //lua_State *L;
    u8         lbuf[40];
    int        ttyfd,rc;
    void      *context,*BackChannel;

    pthread_t  SRecv_threadID;
    pthread_t  SSend_threadID;
    pthread_t  PRecv_threadID;
    pthread_t  PCent_threadID;
    int        rc1_dup;
    int        rc2_dup;

    pid_t      the_pid;
    FILE      *xfd;

    signal(SIGCHLD, SIG_IGN);
    if(fork()) {return 0;}                      // Parent returns. Child executes as background process

    printf("----Hello Johnny-----\n");

    mkdir("/tmp/zmqfeeds",0777);       // NOP if already existing
    chmod("/tmp/zmqfeeds",0777);       // needed?  Seems so.

    context     = zmq_ctx_new();
    BackChannel = zmq_socket ( context,     ZMQ_REP                  );
    rc          = zmq_bind   ( BackChannel, ZMQPORT_BACKCHANNEL_main );    assert(rc == 0);

    ttyfd = open(TTYPORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if( ttyfd == -1 )
    {
        printf("Open Failure on %s\n", TTYPORT);
        return -1;
    }

    setup_the_tty(ttyfd);
    rc1_dup = dup(ttyfd);
    rc2_dup = dup(ttyfd);


    if( pthread_create(&SRecv_threadID, 0, Serial_Recv, (void *)&rc1_dup) != 0 )
    {
        printf("pthread_create Failure on Serial_Recv\n");
        return -1;
    }

    if( pthread_create(&SSend_threadID, 0, Serial_Send, (void *)&rc2_dup) != 0 )
    {
        printf("pthread_create Failure on Serial_Send\n");
        return -1;
    }

    if( pthread_create(&PRecv_threadID, 0, PacketHandler, 0) != 0 )
    {
        printf("pthread_create Failure on PacketHandler\n");
        return -1;
    }

    if( pthread_create(&PCent_threadID, 0, PacketCentral, 0) != 0 )
    {
        printf("pthread_create Failure on PacketCentral\n");
        return -1;
    }

    the_pid = getpid();
    sprintf((char *)lbuf,"%d",the_pid);
    xfd = fopen("/var/run/di.pid","w");
    fwrite(lbuf,strlen((char *)lbuf),1,xfd);
    fclose(xfd);

    while(1)
    {
        zmq_recv( BackChannel, lbuf, 40, 0 );        // Just friggin wait
    }

    return 0;
}








static void setup_the_tty( int ttyfd )
{
    int              tmp;
    struct termios   options;

    //
    //    **  All this tty setup code leveraged from the ezV24 source code **
    //

    tmp = fcntl(ttyfd,F_GETFL,0);
    fcntl(ttyfd, F_SETFL, tmp & ~O_NDELAY);

    tcgetattr(ttyfd,&options);
    cfmakeraw(&options);

    //  This is what cfmakeraw does:
    //
    //  termios_p->c_iflag  &=  ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    //  termios_p->c_oflag  &=  ~OPOST;
    //  termios_p->c_lflag  &=  ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    //  termios_p->c_cflag  &=  ~(CSIZE | PARENB);
    //  termios_p->c_cflag  |=  CS8;


    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 4;

    options.c_iflag  &=  ~(IXON|IXOFF|IXANY|INPCK);

    options.c_cflag  &=  ~(CRTSCTS|HUPCL);
    options.c_cflag  |=  (CLOCAL|CREAD);

    cfsetispeed(&options,B9600);
    cfsetospeed(&options,B9600); 

    tcsetattr(ttyfd,TCSANOW,&options);
}
