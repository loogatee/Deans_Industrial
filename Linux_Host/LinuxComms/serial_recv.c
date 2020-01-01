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
#include "proj_common.h"
#include "cdc_cmds.h"

/*
*    Note to self:
*
*      For Every Packet Sent:
*
*            Reply packet MUST BE received within 3 seconds OR:
*
*                   Resend the original packet
*
*        
*      Reject duplicate or expired packets
*
*
*/

/*
*    Serial Packet on the wire looks like this:
*
*        0xCC       Start of Packet                                               CC,57
*        0x57       Start of Packet                                               tag
*        Cmd        Cmd Byte                                                      len
*        Len        Len of Data only.       Limit to 254 bytes of data            cmd
*        tag        a sequnce number                                              data[0]
*        csum       checksum
*        data[0]
*        ..
*        ..
*        data[len-1]
*
*
*    Reason limiting to 254:
*
*        Has to do with storing the Packet locally, and limiting
*        the storage on that space to 256 bytes.
*
*            1 byte for Cmd
*            1 byte for Len
*          254 bytes of data
*         -----
*          256
*
*        Then that 256 byte packet will be shipped on to the "PACKET HANDLER GUY"
*
*/


typedef struct
{
    u8               dbuf[BUFLEN];
    time_t           Time0;
    u8               Tag_Seqn;
    Bool             PackFirst,TagFirst;
    int              ttyfd;
    uint             TotBytes,TotZSendErrs,TotZSendGood;
    void            *context1,*context2,*context3;
    void            *PaktChannel,*BackChannel,*CdcResponse;
    zmq_pollitem_t   PollBackChannel[1];
} Globes;

static Globes    Globals;



/* ===================================================================== */


static int dochar(u8 *retchar, time_t T0)
{
    int   n;
    u8    cbuf[1];
    uint  counts = 0;

    do {
        if( (n = read(Globals.ttyfd,cbuf,1)) <= 0 )
        {
            if( ++counts > 2 )
            {
                printf("CHAR_TIMEOUT!\n");
                return RETV_CHAR_TIMEOUT;
            }
        }
    } while (n <= 0);

    *retchar = cbuf[0];
    return RETV_CHAR_OK;
}


static void check_BackChannel(void)
{
    char    lbuf[20];       // match size in zmq_recv statement
    Globes  *G = &Globals;

    if( zmq_poll( G->PollBackChannel, 1, 0 ) > 0 )
    {
        lbuf[0] = lbuf[1] = 0;

        if( zmq_recv(G->BackChannel,lbuf,20,0) == -1 ) { return; }

        printf("%02x, %02x\n",lbuf[0],lbuf[1]);

        if( lbuf[0] == 0x6d && lbuf[1] == 0x01 )
        {
            sprintf(lbuf,"ok,%d,%d,%d", G->TotBytes,G->TotZSendErrs,G->TotZSendGood);
            zmq_send( G->BackChannel, lbuf, strlen(lbuf), 0 );
        }
        else
        {
            zmq_send( G->BackChannel, "err", 3, 0 );
        }
    }
}


//
//  returns true if checksums match
//          false if checksums do not match
//
static Bool Get_Input_Packet(u8 *rtag)
{
    u8      cmd,len,Tch,Lcsum,Pcs;
    u8      cbuf[1];
    u8     *dptr;
    int     i,k;
    time_t  Time0;

//int dbgi;

    dptr = Globals.dbuf;

    while(1)
    {
        i       = 0;
        cbuf[0] = 0;

        do {
            cbuf[0] = 0;

            k = read(Globals.ttyfd,cbuf,1);
            if( k <= 0 ) { check_BackChannel(); }
        } while (cbuf[0] != 0xCC);

        ++Globals.TotBytes;
        Time0 = time(0);
        if( Globals.PackFirst == true )
        {
            Globals.PackFirst = false;
            Globals.Time0 = Time0;
        }

//dbgi = 0;

        if( dochar(&Tch, Time0) == RETV_CHAR_TIMEOUT || Tch != 0x57 ) { continue; }
        ++Globals.TotBytes;

        if( dochar(rtag, Time0) == RETV_CHAR_TIMEOUT ) { continue; }
        ++Globals.TotBytes;

//printf("rtag = %x\n",*rtag);

        if( dochar(&len, Time0) == RETV_CHAR_TIMEOUT ) { continue; }
        dptr[i++] = len;
        ++Globals.TotBytes;

//printf("len = %x\n",len);

        if( dochar(&cmd, Time0) == RETV_CHAR_TIMEOUT ) { continue; }
        dptr[i++] = cmd;
        ++Globals.TotBytes;

//printf("cmd = %x\n",cmd);


        for( Lcsum=cmd+len, k=0; k < len+1; ++k )
        {
            if( dochar(&Tch, Time0) == RETV_CHAR_TIMEOUT ) { continue; }
            dptr[i++]  = Tch;
            Lcsum     += Tch;
            ++Globals.TotBytes;

    //        if( ++dbgi < 3 ) { printf("Tch = %02x\n", Tch ); }
        }

        Lcsum -= Tch;     // back out the checksum

        check_BackChannel();

        Pcs = dptr[2 + len];    // start of data is at index 2

        //printf("CSUMs: %x,%x\n",Lcsum,Pcs);

        if( Lcsum == Pcs )
            return true;
        else
            return false;
    }
}


