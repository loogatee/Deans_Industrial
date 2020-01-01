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
#include "Cmds.h"
#include "string.h"
#include "i2c.h"
#include "timer.h"
#include "Utils.h"
#include "main.h"
#include "gpio_CDC3.h"
#include "AtoDs.h"
#include <stdio.h>
#include "PacketDrivers.h"
#include "monitor_host.h"
#include "PacketSendThread.h"


#define TIMEOUT_VAL     60


#define RTN_CONTINUE    0
#define RTN_SUCCESS     1

#define I2C_STATE_GETJOB                 0
#define I2C_STATE_GO_AGAIN               1
#define I2C_STATE_SENDSTART              2
#define I2C_STATE_START_COMPLETE         3
#define I2C_STATE_WRITE_SLAVEADDR        4
#define I2C_STATE_WRITE_SLAVEADDR_RW     5
#define I2C_STATE_READ_THE_DATA          6
#define I2C_STATE_WAIT_MBR               7
#define I2C_STATE_WAIT_STOP              8
#define I2C_STATE_DO_WRITEONLY           9
#define I2C_STATE_SETTLE_WAIT            10
#define I2C_STATE_FINISH_CLEANUP         11
#define I2C_STATE_BUSY_WAIT2             12
#define I2C_STATE_BUSY_WAIT3             13

#define I2C_WRITE_SUBSTATE_START         0
#define I2C_WRITE_SUBSTATE_WAITCOMPLETE  1



#define WAIT_TYPE_WHOLE_BUNCH_O_BITS     1
#define WAIT_TYPE_BTF_ONLY               2
#define WAIT_TYPE_ADDR_ONLY              3

#define SLAVE_ADDR   0x28



typedef struct
{
    u16       ConfiguredChans;
    u8        StateMachine;
    u32       StartTime;
    u8        CmdType;
    u8        Cleanup_Count;
    u8        Readbuf[2];
    u8        ActiveChannel;
    bool      FirstTime_Flag;
    u8        Nsamples;
    bool      ConfigTrigger;
} I2C_1_GLOBALS_t;

static I2C_1_GLOBALS_t  Globals;




static u8    write_complete( unsigned int   wait_type );
static void  set_next_channel( void );
static void  cleanup( void );







//
//   'C' A/D's are 0..15
//
//   Labels on the board are 1..16
//
//      Examples:
//              ADchans[1]  = AI2
//              ADchans[10] = AI11
//
GLOBALLY_VISIBLE void I2C_1_master_Init( void )
{
    I2C_1_GLOBALS_t  *G = &Globals;

    G->ConfiguredChans = 0x0001;                              // I0.CDv, Chan 0, Always configured, Always Measured.
    G->ActiveChannel   = 15;                                  // state GETJOB will increment this by 1 by calling 'set_next_channel()'
    G->FirstTime_Flag  = true;                                // 1st message to chip is a WRITEONLY that sets sampling to 30hz
    G->Cleanup_Count   = 0;                                   // prevents infinite recovery attempts on i2c bus issues
    G->StateMachine    = I2C_STATE_GETJOB;                    // start with GETJOB
    G->ConfigTrigger   = False;
}



