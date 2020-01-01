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
#include "semphr.h"
#include "proj_common.h"
#include "Uart.h"
#include "Cmds.h"
#include "string.h"
#include "gpio_CDC3.h"



typedef struct
{
    uint32_t      SysTicks;
} GLOBALS_t;



static GLOBALS_t   TimrGlobals;


void init_timer( void )
{
    TimrGlobals.SysTicks = 0;
}


uint32_t GetSysTick( void )
{
    return TimrGlobals.SysTicks;
}


uint32_t GetSysDelta( uint32_t OriginalTime )
{
    uint32_t v = TimrGlobals.SysTicks;

    if( v >= OriginalTime )
        return( v - OriginalTime );
    else
        return( ~OriginalTime + 1 + v );
}


void Increment_SysTicks( void )
{
    ++TimrGlobals.SysTicks;
}



