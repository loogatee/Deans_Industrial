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
#include "PacketDrivers.h"
#include "PacketSendThread.h"
#include "monitor_host.h"


#define STATE_WAIT_HOSTUP          1
#define STATE_WAITCOMPLETE         2
#define STATE_WAIT_THEN_GO         3

// local to stm32f4xx_dma.c
#define RESERVED_MASK           (uint32_t)0x0F7D0F7D



typedef struct
{
    DMA_InitTypeDef  dmaStruct;
    u8               state_machine;
    u32              Atime;
    u8              *paktptr;
    u8               Trigger;
    u8               xxSeqn;
} GLOBALS_PacketSend_t;

static GLOBALS_PacketSend_t    Globals;



void PacketSendDriver_Init( void )
{
    DMA_InitTypeDef  *dm = &Globals.dmaStruct;

    DMA_StructInit( dm );
      dm->DMA_Channel            = DMA_Channel_4;
      dm->DMA_PeripheralBaseAddr = (u32) STM_REGISTER &USART2->DR;
      dm->DMA_DIR                = DMA_DIR_MemoryToPeripheral;
      dm->DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
      dm->DMA_MemoryInc          = DMA_MemoryInc_Enable;
      dm->DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
      dm->DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
      dm->DMA_Mode               = DMA_Mode_Normal;
      dm->DMA_Priority           = DMA_Priority_Low;
      dm->DMA_FIFOMode           = DMA_FIFOMode_Disable;
      dm->DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull;
      dm->DMA_MemoryBurst        = DMA_MemoryBurst_Single;
      dm->DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
    DMA_Init( DMA1_Stream6, dm );

    Globals.state_machine = STATE_WAIT_HOSTUP;
    Globals.Atime         = 0;
    Globals.Trigger       = 0;
    Globals.xxSeqn        = 0;

    //   [0] = fixed0         5 Header Bytes
    //   [1] = fixed1
    //   [2] = seqn
    //   [3] = len      to retrieve is:  len + 1 + 1        len bytes + 1 Cmd Byte + 1 Csum Byte
    //   [4] = cmd
    //   [5] = data[0]
    //   [6] = data[1]
    //    .....
    //  [56] = data[51] = 'Z'
    //  [57] = data[52] = '\r'
    //  [58] = data[53] = '\n'
    //  [59] = csum                    csum byte is not included in the length
    //
    //memcpy(dbgpacket, "\314\127\001\066\146abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n\001",60);
}



static void doCsum( u8  *Pptr )
{
    u32 csum,i;
    u8  lendata,*dptr;

    lendata = Pptr[PAKTLENi];

    //
    // checksum includes BOTH len byte and cmd byte
    //
    for( csum=0, dptr=&Pptr[PAKTLENi], i=0; i < (lendata+2); ++i )
    {
        csum += *dptr++;
    }

    Pptr[PAKTSODi+lendata] = (u8)(csum & 0xff);
}

//
//   6 bytes added to every data buffer
//      - magic1
//      - magic2
//      - seqn
//      - len
//      - cmd
//      - csum    (only one added at the end)
//
void PacketSendDriver_Process( void )
{
    GLOBALS_PacketSend_t   *G = &Globals;

    switch( G->state_machine )
    {
    case STATE_WAIT_HOSTUP:

        if( !MonitorHost_IsUp() ) { return; }                                      // one-time Wait after reset, then never in here again
        G->state_machine = STATE_WAIT_THEN_GO;
        FALL_THRU;

    case STATE_WAIT_THEN_GO:

        if( G->Trigger == 0 ) { return; }                                           // stuck here until Triggered

        doCsum(G->paktptr);                                                         // Checksum byte placed at end of data buffer

        G->Trigger            = 0;                                                  // reset to: not triggered
        G->paktptr[PAKTMAG1i] = 0xCC;                                               // magic1
        G->paktptr[PAKTMAG2i] = 0x57;                                               // magic2
        G->paktptr[PAKTSEQNi] = G->xxSeqn++;                                        // Sequence Number

        STM_REGISTER DMA1_Stream6->M0AR  = (u32)G->paktptr;                         // start addr of packet
        STM_REGISTER DMA1_Stream6->NDTR  = G->paktptr[PAKTLENi] + 6;                // length of data + 6 header bytes
        STM_REGISTER DMA1_Stream6->CR   |= (uint32_t)DMA_SxCR_EN;                   // DMA_Cmd( DMA1_Stream6, ENABLE );
        STM_REGISTER USART2->CR3        |= USART_DMAReq_Tx;                         // USART_DMACmd( USART2, USART_DMAReq_Tx, ENABLE );

        G->state_machine = STATE_WAITCOMPLETE;                                      // state to monitor completion
        FALL_THRU;                                                                  // ok to fall through

    case STATE_WAITCOMPLETE:

        if( !(STM_REGISTER USART2->SR & USART_FLAG_TC ) ) { return; }               // Transfer Complete in the Usart Status Register
        if( !(STM_REGISTER DMA1->HISR & DMA_FLAG_TCIF6) ) { return; }               // Transfer Complete in the DMA Status Register

        STM_REGISTER DMA1->HIFCR = (uint32_t)((DMA_FLAG_TCIF6 | DMA_FLAG_HTIF6 | DMA_FLAG_FEIF6) & RESERVED_MASK);    // clean up and ready for next
        STM_REGISTER USART2->SR  = (uint16_t)~USART_FLAG_TC;                                                          // clear out TC

        G->state_machine = STATE_WAIT_THEN_GO;                                      // Look for another packet to send
        G->Atime         = GetSysTick();                                            // todo: needed?
        PacketSend_xfer_Done();                                                     // set semaphore to indicate Done!
        return;
    }

}


void PacketSendDriver_Go( u8 *Packetptr )
{
    GLOBALS_PacketSend_t   *G = &Globals;

    // UD_PrintSTR("SendDriver_Go\n\r");

    G->paktptr = Packetptr;
    G->Trigger = 1;
}










