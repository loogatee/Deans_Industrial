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

#define Q_ELEMENTS    3
#define Q_ITEMSIZE    sizeof( uint32_t )

typedef struct
{
    uint32_t           Ntime;
    TaskHandle_t       RT_Thand;
    QueueHandle_t      RQhand;
    StaticQueue_t      StaticQueue;
    u8                 DataBuffer[ Q_ELEMENTS * Q_ITEMSIZE ];
} GLOBALS_PRT_t;




static GLOBALS_PRT_t   Globals;
static u8              dloopBuffer[256];



STATIC void PacketRecv_Thread(void const * argument)
{
    u8   *Dptr;

    while(1)
    {
        xQueueReceive( Globals.RQhand, &Dptr, portMAX_DELAY );

        // UD_Print32("Qrecv: ", (u32)Dptr);


        // [0] is len
        // [1] is cmd
        // [2] is data[0]
        switch( Dptr[1] )
        {
        case HOSTCMD_IMUP:

            if( !memcmp(&Dptr[2],"IMUP",4) )
            {
                UD_Print8("IMUP paktlen: ",  (u8)Dptr[0]);
                MonitorHost_IMUP_pakt();
            }
            break;

        case HOSTCMD_ADCONFIG:

            UD_Print8("ADCONFIG paktlen: ",  (u8)Dptr[0]);
            AtoD_Set_NewConfig( Dptr[0], &Dptr[2] );
            break;

        case HOSTCMD_DLOOP_OH:

            //UD_Print8("DLOOP_OH paktlen: ",  (u8)Dptr[0]);
            memcpy((void *)&dloopBuffer[PAKTSODi],(void *)&Dptr[2], Dptr[0]);
            PacketSend_DloopResp( Dptr[0], dloopBuffer );
            break;

        default: UD_Print8N("ERROR DEFAULT, cmd: ",  (u8)Dptr[1]);
        }


        DataBuffer_Return( Dptr );
    }
}


GLOBALLY_VISIBLE void PacketRecv_Qsend( u8 *bufptr )
{
    //UD_Print32("Qsend: ", (u32)bufptr);
    xQueueSend( Globals.RQhand, &bufptr, 0);
}




GLOBALLY_VISIBLE void PacketRecv_TaskCreate( void )
{
    Globals.RQhand = xQueueCreateStatic( Q_ELEMENTS, Q_ITEMSIZE, Globals.DataBuffer, &Globals.StaticQueue);

    xTaskCreate((TaskFunction_t)PacketRecv_Thread,(const char * const)"PacketRecv", 128, 0, 1, &Globals.RT_Thand);
}











