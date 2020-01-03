#ifndef _PROJ_COMMON_H_
#define _PROJ_COMMON_H_


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


#define true   1
#define True   1
#define false  0
#define False  0

#define TTYPORT                   "/dev/ttymxc1"
#define ZMQPORT_SERIAL_TOHOST     "ipc:///tmp/zmqfeeds/0"
#define ZMQPORT_HOST_TOSERIAL     "ipc:///tmp/zmqfeeds/1"
#define ZMQPORT_BACKCHANNEL_1     "ipc:///tmp/zmqfeeds/backchannel1"
#define ZMQPORT_BACKCHANNEL_2     "ipc:///tmp/zmqfeeds/backchannel2"
#define ZMQPORT_BACKCHANNEL_3     "ipc:///tmp/zmqfeeds/backchannel3"
#define ZMQPORT_BACKCHANNEL_main  "ipc:///tmp/zmqfeeds/backchannelmain"
#define ZMQPORT_CMDCHANNEL        "ipc:///tmp/zmqfeeds/CmdChannel"
#define ZMQPORT_CDCRESPONSE       "ipc:///tmp/zmqfeeds/CdcResponse"
#define ZMQPORT_BACKCHANNEL_PR    "ipc:///tmp/zmqfeeds/backchannelpaktrecv"


//
// Also change the name in lutils.lua
// 
//     Better yet, pass in as an argument!
//
//#define CONFIGDATA_FILENAME       "/usr/share/monkey/cdcX/ConfigData.lua"
#define CONFIGDATA_FILENAME       "/usr/local/bin/ConfigData.lua"

#define LUTILS_FILENAME           "/usr/local/bin/lutils.lua"


#define RETV_CHAR_TIMEOUT    -1
#define RETV_CHAR_OK          0

#define BUFLEN    280


typedef unsigned char    u8;
typedef unsigned long    u32;
typedef unsigned short   u16;
typedef unsigned int     uint;
typedef unsigned int     Bool;

typedef struct timeval   timeval_t;
typedef struct timezone  timezone_t;



typedef struct
{
    u8    cmdName[20];
    int   parm1;
    int   parm2;
} PARMS_t;




void *Serial_Recv  ( void *Sfd );
void *Serial_Send  ( void *Sfd );
void *PacketHandler( void *xyz );
void *PacketCentral( void *xyz );



#endif
