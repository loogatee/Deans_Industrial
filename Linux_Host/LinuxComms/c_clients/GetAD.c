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


static void PC_GetAllAD( void )
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

        printf("%s\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\",\"%.2f\"]\n", S1buf,XX[8],XX[9],XX[10],XX[11],XX[12],XX[13],XX[14],XX[15]);
    }
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

    tbuf[0] = 30;
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
    Globes  *G = &Globals;

    G->RespTimeouts = 0;

    Init_Dloop_ZMQs();
    
    PC_GetAllAD();
}


