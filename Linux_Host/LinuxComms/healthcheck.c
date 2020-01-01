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
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include "proj_common.h"
#include "cdc_cmds.h"


//#ifdef NOTNOW
typedef struct
{
    timeval_t  Time0;
} Globs;

static Globs  Globals;
//#endif

/*
static int   do_comms( char *Channel, char *showstr )
{
    char  buffer[40];
    void *context,*requester;
    int   rc,len;

    context   = zmq_ctx_new();
    requester = zmq_socket ( context,   ZMQ_REQ );
    rc        = zmq_connect( requester, Channel );   assert(rc == 0);
    
    buffer[0] = 0x6d;                                     // 6d is the Magic Header Byte
    buffer[1] = 0x01;                                     //    01:  "ok, TotBytes, zmqsend_errors"
    rc = zmq_send( requester, buffer,  2, ZMQ_NOBLOCK );
    len = zmq_recv( requester,  buffer, 40, 0 );

    buffer[len] = 0;
    printf( "%s:  %s\n", showstr, buffer);
    zmq_close(requester);
    zmq_ctx_destroy(context);

    return 0;
} */

//#ifdef NOTNOW
static int  Wait_On_Response(void *Chan, int millis, char *pbuf, int llen)
{
    zmq_pollitem_t  Pitems[1];

    Pitems[0].socket  = Chan;
    Pitems[0].fd      = 0;
    Pitems[0].events  = ZMQ_POLLIN;
    Pitems[0].revents = 0;

    if( zmq_poll(Pitems,1,millis) <= 0 ) { return -1; }

    return zmq_recv( Chan, pbuf, llen, 0 );
}


static void    start_timestamp(void)
{
    timeval_t tv;

    gettimeofday(&tv, NULL);

    Globals.Time0.tv_sec  = tv.tv_sec;
    Globals.Time0.tv_usec = tv.tv_usec;
}

/*
static uint get_timediff(void)
{
    timeval_t  Time1,Time2;

    gettimeofday(&Time1, NULL);

    if( Time1.tv_usec < Globals.Time0.tv_usec )
    {
        Time1.tv_usec += 1000000;
        Time1.tv_sec  -= 1;
    }

    Time2.tv_sec  = Time1.tv_sec  - Globals.Time0.tv_sec;
    Time2.tv_usec = Time1.tv_usec - Globals.Time0.tv_usec;

    return (uint)((Time2.tv_sec * 1000) + (Time2.tv_usec / 1000));
} */

//
//  #define ZMQPORT_CMDCHANNEL     "ipc:///tmp/zmqfeeds/CmdChannel"
//
static int   dloop_trigger( void )
{
    char    buffer[80];
    void   *context,*requester;
    int     rc,len;
    //u16    *bptr;

    //bptr = (u16 *)buffer;

    context   = zmq_ctx_new();
    requester = zmq_socket ( context,   ZMQ_REQ            );
    rc        = zmq_connect( requester, ZMQPORT_CMDCHANNEL );   assert(rc == 0);
    
    buffer[0] = CDCCMD_DLOOP;
    buffer[1] = 0x00;
    buffer[2] = 164;
    buffer[3] = 1;

    start_timestamp();

    zmq_send( requester,buffer,4,0 );

    len = Wait_On_Response( requester, -1, buffer, 80 );     // -1 is wait forever

    if( len > 0 )
    {
        buffer[len] = 0;
        //printf( "dloop result:  %s,x%dx\n", buffer,get_timediff());
        printf( "dloop result:  %s\n", buffer);
    }
    else
    {
        printf( "dloop ERROR, Wait_On_Response\n");
    }

    zmq_close(requester);
    zmq_ctx_destroy(context);

    return 0;
}
//#endif

int main( void )
{
    //do_comms(ZMQPORT_BACKCHANNEL_1, "Serial_Recv_Thread");
    //do_comms(ZMQPORT_BACKCHANNEL_2, "PacketCentral_Thread");
    //do_comms(ZMQPORT_BACKCHANNEL_3, "Serial_Send_Thread");
    //do_comms(ZMQPORT_BACKCHANNEL_PR, "Packet_Recv_Thread");

    dloop_trigger();


    return 0;
}