static Bool Verify_Seqn_Tag(u8 tag)
{
    Bool  retv = true;

    if( (u8)(Globals.Tag_Seqn+1) != tag )
    {
        if( Globals.TagFirst == true )                  // Exception: 1st tag seen since code started executing
            Globals.TagFirst = false;                   //            Will return true, saying that Tag is OK
        else
            retv = false;
    }

    return retv;
}

//
//    Examines packet to determine where to send
//
static void ProcessPacket(void)
{
    int  rc;
    uint len = Globals.dbuf[0] + 2;

    u8  *dptr = Globals.dbuf;
    printf( "ProcessPacket.   %d   Rx:  cmd=0x%02x, len=%d\n", Globals.TotZSendGood, dptr[1], dptr[0] );
    fflush(stdout);

    switch( Globals.dbuf[1] )
    {
        case  CDCAPI_ADGETALLAVGS:
        case  CDCAPI_ADGETVALS:
        case  CDCAPI_DLOOP_FHOST: rc = zmq_send( Globals.CdcResponse, Globals.dbuf, len, 0 ); break;

        default:                  rc = zmq_send( Globals.PaktChannel, Globals.dbuf, len, 0 ); break;
    }

    if( rc < 0 )
        ++Globals.TotZSendErrs;
    else
        ++Globals.TotZSendGood;
}



static void Init_serial_recv_ZMQs( void )
{
    char    tmpbuf[50];
    int     rc;
    Globes *G = &Globals;

    G->context1    = zmq_ctx_new();
    G->PaktChannel = zmq_socket ( G->context1,    ZMQ_PUSH              );
    rc             = zmq_connect( G->PaktChannel, ZMQPORT_SERIAL_TOHOST );    assert(rc == 0);

    G->context2    = zmq_ctx_new();
    G->BackChannel = zmq_socket ( G->context2,    ZMQ_REP               );
    rc             = zmq_bind   ( G->BackChannel, ZMQPORT_BACKCHANNEL_1 );    assert(rc == 0);

    G->context3    = zmq_ctx_new();
    G->CdcResponse = zmq_socket ( G->context3,    ZMQ_PUSH              );
    rc             = zmq_connect( G->CdcResponse, ZMQPORT_CDCRESPONSE   );    assert(rc == 0);

    strcpy(tmpbuf, ZMQPORT_BACKCHANNEL_1);
    chmod(&tmpbuf[6], 0777);
}



//void Serial_Recv( int  Sfd )
void *Serial_Recv( void *Iptr )
{
    int     Sfd;
    u8      tag;
    Globes *G = &Globals;

    Sfd = *((int *)Iptr);

    G->ttyfd        = Sfd;
    G->TagFirst     = true;
    G->PackFirst    = true;
    G->TotBytes     = 0;
    G->TotZSendErrs = 0;
    G->TotZSendGood = 0;

    Init_serial_recv_ZMQs();

    G->PollBackChannel[0].socket  = G->BackChannel;
    G->PollBackChannel[0].fd      = 0;
    G->PollBackChannel[0].events  = ZMQ_POLLIN;
    G->PollBackChannel[0].revents = 0;

    while(1)
    {
        if( Get_Input_Packet(&tag) == true )
        {
            if( Verify_Seqn_Tag(tag) == false ) { /* printf("    ----seqn_tag = BAD\n") */    ; }
            ProcessPacket();
            G->Tag_Seqn = tag;
        }
        else
        {
            printf("    ----calc_csum = BAD\n");
        }
    }
}


