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
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "proj_common.h"
#include "cdc_cmds.h"


#define DLOOP_GOOD     1
#define DLOOP_TIMEOUT  2
#define DLOOP_BADCMP   3





typedef struct
{
    u8         dloop_original[256];
    void      *context1,*context2,*context3,*context4,*context5;
    void      *CmdChannel;
    void      *SerialChannel;
    void      *BackChannel;
    void      *BackChannel_PR;   // backchannel for packet_recv (also the Lua server)
    void      *CdcResponse;
    uint       RespTimeouts;
    PARMS_t    localParms;
} Globes;



static Globes  Globals;





static void do_backchannel_response( void )
{
    char  lbuf[80];

    lbuf[0] = lbuf[1] = 0;

    zmq_recv( Globals.BackChannel, lbuf, 80, 0 );                   // polled already, so data avail now

    if( lbuf[0] == 0x6d  && lbuf[1] == 0x01 )                       // 6d and 01 are the magic sauce
    {
        sprintf(lbuf,"ok");                                         // need to think of something better than just 'ok'
        zmq_send( Globals.BackChannel, lbuf, strlen(lbuf), 0 );
    }
    else
    {
        zmq_send( Globals.BackChannel, "err", 3, 0 );
    }
}


/*
static void memdump( u8 *D )
{
     uint i;

     for( i=0; i < 32; ++i )
     {
         if( !(i % 16) ) { printf("\n"); }
         printf("%02x ",D[i]);
     }
     printf("\n");

} */


//
//   Wait time on zmq_poll is in milliseconds.
//   Note that there is no poll on the backchannel.
//   It will have to wait at most 3 seconds in here, so backchannel can wait.
//
//
static int  Wait_On_Response(u8 *pbuf, int llen)
{
    zmq_pollitem_t  Pitems[1];
    int rc;
    int retv;

    Pitems[0].socket  = Globals.CdcResponse;
    Pitems[0].fd      = 0;
    Pitems[0].events  = ZMQ_POLLIN;
    Pitems[0].revents = 0;

    if( (retv=zmq_poll(Pitems,1,3000)) <= 0 )
    {
        printf("zmq_poll timeout: %d\n",retv); fflush(stdout);
        ++Globals.RespTimeouts;
        return -1;
    }

    rc = zmq_recv( Globals.CdcResponse, pbuf, llen, 0 );
    //printf("WOR, pbuf[3] = %02x\n",pbuf[3]);
    return rc;
}


static void seed_the_random( void )
{
    timeval_t  tvt;
    timezone_t tzt;

    gettimeofday(&tvt,&tzt);
    srand((uint)(tvt.tv_usec+13));        // Feeling Lucky??
}

u8 Get_datasize( void )
{
    u8 val;

    do
    {
        val = (u8)(rand() & 0xff);
    } while( val < 5 || val > 246 );

    return val;     
}


static int PC_dloop( int  data_size, int num_loops )
{
    u8    dbuf[BUFLEN],*dptr,*tptr,tmpb;
    uint  i,j,result;
    int   rc, was_zero;

    if( num_loops == -9999 || num_loops == 0   ) { num_loops = 1;   }
    if( data_size == -9999 || data_size >= 246 ) { data_size = 246; }


    result    = DLOOP_GOOD;
    dptr      = Globals.dloop_original;
    tptr      = &dptr[2];
    was_zero  = 0;


    seed_the_random();

    for( j=0; j < num_loops; ++j )
    {
        if( data_size == 0 || was_zero == 1 )
        {
            was_zero = 1;
            data_size = Get_datasize();
        }
        dptr[0] = data_size;                      // Len Byte
        dptr[1] = CDCAPI_DLOOP_FHOST;             // Cmd Byte

        for( i=0; i < data_size; ++i)             //   Data Bytes:  D[0] .. D[data_size-1]
            tptr[i] = (u8)(rand() & 0xff);

        zmq_send( Globals.SerialChannel, dptr, data_size+2, 0 );         // +2 adds in len byte and cmd byte

        memset(dbuf,0,8);

        if( (rc=Wait_On_Response(dbuf,BUFLEN)) < 0 )                      // rc bytes in dbuf on success
        {
            printf(" DLOOP_TIMEOUT: data_size = %d\n", data_size);  fflush(stdout);
            result = DLOOP_TIMEOUT;
            break;
        }

        //memdump(dptr);
        //memdump(dbuf);

        tmpb = dbuf[0];
        dbuf[0] = dbuf[1];
        dbuf[1] = tmpb;

        if(memcmp(dptr,dbuf,rc))                                      // dptr is the original data, dbuf just got read in
        {
            result = DLOOP_BADCMP;
            break;
        }
    }

    if( result == DLOOP_GOOD)
        sprintf((char *)dbuf,"GOOD,%d,%d",num_loops,data_size);
    else if( result == DLOOP_TIMEOUT)
        sprintf((char *)dbuf,"ERROR,Timeout,Numloops = %d",num_loops);
    else
        sprintf((char *)dbuf,"ERROR,dloop data mismatch,numloops = %d",num_loops);

    zmq_send( Globals.CmdChannel, dbuf, strlen((char *)dbuf), 0 );
    return 0;
}