GLOBALLY_VISIBLE void I2C_1_master_Process( void )
{
    u16 S1,S2,*sptr;
    I2C_1_GLOBALS_t  *G = &Globals;


    if( (G->StateMachine != I2C_STATE_GETJOB) && (G->StateMachine < I2C_STATE_FINISH_CLEANUP) )
    {
        if( GetSysDelta( G->StartTime ) >= TIMEOUT_VAL )                               // Safety valve: too long to complete this job ?
        {
            UD_Print8( "TO! State: ", G->StateMachine );                               //     Message out to debug port (optional)
            cleanup();                                                                 //     Attempt to recover the bus
            return;                                                                    //     return immediately
        }
    }

    switch( G->StateMachine )
    {
    case I2C_STATE_GETJOB:

        set_next_channel();                                                       // selects next channel and sets the mux pins

        G->Nsamples  = 0;                                                         // tracking for: toss 1st sample, keep the second
        G->StartTime = GetSysTick();                                              // Initialize the "safety valve" ticker
        FALL_THRU;

    case I2C_STATE_GO_AGAIN:

        G->CmdType = I2C_CMDTYPE_RW;
        if( G->FirstTime_Flag == True )
        {
            G->FirstTime_Flag = False;                                            // Send the Write-Only Job only once
            G->CmdType        = I2C_CMDTYPE_WRITEONLY;                            // Change CmdType if this is the 1st time thru the loop
        }

        G->StateMachine = I2C_STATE_SENDSTART;                                    // transition to SENDSTART
        FALL_THRU;                                                                // no need to break

    case I2C_STATE_SENDSTART:

        if( STM_REGISTER I2C1->SR2 & I2C_SR2_BUSY ) {return;}                     // Must wait for IDLE

        G->StateMachine = I2C_STATE_START_COMPLETE;                               // Have IDLE, new state will be START_COMPLETE
        STM_REGISTER I2C1->CR1 |= I2C_CR1_START;                                  // START!
        FALL_THRU;                                                                // no need to break

    case I2C_STATE_START_COMPLETE:

        S1 = STM_REGISTER I2C1->SR1;                                                          // local copy of SR1 reg
        S2 = STM_REGISTER I2C1->SR2;                                                          // local copy of SR2 reg
        if( !(S1 & I2C_SR1_SB) ||  !(S2 & I2C_SR2_MSL) || !(S2 & I2C_SR2_BUSY) ) { return; }  // if any of these bits NOT set, get out

        if( G->CmdType == I2C_CMDTYPE_WRITEONLY )                                             // Check to see if this job is WRITE-ONLY
        {
            G->StateMachine = I2C_STATE_WRITE_SLAVEADDR;                                      // Got it!  Transition to WRITE_SLAVEADDR
            STM_REGISTER I2C1->DR = SLAVE_ADDR;                                               // transmits 1 byte
        }
        else
        {
            G->StateMachine = I2C_STATE_WRITE_SLAVEADDR_RW;                                   // Got it!  Transition to WRITE_SLAVEADDR_RW
            STM_REGISTER I2C1->DR = SLAVE_ADDR | 1;                                           // *** Note the 1-bit OR'd in with the Slave Address
        }

        return;

    case I2C_STATE_WRITE_SLAVEADDR:
            
        if( write_complete( WAIT_TYPE_WHOLE_BUNCH_O_BITS) == RTN_CONTINUE ) { return; }             // spin till ALL bits are set
                                                                                                    // ELSE success
        G->StateMachine = I2C_STATE_DO_WRITEONLY;                                                   //    new state is DO_WRITEONLY
        STM_REGISTER I2C1->DR = 1;                                                                  // *** Note:   0=60hz sampling, 1=30hz sampling
        return;


    case I2C_STATE_WRITE_SLAVEADDR_RW:

        if( write_complete( WAIT_TYPE_ADDR_ONLY ) == RTN_CONTINUE ) { return; }

        G->StateMachine = I2C_STATE_READ_THE_DATA;                                          // new state = READ_THE_DATA
        STM_REGISTER I2C1->CR1 |= I2C_NACKPosition_Next;                                    // Sends Nack
                                                                                            //__disable_irq();
        Hammer( STM_REGISTER I2C1->SR2 );                                                   // Side effect is to Clear ADDR flag
        STM_REGISTER I2C1->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);                       // I2C_AcknowledgeConfig(I2Cx, DISABLE);
        FALL_THRU;                                                                          //__enable_irq();

    case I2C_STATE_READ_THE_DATA:

        if( !(STM_REGISTER I2C1->SR1 & I2C_SR1_BTF) ) { return; }                 // Spin here till BTF bit is set
                                                                                  //__disable_irq();
        STM_REGISTER I2C1->CR1 |= I2C_CR1_STOP;                                   // Generate a STOP condition
        G->Readbuf[1]   = STM_REGISTER I2C1->DR;                                  // Reads byte 1 and __enable_irq();
        G->Readbuf[0]   = STM_REGISTER I2C1->DR;                                  // Reads byte 2, **Note the Byte swap!!
        G->StateMachine = I2C_STATE_WAIT_STOP;
        FALL_THRU;

    case I2C_STATE_WAIT_STOP:

        if( STM_REGISTER I2C1->SR1 & I2C_SR1_STOPF ) { return; }                  // Spin here until STOPF is cleared

        G->StartTime = GetSysTick();                                              // Start timer for the new transaction

        if( (G->CmdType == I2C_CMDTYPE_RW) && (++G->Nsamples == 1) )
        {
            G->StateMachine = I2C_STATE_SETTLE_WAIT;
        }
        else
        {
            if( G->CmdType == I2C_CMDTYPE_RW )
            {
                sptr = (u16 *)G->Readbuf;
                AtoD_Set_value( G->ActiveChannel, *sptr );                  // Stores the raw reading, generates a cooked reading
            }
            G->StateMachine = I2C_STATE_BUSY_WAIT2;                         // All Done: Wait a bit, then move to the next channel
        }
        return;


    case I2C_STATE_DO_WRITEONLY:

        if( write_complete( WAIT_TYPE_BTF_ONLY ) == RTN_CONTINUE ) { return; }

        STM_REGISTER I2C1->CR1 |= I2C_CR1_STOP;                                   // Generates STOP condition
        G->StateMachine = I2C_STATE_WAIT_STOP;                                    // mop up for transaction just completed
        return;

    case I2C_STATE_SETTLE_WAIT:

        if( GetSysDelta(G->StartTime) < 35 ) { return; }                          // spin for 35ms:  Allows measurement to settle

        G->StartTime    = GetSysTick();                                           // Start timer for the new transaction
        G->StateMachine = I2C_STATE_GO_AGAIN;                                     // State to start up the sample which will be saved
        return;

    case I2C_STATE_BUSY_WAIT2:

        if( GetSysDelta(G->StartTime) < 47 ) { return; }
        G->StateMachine = I2C_STATE_GETJOB;
        return;

    case I2C_STATE_BUSY_WAIT3:

        if( GetSysDelta(G->StartTime) < 8000 ) { return; }
        G->StateMachine = I2C_STATE_GETJOB;
        return;

    case I2C_STATE_FINISH_CLEANUP:

        init_gpioI2C();                                                                // re-inits Pins to I2C functionality

        if( (G->Cleanup_Count < 50) && (STM_REGISTER I2C1->SR2 & I2C_SR2_BUSY) )       // cleanup_count prevents an endless loop
        {                                                                              // We're really looking for the BUSY bit to be clear.
            cleanup();                                                                 //   if BUSY, attempts another reset of the I2C line
            return;                                                                    //   stays in this state, return and come back shortly
        }

        UD_Print8("Cleanup_Count: ", G->Cleanup_Count);                                // Either non-busy, or REALLY hosed.  Show cleanup_count.

        G->StateMachine = I2C_STATE_GETJOB;                                            // now transition to GETJOB
        if( G->Cleanup_Count == 50)
        {
            G->StartTime    = GetSysTick();                                            // Initialize the "safety valve" ticker
            G->StateMachine = I2C_STATE_BUSY_WAIT3;
        }

        G->Cleanup_Count  = 0;
        G->ActiveChannel  = 15;                                                    // so that 1st channel sampled will be chan 0
        G->FirstTime_Flag = true;                                                  // Forces WRITEONLY
        return;
    }
}



