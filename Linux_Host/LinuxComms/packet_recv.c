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
#include <sys/stat.h>
#include <fcntl.h>
#include "proj_common.h"
#include "cdc_cmds.h"
#include "utils.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


typedef struct
{
    u8          dbuf[BUFLEN];
    void       *context1,*context2,*context3;
    void       *RcvFromSerial;
    void       *TxToSerial;
    void       *BackChan3;
    lua_State  *LS1;
    uint        TotBytes,TotPacks;
    Bool        ControlsUP;
} GLOBALS_PAKTRECV_t;

static GLOBALS_PAKTRECV_t   Globals;






static void getLSstackval(char *rptr)
{
    lua_next(Globals.LS1, -2);
    const char *rp = lua_tostring(Globals.LS1,-1);
    lua_pop(Globals.LS1, 1);
    strcpy(rptr,rp);
}

static int getstackval(void)
{
    lua_next(Globals.LS1, -2);
    int rv = lua_tointeger(Globals.LS1,-1);
    lua_pop(Globals.LS1, 1);
    return rv;
}




//
//    Also working as a Lua server.
//    packet_recv is the only thread that has a lua interpreter 
//
static void do_backchannel_response( void )
{
    PARMS_t  *XX;
    uint      i;
    char      S1buf[100],S2buf[200];
    char      LS[16][10];

    GLOBALS_PAKTRECV_t *G = &Globals;

    zmq_recv( G->BackChan3, S1buf, 40, 0 );                         // polled already, so data avail now
    S1buf[40] = 0;                                                  // Guarantees string termination!!

    if( !strncmp(S1buf,"healthcheckdata",15) )                      // local stats.  also notes alive.
    {
        sprintf(S1buf,"ok,%d,%d",G->TotBytes,G->TotPacks);
        zmq_send( G->BackChan3, S1buf, strlen(S1buf), 0 );
    }
    else if( !strncmp(S1buf,"ADGetNames",10) )                      // Names of all 16 A/D channels
    {
        lua_getglobal(G->LS1,"lua_Get_AD_Names");                   // This is the name of the Lua function
        lua_pcall(G->LS1,0,1,0);                                    // executes the Lua function, 1 return arg on stack
        lua_pushnil(G->LS1);                                        // precursor needed for reading stack values

        for(i=0; i < 16; ++i)                                       // iterate through them all
        {
            getLSstackval(LS[i]);                                   // get the name: storage pointer passed in as parm
        }

        sprintf(S1buf,  "[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",", LS[0],LS[1],LS[2],LS[3], LS[4], LS[5], LS[6], LS[7]);
        sprintf(S2buf, "%s\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]", S1buf,LS[8],LS[9],LS[10],LS[11],LS[12],LS[13],LS[14],LS[15]);
        zmq_send( G->BackChan3, S2buf, strlen(S2buf), 0 );
    }
    else if( !strncmp(S1buf,"InParms",7) )                          // 0..7      Field_0
    {
        memcpy((void *)&XX,&S1buf[8],4);                            // 8..11     Field_1

        lua_getglobal(G->LS1,"lua_Get_parms");                      // This is the name of the Lua function
        sprintf(S2buf,"Z=%s",&S1buf[12]);                           // 12..N     Field_2       Note the 'Z=' addition (check lutils.lua)

        lua_pushstring(G->LS1, S2buf);
        lua_pcall(G->LS1,1,1,0);                                    // executes the Lua function, 1 return arg on stack

        lua_pushnil(G->LS1);                                        // precursor needed for reading stack values
        getLSstackval((char *)XX->cmdName);                         // 1st value is the Command name in string format
        XX->parm1 = getstackval();                                  // 2nd value is an integer
        XX->parm2 = getstackval();                                  // 3rd value is an integer
                                                                    // *** Note for now, that parameters are fixed
        zmq_send( G->BackChan3, S2buf, 1, 0 );
    }
    else
    {
        zmq_send( G->BackChan3, "Err", 3, 0 );
    }
}




static void PH_dloop( int len )
{
    zmq_send( Globals.TxToSerial, Globals.dbuf, len, 0);
}


static void send_IMUP( void )
{
    u8 localdbuf[10];

    //                   len cmd d0 d1 d2 d3
    //  data len is 4
    //  but need to write 6 bytes
    //  \147=0x67=CDCAPI_IMUP
    memcpy( localdbuf, "\004\147IMUP",6);

    zmq_send( Globals.TxToSerial, localdbuf, 6, 0);
}



//
//
//


//    CAll a Lua function to do all the 'heavy' lifting.   We need three values
//    for each A/D sensor:
//          - Name
//          - Enbl
//          - Type
//    And we also need 2 coefficients: one for SYw and one for CPw.
//    The Lua function pushes all the needed values onto the stack, in the 
//    form of a Lua Table.   This code then just pops the values from the stack
//    and puts them right into the data packet.
//
//     **** Look for swapping issues on the memcpy for host/devices pairs. ****
//
//    Weirdism with Lua:  even though there are many values on the stack, they
//                        are all contained within a single table, that gets
//                        pushed onto the stack. This means there is only
//                        1 return value from the Lua call.
//
//
//        name,enable,type
//        (u8,  u8,    u8) * 16  = 48 bytes
//        (u32) * 2              =  8 bytes
//                             -------------
//                                 56
//
static void PH_AnConfig( void )
{
    u8       dbuf[BUFLEN];
    int      I1,I2,i;
    u8      *dptr = dbuf;

    GLOBALS_PAKTRECV_t *G = &Globals;

    printf("PH_AnConfig\n\r");  fflush(stdout);

#define SIZE_ANCONFIG_PACKET   56

    *dptr++ = SIZE_ANCONFIG_PACKET;                             // Byte[0] = Len
    *dptr++ = CDCAPI_ANCONFIG;                                  // Byte[1] = Cmd

    lua_getglobal(G->LS1,"lua_Get_AD_Config");                  // This is the name of the Lua function
    lua_pcall(G->LS1,0,1,0);                                    // executes the Lua function, 1 return arg on stack
    lua_pushnil(G->LS1);                                        // precursor needed for reading stack values

    for(i=0; i < 16; ++i)
    {
        *dptr++ = (u8)getstackval();                            // Name
        *dptr++ = (u8)getstackval();                            // Enbl
        *dptr++ = (u8)getstackval();                            // Type
    }

    I1 = getstackval();                                         // Coef for SYw
    I2 = getstackval();                                         // Coef for CPw

    memcpy((void *)dptr,    (const void *)&I1, 4);              // Identical Byte ordering allows this
    memcpy((void *)(dptr+4),(const void *)&I2, 4);              //   this being:  Intel host + Blackfin device
                                                                //   *** might be different for RPI or Beagle ***
/*  dptr = &dbuf[2];
    printf("static u8 Canned_AtoD_config[56] = {\n\r");
    for( i=0; i<16; ++i)
    {
        printf("   0x%02x, 0x%02x, 0x%02x,\n\r",dptr[0],dptr[1],dptr[2]);
        dptr += 3;
    }
    printf("   0x%02x, 0x%02x, 0x%02x, 0x%02x,\n\r",dptr[0],dptr[1],dptr[2],dptr[3]);
    printf("   0x%02x, 0x%02x, 0x%02x, 0x%02x\n\r",dptr[4],dptr[5],dptr[6],dptr[7]);
    printf("};\n\r");
    fflush(stdout);           */

    zmq_send( G->TxToSerial, dbuf, SIZE_ANCONFIG_PACKET+2, 0);  // Add in Len byte + Cmd byte
}


void Set_Comms_to_CTRL_pin( void )
{
    int  fd;

    fd = open("/sys/class/gpio/export",           O_WRONLY);  write(fd,"137\n",4);  close(fd); 
    fd = open("/sys/class/gpio/gpio137/direction",O_WRONLY);  write(fd,"out\n",4);  close(fd);
    fd = open("/sys/class/gpio/gpio137/value",    O_WRONLY);  write(fd,"0\n",  2);  close(fd);

    usleep( 2000000 );      // 2 seconds
    fd = open("/sys/class/gpio/gpio137/value",    O_WRONLY);  write(fd,"1\n",  2);  close(fd);
}





//=============================================================================
//=============================================================================

static void Init_PacketHandler_ZMQs( void )
{
    char    dbuf[40];
    int     rc;
//  Globes             *G = &Globals;
    GLOBALS_PAKTRECV_t *G = &Globals;

    G->context1      = zmq_ctx_new();
    G->RcvFromSerial = zmq_socket ( G->context1,      ZMQ_PULL              );
    rc               = zmq_bind   ( G->RcvFromSerial, ZMQPORT_SERIAL_TOHOST );   assert(rc == 0);

    G->context2      = zmq_ctx_new();
    G->TxToSerial    = zmq_socket ( G->context2,      ZMQ_PUSH              );
    rc               = zmq_connect( G->TxToSerial,    ZMQPORT_HOST_TOSERIAL );   assert(rc == 0);

    G->context3      = zmq_ctx_new();
    G->BackChan3     = zmq_socket ( G->context3,      ZMQ_REP                );
    rc               = zmq_bind   ( G->BackChan3,     ZMQPORT_BACKCHANNEL_PR );    assert(rc == 0);

    strcpy((void *)dbuf, ZMQPORT_SERIAL_TOHOST);     chmod((void *)&dbuf[6], 0777);
    strcpy((void *)dbuf, ZMQPORT_BACKCHANNEL_PR);    chmod((void *)&dbuf[6], 0777);
}

#define ZMQPORT_BACKCHANNEL_PR    "ipc:///tmp/zmqfeeds/backchannelpaktrecv"


void *PacketHandler( void *dummy )
{
    u8              cmd;
    int             len;
    zmq_pollitem_t  PollItems[2];

    GLOBALS_PAKTRECV_t *G = &Globals;

    G->TotBytes   = 0;
    G->TotPacks   = 0;
    G->ControlsUP = False;

    Init_PacketHandler_ZMQs();                          //   Socket initialization here

    G->LS1 = luaL_newstate();                           //   This makes Lua available (like for reading configs!)
    luaL_openlibs(G->LS1);                              //   Book says to also do this

    if( luaL_dofile(G->LS1, LUTILS_FILENAME) != 0 )        //   Can now extend the app with these lua functions
    {
        printf("ERROR opening %s\n", LUTILS_FILENAME);
        exit(1);
    }
    else
        printf("GOOD opening %s\n", LUTILS_FILENAME);
    fflush(stdout);

    PollItems[0].socket  = G->RcvFromSerial;
    PollItems[0].fd      = 0;
    PollItems[0].events  = ZMQ_POLLIN;
    PollItems[0].revents = 0;

    PollItems[1].socket  = G->BackChan3;
    PollItems[1].fd      = 0;
    PollItems[1].events  = ZMQ_POLLIN;
    PollItems[1].revents = 0;

    Set_Comms_to_CTRL_pin();
    send_IMUP();

    while( 1 )
    {
        zmq_poll( PollItems, 2, -1 );

        if(   PollItems[1].revents & ZMQ_POLLIN  ) { do_backchannel_response(); }
        if( !(PollItems[0].revents & ZMQ_POLLIN) ) { continue; }

        len = zmq_recv( G->RcvFromSerial, G->dbuf, BUFLEN, 0 );
        cmd = G->dbuf[0];

        ++G->TotPacks;
        G->TotBytes += len;

        switch( cmd )
        {
          case CDCAPI_DLOOP:
          case CDCAPI_DLOOP2:     PH_dloop(len);   break;

          case CDCAPI_ANCONFIG:

                      PH_AnConfig();
                      break;


          case CDCAPI_IMUP:

                      //G->dbuf[2+len] = 0;
                      if( !memcmp((const void *)&G->dbuf[2],(const void *)"imup2",5) )
                      {
                          printf("Got imup2\n\r");
                          G->ControlsUP = True;
                      }
                      else
                      {
                          printf("ERROR: No imup2\n\r");
                      }
                      fflush(stdout);
                      break;

/*        case CDCAPI_JRTEST:     PH_JRtest();     break;
          case CDCAPI_BLASTAWAY:

                      memcpy((void *)dbgbuf, (void *)&G->dbuf[2], len-2);
                      dbgbuf[len-2]=0;
                      printf("%s",dbgbuf); fflush(stdout);
                      break;   */

          default:    printf( "--Default Packet: zmqlen=%d, cmd=0x%02x, len=%d\n", len, cmd, len );
                      fflush(stdout);
                      break;
        }

    }
}