/*
//
//
//    b2  LED1
//    b3  LED2
//    b4  LED3
//    b5  LED4
//
static void PC_setled( u8 *abuf )
{
    u8    dbuf[BUFLEN];

    dbuf[0] = CDCAPI_SETLED;             // Cmd Byte
    dbuf[1] = 2;                         // Len Byte
    dbuf[2] = abuf[2];
    dbuf[3] = abuf[3];

    zmq_send( Globals.SerialChannel, dbuf, 4, 0 );         // 4:  cmd byte + len byte + 2 data bytes
    zmq_send( Globals.CmdChannel,    "ok", 2, 0 );         // reply back to CmdChannel with ok msg
} */

/*
static void PC_resetdev( void )
{
    u8    dbuf[BUFLEN];

    dbuf[0] = CDCAPI_RESETDEVICE;        // Cmd Byte
    dbuf[1] = 1;                         // Len Byte
    dbuf[2] = 0;

    zmq_send( Globals.SerialChannel, dbuf, 3, 0 );              // 
    zmq_send( Globals.CmdChannel, "ok", 2 , 0 );                // reply back to CmdChannel with ok msg
} */


/*
//
//   7 32-bit values are returned.   Len = 28bytes
//
static void PC_adgetvals( u8 *abuf )
{

    u8    dbuf[5],rbuf[LEN_RBUF];
    char  Sbuf[100];
    uint  xchan,xmin,xmax,xsum,xsamps,xminv,xmaxv;

    memset(rbuf,0,32);

    dbuf[0] = CDCAPI_ADGETVALS;          // Cmd Byte
    dbuf[1] = 1;                         // Len Byte
    dbuf[2] = abuf[2];                   // Channel number:  1..16

    zmq_send( Globals.SerialChannel, dbuf, 3, 0 );                 // 3:  cmd byte + len byte + 1 data bytes

    if( Wait_On_Response(rbuf,LEN_RBUF) < 0 )
    {
        sprintf(Sbuf, "[\"Timeout\",\"%d\"]", (uint)abuf[2]);
    }
    else
    {
        memcpy( (void *)&xchan,  (void *)&rbuf[2],  4);
        memcpy( (void *)&xmin,   (void *)&rbuf[6],  4);
        memcpy( (void *)&xmax,   (void *)&rbuf[10], 4);
        memcpy( (void *)&xsum,   (void *)&rbuf[14], 4);
        memcpy( (void *)&xsamps, (void *)&rbuf[18], 4);
        memcpy( (void *)&xminv,  (void *)&rbuf[22], 4);
        memcpy( (void *)&xmaxv,  (void *)&rbuf[26], 4);

        sprintf(Sbuf, "[\"ok\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\"]", xchan,xmin,xmax,xsum,xsamps,xminv,xmaxv);
    }

    zmq_send( Globals.CmdChannel,  Sbuf, strlen(Sbuf), 0 );        // reply back to CmdChannel with ok msg
} */


