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

#include "stm32f4xx.h"
#include "proj_common.h"
#include "Uart.h"
#include <stdio.h>
#include <string.h>
#include "Cmds.h"
#include "gpio_CDC3.h"
#include "Utils.h"
#include "i2c.h"
#include "AtoDs.h"
#include "timer.h"
#include "DataBuffer.h"
#include "PacketDrivers.h"
#include "PacketSendThread.h"
#include "Host_Cmds.h"


#define STATE_LOOK_FOR_0           1
#define STATE_LOOK_FOR_1           2
#define STATE_LOOK_FOR_IMUP_PAKT   3
#define STATE_STABLE_UP            4



typedef struct
{
    bool             has_IMUPpakt;
    u8               state_machine;
    u8               skipcount;
    u8               sendBuf1[12];
    u8               sendBuf2[10];
    bool             Sent_ADreq;
} GLOBALS_MonitorHost_t;

static GLOBALS_MonitorHost_t    Globals;



void MonitorHost_Init( void )
{
    GLOBALS_MonitorHost_t   *G = &Globals;

    G->state_machine = STATE_LOOK_FOR_0;
    G->has_IMUPpakt  = False;
    G->skipcount     = 0;
    G->Sent_ADreq    = False;

    strcpy( (char *)&G->sendBuf1[PAKTSODi], (const char *)"imup2" );

    G->sendBuf2[PAKTSODi]   = 4;    // placeholder only
    G->sendBuf2[PAKTSODi+1] = 9;    // placeholder only
}


//
//  The 'COMMS_to_CTRL_Pin' is reading 'sticky' after either a reset or power on.
//    if 0, then reset or power event happens, then comes back up:  it reads as 0
//    if 1, then reset or power event happens, comes back up:       it reads as 1
//
//    Totally weird.
//
//  That's my main problem.
//
void MonitorHost_Process( void )
{
    u8 PinVal;
    GLOBALS_MonitorHost_t   *G = &Globals;

    if( ++G->skipcount < 10 ) { return; }

    G->skipcount = 0;
    PinVal = GPIO_ReadInputDataBit(GPIOB, COMMS_to_CTRL_Pin);

    switch( G->state_machine )
    {
    case STATE_LOOK_FOR_0:           if( PinVal == 1 ) { return; }
                                     G->state_machine = STATE_LOOK_FOR_1;
                                     FALL_THRU;

    case STATE_LOOK_FOR_1:           if( PinVal == 0 ) { return; }
                                     G->state_machine = STATE_LOOK_FOR_IMUP_PAKT;
                                     UD_PrintSTR( "got my transition\n\r" );
                                     FALL_THRU;

    case STATE_LOOK_FOR_IMUP_PAKT:
    case STATE_STABLE_UP:            if( G->has_IMUPpakt == False && PinVal == 1 ) { return; }

                                     if( PinVal == 0 )
                                     {
                                         G->state_machine = STATE_LOOK_FOR_1;
                                         G->has_IMUPpakt  = False;              // in case of royal flush
                                         return;
                                     }
                                     if( G->has_IMUPpakt == True )
                                     {
                                         UD_Print32("state: STABLE_UP   ", (u32)G->sendBuf1);
                                         G->has_IMUPpakt  = False;
                                         G->state_machine = STATE_STABLE_UP;

                                         PacketSend_pakt( HOSTCMD_IMUP, 5, G->sendBuf1, 0 );

                                         if( G->Sent_ADreq == False )
                                         {
                                             G->Sent_ADreq = True;
                                             PacketSend_pakt( HOSTCMD_ADCONFIG, 2, G->sendBuf2, 0 );
                                         }

                                     }
                                     return;
    }
}

bool MonitorHost_IsUp( void )
{
    if( Globals.state_machine == STATE_STABLE_UP )
        return True;
    else
        return False;
}

u32 MonitorHost_GetState( void )
{
    return Globals.state_machine;
}

void MonitorHost_IMUP_pakt( void )
{
    Globals.has_IMUPpakt = True;
}