static u8 write_complete( unsigned int  WaitType )
{
    u16 S1,S2;

    S1 = STM_REGISTER I2C1->SR1;

    if( WaitType == WAIT_TYPE_WHOLE_BUNCH_O_BITS )
    {
        S2 = STM_REGISTER I2C1->SR2;

        if( !(S1 & I2C_SR1_ADDR) || !(S1 & I2C_SR1_TXE) || !(S2 & I2C_SR2_MSL) || !(S2 & I2C_SR2_BUSY) || !(S2 & I2C_SR2_TRA) )
        {
            return RTN_CONTINUE;                                  //  stay here, keep looking for BF to be 0
        }
    }
    else if( WaitType == WAIT_TYPE_BTF_ONLY )
    {
        if( !(S1 & I2C_SR1_BTF) ) { return RTN_CONTINUE; }
    }
    else if( WaitType == WAIT_TYPE_ADDR_ONLY )
    {
        if( !(S1 & I2C_SR1_ADDR) ) { return RTN_CONTINUE; }
    }

    return RTN_SUCCESS;                                               // write substate done, byte transfer succeeded
}



//
//
//
static void set_next_channel( void )
{
    I2C_1_GLOBALS_t  *G = &Globals;

    if( G->ConfigTrigger == True )
    {
        G->ConfigTrigger = False;
        AtoD_Init();
    }

    G->ConfiguredChans = AtoD_Get_ConfiguredChannels();

    do
    {
        if( ++G->ActiveChannel == NUM_AD_CHANS )
            G->ActiveChannel = 0;
    } while (!(G->ConfiguredChans & (1 << G->ActiveChannel)));

    GPIO_ResetBits( GPIOB, MUX1_EN_Pin );
    GPIO_ResetBits( GPIOB, MUX2_EN_Pin );
    GPIO_ResetBits( GPIOB, MUXBIT_0_Pin | MUXBIT_1_Pin | MUXBIT_2_Pin );

    (G->ActiveChannel & 1) ? GPIO_ResetBits( GPIOB, MUXBIT_0_Pin) : GPIO_SetBits( GPIOB, MUXBIT_0_Pin);
    (G->ActiveChannel & 2) ? GPIO_ResetBits( GPIOB, MUXBIT_1_Pin) : GPIO_SetBits( GPIOB, MUXBIT_1_Pin);
    (G->ActiveChannel & 4) ? GPIO_ResetBits( GPIOB, MUXBIT_2_Pin) : GPIO_SetBits( GPIOB, MUXBIT_2_Pin);
    (G->ActiveChannel & 8) ? GPIO_SetBits  ( GPIOB,  MUX2_EN_Pin) : GPIO_SetBits( GPIOB,  MUX1_EN_Pin);
}



static void cleanup( void )
{
    GPIO_InitTypeDef Xgpio;
    I2C_1_GLOBALS_t  *G = &Globals;

    if( ++G->Cleanup_Count == 1 )
    {
        UD_Print8( "cleanup, State: ", G->StateMachine );
        UD_Print32("  SR1: ", STM_REGISTER I2C1->SR1);
        UD_Print32("  SR2: ", STM_REGISTER I2C1->SR2);
    }

    STM_REGISTER I2C1->CR1 |= I2C_CR1_SWRST;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,   DISABLE);
    STM_REGISTER GPIOB->AFR[0] = 0;

    GPIO_StructInit(&Xgpio);
     Xgpio.GPIO_Mode  = GPIO_Mode_OUT;
     Xgpio.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
     Xgpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;
     Xgpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &Xgpio);

    GPIO_SetBits(GPIOB, GPIO_Pin_6|GPIO_Pin_7);

    G->StateMachine = I2C_STATE_FINISH_CLEANUP;
}


GLOBALLY_VISIBLE void I2C_1_master_NewConfig( void )
{
    Globals.ConfigTrigger = True;
}