//
//   16 32-bit values are returned.   Len = 64bytes
//
static void PC_adgetall( void )
{
#define LEN_RBUF     80

    u8     dbuf[5],rbuf[LEN_RBUF];
    char   S1buf[100],S2buf[200];
    uint   i,j,tmpu;
    float  XX[16];

    memset(rbuf,0,LEN_RBUF);

    dbuf[0] = 1;                     // Len Byte
    dbuf[1] = CDCAPI_ADGETALL;       // Cmd Byte
    dbuf[2] = 0;                     // a dummy

    zmq_send( Globals.SerialChannel, dbuf, 3, 0 );                 // 3:  cmd byte + len byte + 1 data bytes

    if( Wait_On_Response(rbuf,LEN_RBUF) < 0 )
    {
        sprintf(S2buf, "[\"Timeout\",\"%d\"]", (uint)0);
    }
    else
    {
        for( j=5,i=0; i<16; ++i,j+=4)
        {
            memcpy((void *)&tmpu, (void *)&rbuf[j],  4);
            XX[i] = (tmpu == 0) ? 0.0 : (float)tmpu / 1000.0;
        }

        sprintf(S1buf,  "[\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\",", XX[0],XX[1],XX[2],XX[3], XX[4], XX[5], XX[6], XX[7]);
        sprintf(S2buf, "%s\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\"]", S1buf,XX[8],XX[9],XX[10],XX[11],XX[12],XX[13],XX[14],XX[15]);
    }

    zmq_send( Globals.CmdChannel,  S2buf, strlen(S2buf), 0 );        // reply back to CmdChannel with ok msg
}





//================================================================================
//================================================================================

//
//     struct
//     {
//         char      CmdName[8];      // "InParms\0".  exactly 8 bytes            0..7
//         PARMS_t  *Pptr;            // parameters are written to this addr      8..11
//         char      InputBuf[30];    // zero terminated string                  12..N
//     }
static void Get_InputParms( char *Sptr)
{
    u32      Dval;
    //u32     *dptr;
    char     dbuf[80];
    Globes  *G = &Globals;

    strcpy(dbuf,"InParms");

    Dval = (u32)&G->localParms;
    memcpy( &dbuf[8], (void *)&Dval, 4 );

    strcpy(&dbuf[12],Sptr);

    zmq_send(G->BackChannel_PR, dbuf, strlen(Sptr) + 13, ZMQ_NOBLOCK);      // 12 + (The string \0 terminator)
    zmq_recv(G->BackChannel_PR, dbuf, 10, 0 );                              // 1 byte comes back.  PARMS_t already filled
}




static void Init_PacketCentral_ZMQs( void )
{
    char    tbuf[50];
    int     rc;
    Globes *G = &Globals;


    // Some External Entity Performs a (Send-Request) on the CmdChannel (Give me the Tank Water Temp)

    // (Recv-Request) FROM the Cmd Channel, results in (Send) TO the SerialChannel
    G->context1      = zmq_ctx_new();
    G->CmdChannel    = zmq_socket ( G->context1,      ZMQ_REP               );
    rc               = zmq_bind   ( G->CmdChannel,    ZMQPORT_CMDCHANNEL    );  assert(rc == 0);

    // (Send) TO the SerialChannel, results in (Recv) FROM the Response Channel
    G->context2      = zmq_ctx_new();
    G->SerialChannel = zmq_socket ( G->context2,      ZMQ_PUSH              );
    rc               = zmq_connect( G->SerialChannel, ZMQPORT_HOST_TOSERIAL );  assert(rc == 0);

    // (Read) FROM the Response Channel, results in (Send-Reply) TO the Cmd Channel 
    G->context3      = zmq_ctx_new();
    G->CdcResponse   = zmq_socket ( G->context3,      ZMQ_SUB               );
    rc               = zmq_connect( G->CdcResponse,   ZMQPORT_CDCRESPONSE   );  assert(rc == 0);
    rc = zmq_setsockopt (G->CdcResponse, ZMQ_SUBSCRIBE, 0, 0);

    // (Recv-Request) FROM the backchannel, results in (Send-Reply) TO the Back Channel
    G->context4      = zmq_ctx_new();
    G->BackChannel   = zmq_socket ( G->context4,      ZMQ_REP               );
    rc               = zmq_bind   ( G->BackChannel,   ZMQPORT_BACKCHANNEL_2 );  assert(rc == 0);

    // Request-Recv) FROM the backchannel_PR
    G->context5       = zmq_ctx_new();
    G->BackChannel_PR = zmq_socket ( G->context5,       ZMQ_REQ                );
    rc                = zmq_connect( G->BackChannel_PR, ZMQPORT_BACKCHANNEL_PR );  assert(rc == 0);

    strcpy(tbuf, ZMQPORT_CDCRESPONSE);     chmod(&tbuf[6], 0777);
    strcpy(tbuf, ZMQPORT_CMDCHANNEL);      chmod(&tbuf[6], 0777);
    strcpy(tbuf, ZMQPORT_BACKCHANNEL_2);   chmod(&tbuf[6], 0777);
}




