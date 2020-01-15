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
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
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
#include "monitor_host.h"
#include "Host_Cmds.h"
#include "PacketDrivers.h"
#include "PacketSendThread.h"
#include "Host_Cmds.h"

#define Q_ELEMENTS    3
#define Q_ITEMSIZE    sizeof( uint32_t )

#define LENi  0
#define CMDi  1
#define SODi  2


typedef struct
{
    uint32_t           Ntime;
    TaskHandle_t       RT_Thand;
    QueueHandle_t      RQhand;
    StaticQueue_t      StaticQueue;
    u8                 DloopBuf[256];
    u8                 StaticQBuffer[ Q_ELEMENTS * Q_ITEMSIZE ];    // data buffer reserved for StaticQueue
} GLOBALS_PRT_t;


static GLOBALS_PRT_t   Globals;



static u32 get_value_cmd( u8 parm );

extern AD_SignalDef_t CSVar[];           // for convenience.          Per Signal

STATIC void PacketRecv_Thread(void const * argument)
{
    Bool            ReturnTheBuffer;
    u8             *Dptr,Z;
    u32             tmp;
    GLOBALS_PRT_t  *G = &Globals;

    while(1)
    {
        xQueueReceive( G->RQhand, &Dptr, portMAX_DELAY );               // Hang here till incoming

        ReturnTheBuffer = True;                                         // Default: yes!

        switch( Dptr[CMDi] )
        {
        case HOSTCMD_IMUP:                                              // 1st packet after Host Comm App is Up!

            UD_Print8("IMUP paktlen: ",  Dptr[LENi]);                   // msg to the peeps
            MonitorHost_IMUP_pakt();                                    // Signal that host is up.
            break;

        case HOSTCMD_ADCONFIG:                                          // Set A/D config

            UD_Print8("ADCONFIG paktlen: ",  Dptr[LENi]);               // bark a lil bit
            AtoD_Set_NewConfig( Dptr[LENi], &Dptr[SODi] );              // Configs A/D based on data @ Dptr[2]
            break;

        case HOSTCMD_DLOOP_OH:                                          // Dloop, originating on the host

            memcpy(&G->DloopBuf[PAKTSODi], &Dptr[SODi], Dptr[LENi]);    // copies all the data from input buffer to a local buffer
            PacketSend_pakt(HOSTCMD_DLOOP_OH,Dptr[LENi],G->DloopBuf,0); // Sends the newly populated buffer back to the host
            break;

        case HOSTCMD_SETGPIOS:

            Z = Dptr[SODi+1];
            switch( Dptr[SODi] )
            {
              case pFAN2:     { if( !Z ) GPIO_ResetBits(GPIOA,   FAN2_ON_Pin); else GPIO_SetBits(GPIOA,   FAN2_ON_Pin); } break;
              case pFAN1:     { if( !Z ) GPIO_ResetBits(GPIOA,   FAN1_ON_Pin); else GPIO_SetBits(GPIOA,   FAN1_ON_Pin); } break;
              case pWPUMP:    { if( !Z ) GPIO_ResetBits(GPIOA,  WPUMP_ON_Pin); else GPIO_SetBits(GPIOA,  WPUMP_ON_Pin); } break;
              case pSVALVE:   { if( !Z ) GPIO_ResetBits(GPIOA, SVALVE_ON_Pin); else GPIO_SetBits(GPIOA, SVALVE_ON_Pin); } break;
            }

            break;

        case HOSTCMD_GET_ALLAD:                                         // All 16 A/D's returned.  0 returned for non-configured Sensors

            AtoD_GetAllReadings( (u32 *)&Dptr[8] );                     //  Note the 8-byte alignment of the address.
            PacketSend_pakt( HOSTCMD_GET_ALLAD, 67, Dptr, (u32)Dptr );  //  (16*4) + 3 empty bytes: [5],[6],[7]
            ReturnTheBuffer = False;                                    //  Buffer will be returned in the Send driver
            break;

        case HOSTCMD_GET_VALUE:                                         // bout anything

            tmp = get_value_cmd( Dptr[SODi] );                          // 256 possible variables can be retrieved
            UD_Print8("CMD_GET_VALUE parm: ",  Dptr[SODi]);
            memcpy(&Dptr[PAKTSODi], (void *)&tmp, 4);                   // copy retrieved value in the the packet
            PacketSend_pakt( HOSTCMD_GET_VALUE, 4, Dptr, (u32)Dptr );   // 4 bytes is the total
            ReturnTheBuffer = False;                                    //    ** Buffer will be returned in the Send driver
            break;

        case HOSTCMD_DORESET:
            /* BING, BING.  Hit something. */
            UD_Print8("DORESET pakt: ",  Dptr[SODi]);
            break;

        default: UD_Print8N("ERROR DEFAULT, cmd: ",  Dptr[CMDi]);       // todo: Log this condition
        }

        if( ReturnTheBuffer == True ) { DataBuffer_Return( Dptr ); }    // Very Important to return the damn buffer!
    }
}


