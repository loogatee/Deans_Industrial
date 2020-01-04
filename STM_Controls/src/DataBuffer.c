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


#define DBUF_NUMBUFS     3


typedef struct
{
    u8    *db_ptr;
    bool   IsAvail;
} DBUFI;

static u8 dbuf_one[256];
static u8 dbuf_two[256];
static u8 dbuf_thr[256];


static DBUFI  dbuf_Items[DBUF_NUMBUFS];      // Queue of available buffers



void DataBuffer_Init( void )
{
    dbuf_Items[0].db_ptr = dbuf_one;         // Assign all the pointers to the buffers
    dbuf_Items[1].db_ptr = dbuf_two;
    dbuf_Items[2].db_ptr = dbuf_thr;

    dbuf_Items[0].IsAvail = True;
    dbuf_Items[1].IsAvail = True;
    dbuf_Items[2].IsAvail = True;

    UD_Print32("buf1: ", (u32)&dbuf_one[0]);
    UD_Print32("buf2: ", (u32)&dbuf_two[0]);
    UD_Print32("buf3: ", (u32)&dbuf_thr[0]);
}


u8 *DataBuffer_Get( void )
{
    u32 i;

    for( i=0; i < DBUF_NUMBUFS; ++i )
    {
        if( dbuf_Items[i].IsAvail == True )
        {
            dbuf_Items[i].IsAvail = False;
            return dbuf_Items[i].db_ptr;
        }
    }
    return 0;
}

//
//
void DataBuffer_Return( u8 *Dptr )
{
    u32  i;
    
    for( i=0; i < DBUF_NUMBUFS; ++i )
    {
        if( dbuf_Items[i].db_ptr == Dptr )
        {
            dbuf_Items[i].IsAvail = True;
            return;
        }
    }
}




