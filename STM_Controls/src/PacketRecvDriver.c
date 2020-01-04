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
#include "PacketRecvThread.h"

#define STATE_GET_INPUT_BUFFER     0
#define STATE_LOOKING_FOR_AB       1
#define STATE_LOOKING_FOR_5F       2
#define STATE_GET_SEQN             3
#define STATE_GET_LEN              4
#define STATE_WAIT_THEN_GO         5
#define STATE_WAITCOMPLETE         6

// local to stm32f4xx_dma.c
#define RESERVED_MASK           (uint32_t)0x0F7D0F7D




#define PAKT_F1     0
#define PAKT_F2     1
#define PAKT_SEQN   2
#define PAKT_LEN    3
#define PAKT_CMD    4
#define PAKT_SOD    5


//#define PAKT_CSUM   5




#define COMMS_RECEIVE_ENABLED      1
#define COMMS_RECEIVE_DISABLED     0



typedef struct
{
    DMA_InitTypeDef  dmaStruct;
    u8               state_machine;
    u32              Atime;
    u8              *bufbase;
    bool             RecvEnabled;
    u8               paktseqn;
    u8               paktcmd;
    u8               paktlen;
} GLOBALS_PacketRecv_t;

static GLOBALS_PacketRecv_t    Globals;



static void Set_ReceiveEnabled_Pin( u32 val )
{
    if( val == COMMS_RECEIVE_ENABLED )
    {
        Globals.RecvEnabled = TRUE;
        Hammer(1);
    }
    else
    {
        Globals.RecvEnabled = FALSE;
        Hammer(0);
    }
}


void PacketRecvDriver_Init( void )
{
    DMA_InitTypeDef  *dm = &Globals.dmaStruct;

    Set_ReceiveEnabled_Pin( COMMS_RECEIVE_DISABLED );

    DMA_StructInit( dm );
      dm->DMA_Channel            = DMA_Channel_4;
      dm->DMA_PeripheralBaseAddr = (u32)STM_REGISTER &USART2->DR;
      dm->DMA_DIR                = DMA_DIR_PeripheralToMemory;
      dm->DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
      dm->DMA_MemoryInc          = DMA_MemoryInc_Enable;
      dm->DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
      dm->DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
      dm->DMA_Mode               = DMA_Mode_Normal;
      dm->DMA_Priority           = DMA_Priority_Medium;
      dm->DMA_FIFOMode           = DMA_FIFOMode_Disable;
      dm->DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull;
      dm->DMA_MemoryBurst        = DMA_MemoryBurst_Single;
      dm->DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
    DMA_Init( DMA1_Stream5, dm );

    Globals.Atime         = 0;
    Globals.state_machine = STATE_GET_INPUT_BUFFER;
}




void PacketRecvDriver_Process( void )
{
    u8                      thechar,*dptr;
    u32                     i,csum;
    GLOBALS_PacketRecv_t   *G = &Globals;


    switch( G->state_machine )
    {
    case STATE_GET_INPUT_BUFFER:

        if( !(G->bufbase=DataBuffer_Get()) )  { return; }              //u8   *DataBuffer_Get   ( void     );

        G->state_machine = STATE_LOOKING_FOR_AB;
        Set_ReceiveEnabled_Pin( COMMS_RECEIVE_ENABLED  );
        FALL_THRU;

    case STATE_LOOKING_FOR_AB:

        if( !(STM_REGISTER USART2->SR & USART_FLAG_RXNE) ) { return; }
        thechar = STM_REGISTER USART2->DR & 0xff;

        if( thechar == 0xAB ) { G->state_machine = STATE_LOOKING_FOR_5F; }
        FALL_THRU;

    case STATE_LOOKING_FOR_5F:

        if( !(STM_REGISTER USART2->SR & USART_FLAG_RXNE) ) { return; }
        thechar = STM_REGISTER USART2->DR & 0xff;

        G->state_machine = (thechar == 0x5F) ? STATE_GET_SEQN : STATE_LOOKING_FOR_AB;
        FALL_THRU;

    case STATE_GET_SEQN:

        if( !(STM_REGISTER USART2->SR & USART_FLAG_RXNE) ) { return; }
        G->paktseqn = STM_REGISTER USART2->DR & 0xff;

        G->state_machine = STATE_GET_LEN;
        FALL_THRU;

    case STATE_GET_LEN:

        if( !(STM_REGISTER USART2->SR & USART_FLAG_RXNE) ) { return; }
        G->paktlen = STM_REGISTER USART2->DR & 0xff;
        G->bufbase[0] = G->paktlen;

        STM_REGISTER DMA1_Stream5->M0AR = (u32)&G->bufbase[1];
        STM_REGISTER DMA1_Stream5->NDTR = G->paktlen + 2;         // CMD and CSUM are the '+2'

        G->state_machine = STATE_WAIT_THEN_GO;
        FALL_THRU;

    case STATE_WAIT_THEN_GO:

        if( !(USART2->SR & USART_FLAG_RXNE) ) { return; }                         // Critical timing right here! (maybe setup interrupt for this event)

        STM_REGISTER DMA1_Stream5->CR |= (uint32_t)DMA_SxCR_EN;                   // DMA_Cmd( DMA1_Stream5, ENABLE );
        STM_REGISTER USART2->CR3      |= USART_DMAReq_Rx;                         // USART_DMACmd( USART2, USART_DMAReq_Tx, ENABLE );
        G->state_machine               = STATE_WAITCOMPLETE;
        FALL_THRU;

    case STATE_WAITCOMPLETE:

        if( !(STM_REGISTER DMA1->HISR & DMA_FLAG_TCIF5) ) { return; }

        Set_ReceiveEnabled_Pin( COMMS_RECEIVE_DISABLED );

        STM_REGISTER DMA1->HIFCR = (uint32_t)((DMA_FLAG_TCIF5 | DMA_FLAG_HTIF5 | DMA_FLAG_FEIF5) & RESERVED_MASK);

        for( csum=0, dptr=G->bufbase, i=0; i < (G->paktlen+2); ++i )
        {
            csum += *dptr++;
        }

        if( G->bufbase[2+G->paktlen] == (u8)csum )
        {
            PacketRecv_Qsend( G->bufbase );
        }
        else
        {
            // todo: Log Event
            UD_Print8N("csum Mismatch.  pakt:  ",   G->bufbase[2+G->paktlen]);
            UD_Print8 ("   calc", (u8)(csum & 0xff));
            DataBuffer_Return( G->bufbase );
        }

        G->state_machine = STATE_GET_INPUT_BUFFER;
        return;
    }
}


void show_packetrecv_state( void )
{
    UD_Print8("state: ", Globals.state_machine);
}











