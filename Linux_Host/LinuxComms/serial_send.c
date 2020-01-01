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
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include "proj_common.h"


typedef struct
{
    void  *BackChannel;
    uint   TotPacks;
    uint   TotBytes;
    uint   RecvErrs;
} Globes;

static Globes    Globals;



// sprintf(lbuf,"ok,%d,%d,%d", G->TotBytes,G->TotZSendErrs,G->TotZSendGood);

static void do_backchannel_response( void )
{
    char    lbuf[40];
    Globes *G = &Globals;

    lbuf[0] = lbuf[1] = 0;

    zmq_recv( G->BackChannel, lbuf, 40, 0 );                   // polled already, so data avail now

        printf("%02x, %02x\n",lbuf[0],lbuf[1]);

    if( lbuf[0] == 0x6d  && lbuf[1] == 0x01 )                       // 6d and 01 are the magic sauce
    {
        sprintf(lbuf,"ok,%d,%d,%d,johnr",G->TotBytes,G->RecvErrs,G->TotPacks);
        zmq_send( G->BackChannel, lbuf, strlen(lbuf), 0 );
    }
    else
    {
        zmq_send( G->BackChannel, "err", 3, 0 );
    }
}


//
//   Sfd is the Serial File Descriptor thats already been created.
//   This process needs to use it.
//
void *Serial_Send( void *Iptr )
{
    u8               dbuf[BUFLEN];
    void            *context1,*PktReceiver,*context2;
    int              rc,Tlen;
    uint             i,csum;
    u8              cbuf[6],cmd,plen,Tag_Seqn;
    Globes          *G;
    zmq_pollitem_t   PollItems[2];
    int              Sfd;

    Sfd  = *((int *)Iptr);

    G           = &Globals;
    G->TotPacks = 0;
    G->RecvErrs = 0;
    Tag_Seqn    = 0;

    context1       = zmq_ctx_new();
    PktReceiver    = zmq_socket ( context1,     ZMQ_PULL              );
    rc             = zmq_bind   ( PktReceiver,  ZMQPORT_HOST_TOSERIAL );   assert(rc == 0);

    context2       = zmq_ctx_new();
    G->BackChannel = zmq_socket ( context2,            ZMQ_REP               );
    rc             = zmq_bind   ( Globals.BackChannel, ZMQPORT_BACKCHANNEL_3 );    assert(rc == 0);

    strcpy((void *)dbuf, ZMQPORT_HOST_TOSERIAL);     chmod((void *)&dbuf[6], 0777);
    strcpy((void *)dbuf, ZMQPORT_BACKCHANNEL_3);     chmod((void *)&dbuf[6], 0777);

    PollItems[0].socket  = PktReceiver;
    PollItems[0].fd      = 0;
    PollItems[0].events  = ZMQ_POLLIN;
    PollItems[0].revents = 0;

    PollItems[1].socket  = G->BackChannel;
    PollItems[1].fd      = 0;
    PollItems[1].events  = ZMQ_POLLIN;
    PollItems[1].revents = 0;


    while(1)
    {
        zmq_poll( PollItems, 2, -1 );

        if(   PollItems[1].revents & ZMQ_POLLIN  ) { do_backchannel_response(); }
        if( !(PollItems[0].revents & ZMQ_POLLIN) ) { continue; }

        if( ((Tlen = zmq_recv(PktReceiver,dbuf,BUFLEN,0)) < 0) || (Tlen < 3) )
        {
            ++G->RecvErrs;
            continue;         // drop the packet
        }
        ++G->TotPacks;

        cmd  = dbuf[1];
        plen = dbuf[0];

        for( csum=0,i=0; i < (plen+2); ++i)            // +2 will include cmd and plen bytes
            csum = csum + dbuf[i];
        csum = csum & 0xff;                            // checksum generation

printf( "%d   Tx:  cmd=0x%02x, len=%d, csum=0x%02x\n", Tlen, cmd, plen, csum );

        cbuf[0] = 0xAB;              //  Fixed0
        cbuf[1] = 0x5F;              //  Fixed1
        cbuf[2] = Tag_Seqn++;        //  seqn


        dbuf[plen+2] = csum;         //  csum is the byte after the data
        dbuf[plen+3] = 0xEE;         //  marker for now


        write(Sfd,cbuf,3);
        write(Sfd,dbuf,plen+3);

//printf( "len    %02x\n",dbuf[0]);
//printf( "cmd    %02x\n",dbuf[1]);
//printf( "D0     %02x\n",dbuf[2]);
//printf( "D1     %02x\n",dbuf[3]);
//printf( "164  %02x\n",dbuf[164]);
//printf( "165  %02x\n",dbuf[165]);
//printf( "166  %02x\n",dbuf[166]);

        G->TotBytes += (5 + plen + 1);
    }
}













