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
#include "semphr.h"
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
#include "PacketDrivers.h"
#include "Host_Cmds.h"


typedef struct
{
    u32  F1;
    u32  F2;
} Qvalues_t;


#define Q_ELEMENTS    5
#define Q_ITEMSIZE    sizeof(Qvalues_t)


// typedef void (*CALLBACK)(void);

typedef struct
{
    TaskHandle_t       ST_Thand;
    QueueHandle_t      SQhand;
    StaticQueue_t      StaticQueue;
    u8                 DataBuffer[ Q_ELEMENTS * Q_ITEMSIZE ];
    SemaphoreHandle_t  PacketDone_Sem;
} GLOBALS_PST_t;





static GLOBALS_PST_t   Globals;



STATIC void PacketSend_Thread(void const * argument)
{
    Qvalues_t  QD;

    while(1)
    {
        xQueueReceive( Globals.SQhand, &QD, portMAX_DELAY );
        PacketSendDriver_Go( (u8 *)QD.F1, QD.F2 );
        xSemaphoreTake( Globals.PacketDone_Sem, portMAX_DELAY );
    }
}

GLOBALLY_VISIBLE void PacketSend_xfer_Done( void )
{
    xSemaphoreGive( Globals.PacketDone_Sem );
}

//
//  serves the purpose of an 'Init' function:
//     - creates a message Q
//     - creates a semaphore
//     - creates a task
//  **** (Just sayin its more than 'TaskCreate')
//
GLOBALLY_VISIBLE void PacketSend_TaskCreate( void )
{
    Globals.SQhand         = xQueueCreateStatic( Q_ELEMENTS, Q_ITEMSIZE, Globals.DataBuffer, &Globals.StaticQueue);
    Globals.PacketDone_Sem = xSemaphoreCreateBinary();

    xTaskCreate((TaskFunction_t)PacketSend_Thread,(const char * const)"PacketSend", 128, 0, 1, &Globals.ST_Thand);
}


GLOBALLY_VISIBLE void PacketSend_pakt( u8 cmd, u8 len, u8 *dataptr, u32 zero_or_retBuf )
{
    Qvalues_t   qv;

    qv.F1 = (u32)dataptr;
    qv.F2 = zero_or_retBuf;

    dataptr[PAKTLENi] = len;
    dataptr[PAKTCMDi] = cmd;
    xQueueSend( Globals.SQhand, &qv, 0);
}