GLOBALLY_VISIBLE void PacketRecv_Qsend( u8 *bufptr )                    // All received packets are Q'ed by calling this function.
{
    xQueueSend( Globals.RQhand, &bufptr, 0);                            // Bing!
}



//
//  equivalent to an init function:  called once at startup
//
GLOBALLY_VISIBLE void PacketRecv_TaskCreate( void )
{
    Globals.RQhand = xQueueCreateStatic( Q_ELEMENTS, Q_ITEMSIZE, Globals.StaticQBuffer, &Globals.StaticQueue);

    xTaskCreate((TaskFunction_t)PacketRecv_Thread,(const char * const)"PacketRecv", 128, 0, 1, &Globals.RT_Thand);
}



static u32 get_value_cmd( u8 parm )
{
    float  Fval;
    u16    Rval;
    u8     chan = 0;

    switch( parm )
    {
      case hAD_AI1:  chan=0;    break;
      case hAD_AI2:  chan=1;    break;
      case hAD_AI3:  chan=2;    break;
      case hAD_AI4:  chan=3;    break;
      case hAD_AI5:  chan=4;    break;
      case hAD_AI6:  chan=5;    break;
      case hAD_AI7:  chan=6;    break;
      case hAD_AI8:  chan=7;    break;
      case hAD_AI9:  chan=8;    break;
      case hAD_AI10: chan=9;    break;
      case hAD_AI11: chan=10;   break;
      case hAD_AI12: chan=11;   break;
      case hAD_AI13: chan=12;   break;
      case hAD_AI14: chan=13;   break;
      case hAD_AI15: chan=14;   break;
      case hAD_AI16: chan=15;   break;

      case hI0CBt:   Fval = CSVar[CAB_T].Value;       break;
      case hI0SYp:   Fval = CSVar[SYS_P].Value;       break;
      case hI0CSp:   Fval = CSVar[SUCT_P].Value;      break;
      case hI0CSt:   Fval = CSVar[SUCT_T].Value;      break;
      case hI0WSt:   Fval = CSVar[WETSUCT_T].Value;   break;
      case hI0CLt:   Fval = CSVar[CONDLIQ_T].Value;   break;
      case hI0TWt:   Fval = CSVar[WATER_T].Value;     break;
      case hI0SYw:   Fval = CSVar[SYS_POW].Value;     break;
      case hI0CPw:   Fval = CSVar[COM_POW].Value;     break;
    }

    if( parm >= hAD_AI1 && parm <= hAD_AI16 )
    {
        AtoD_Get_Reading( chan, &Rval, &Fval );
        return (u32)(Fval*1000.0);
    }

    if( parm >= hI0CBt && parm <= hI0CPw )
    {
        return (u32)(Fval*1000.0);
    }

    return 0;
}