//
//    dbuf[0] dbuf[1]:      16 bits representing Command
//    dbuf[2]..dbuf[XX]:    optional data 
//
void *Canned_Packets( void *dummy )
{
    u8               dbuf[BUFLEN];
    zmq_pollitem_t   PollItems[3];
    Globes          *G = &Globals;
    int              rc;

    G->RespTimeouts = 0;

    Init_PacketCentral_ZMQs();

    PollItems[0].socket  = G->CmdChannel;
    PollItems[0].fd      = 0;
    PollItems[0].events  = ZMQ_POLLIN;
    PollItems[0].revents = 0;

    PollItems[1].socket  = G->BackChannel;
    PollItems[1].fd      = 0;
    PollItems[1].events  = ZMQ_POLLIN;
    PollItems[1].revents = 0;

    PollItems[2].socket  = G->CdcResponse;
    PollItems[2].fd      = 0;
    PollItems[2].events  = ZMQ_POLLIN;
    PollItems[2].revents = 0;


    while(1)
    {
        zmq_poll( PollItems, 3, -1 );

        if(   PollItems[1].revents & ZMQ_POLLIN  ) { do_backchannel_response(); }
        if(   PollItems[2].revents & ZMQ_POLLIN  ) { zmq_recv( G->CdcResponse, dbuf, BUFLEN, 0 ); }      // unexpected: read it, dump it
        if( !(PollItems[0].revents & ZMQ_POLLIN) ) { continue; }

        rc=zmq_recv( G->CmdChannel, dbuf, BUFLEN, 0 );           // data should be immediately available
        dbuf[rc] = 0;                                            // will guarantee string termination

        //
        //   { 'Dloop', 100, 2 }    { 'Dloop', 100 }   { 'Dloop' }
        //
        Get_InputParms( (char *)dbuf );    // G->localParms gets filled with data

        //printf("cmdName: %s\n", G->localParms.cmdName);
        //printf("parm1:   %d\n", G->localParms.parm1);
        //printf("parm2:   %d\n", G->localParms.parm2);

        if( !strcmp((const char *)G->localParms.cmdName,"Dloop") )
        {
            PC_dloop(G->localParms.parm1,G->localParms.parm2);
        }
        else if( !strcmp((const char *)G->localParms.cmdName,"ADGetAll") )
        {
            PC_adgetall();
        }
        else
        {
            printf("PacketCentral, cmd = %s\n", G->localParms.cmdName); fflush(stdout);
            zmq_send( G->CmdChannel, "cmd not found", 13 , 0 );            // reply back with err msg
        }

        //case CDCCMD_SETLED:        PC_setled(dbuf);        break;

        //case CDCCMD_RESETDEV:      PC_resetdev();          break;
        //case CDCCMD_ADGETVALS:     PC_adgetvals(dbuf);     break;
        //case CDCCMD_ADGETALLAVGS:  PC_adgetallavgs();      break;
    }

}


