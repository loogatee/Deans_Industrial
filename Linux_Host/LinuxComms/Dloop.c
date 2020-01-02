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

// if DATASIZE == 0, then size will be random between 5 and 246
#define DATASIZE     0
#define NUMLOOPS     2


#define DLOOP_GOOD     1
#define DLOOP_TIMEOUT  2
#define DLOOP_BADCMP   3



typedef struct
{
    u8     dloop_original[256];
    void  *context1,*context2,*context3,*context4;
    void  *SerialChannel;
    void  *CdcResponse;
    uint   RespTimeouts;
} Globes;



static Globes  Globals;



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
    } while( val < 5 || val > 244 );

    return val;     
}


static int PC_dloop( void )
{
    u8    dbuf[BUFLEN];
    u8   *dptr,*tptr,tmpb;
    uint  num_loops,i,j,data_size,result;
    int   rc;
    u8    ZeroFlag;

    //printf("PC_dloop, inbuf[2] = %d,   inbuf[3] = %d\n", inbuf[2], inbuf[3] );

    result    = DLOOP_GOOD;
    dptr      = Globals.dloop_original;
    tptr      = &dptr[2];                      // [0] = Len, [1] = Cmd, [2] = data[0]

    num_loops = NUMLOOPS;
    data_size = DATASIZE;
    ZeroFlag  = 0;

    seed_the_random();

    for( j=0; j < num_loops; ++j )
    {
        if( data_size == 0  || ZeroFlag == 1 )
        {
            ZeroFlag = 1;
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

        // ***** Length Byte and Cmd Byte are swapped upon return (due to ZMQ pub/sub)

        tmpb    = dbuf[0];
        dbuf[0] = dbuf[1];
        dbuf[1] = tmpb;

        //memdump(dptr);
        //memdump(dbuf);

        if(memcmp(dptr,dbuf,rc))                                      // dptr is the original data, dbuf just got read in
        {
            result = DLOOP_BADCMP;
            break;
        }
    }

    if( result == DLOOP_GOOD)
        printf("GOOD,%d,%d\n",num_loops,data_size);
    else if( result == DLOOP_TIMEOUT)
        printf("ERROR,Timeout,Numloops = %d\n",num_loops);
    else
        printf("ERROR,dloop data mismatch,numloops = %d\n",num_loops);

    return 0;
}

//================================================================================
//================================================================================


static void Init_Dloop_ZMQs( void )
{
    char    tbuf[50];
    int     rc;
    Globes *G = &Globals;


    // Some External Entity Performs a (Send-Request) on the CmdChannel (Give me the Tank Water Temp)


    // (Send) TO the SerialChannel, results in (Recv) FROM the Response Channel
    G->context2      = zmq_ctx_new();
    G->SerialChannel = zmq_socket ( G->context2,      ZMQ_PUSH              );
    rc               = zmq_connect( G->SerialChannel, ZMQPORT_HOST_TOSERIAL );  assert(rc == 0);

    // (Read) FROM the Response Channel, results in (Send-Reply) TO the Cmd Channel 
    G->context3      = zmq_ctx_new();
    G->CdcResponse   = zmq_socket ( G->context3,      ZMQ_SUB               );
    rc               = zmq_connect( G->CdcResponse,   ZMQPORT_CDCRESPONSE   );  assert(rc == 0);

    tbuf[0] = 25;
    //tbuf[1] = 0;
    rc = zmq_setsockopt (G->CdcResponse, ZMQ_SUBSCRIBE, tbuf, 1);

    strcpy(tbuf, ZMQPORT_CDCRESPONSE);     chmod(&tbuf[6], 0777);
}




//
//    dbuf[0] dbuf[1]:      16 bits representing Command
//    dbuf[2]..dbuf[XX]:    optional data 
//
int main( void )
{
    Globes          *G = &Globals;

    G->RespTimeouts = 0;

    Init_Dloop_ZMQs();
    
    PC_dloop();
}


