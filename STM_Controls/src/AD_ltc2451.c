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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "proj_common.h"
#include "Uart.h"
#include "Cmds.h"
#include "timer.h"
#include "i2c.h"
#include "Utils.h"
#include "gpio_CDC3.h"



#define STATE_GET_TIME     0
#define STATE_SHOW_TIME    1
#define STATE_DO_WAIT      2







static u8   ad_rbuf[10];           // for returned data from the I2C driver
static u32  ad_i2c1_rcompl;        // location where I2C driver will use to signal completion, reads
static u32  ad_i2c1_wcompl;        // completion pointer, writes
//static char ad_sdat0[27];          // used to build up Date string that is presented to user
//static u8   ad_loop_state;         // state machine for the showtime_loop
//static u16  ad_loop_timer;         // timer counter for the showtime_loop


static I2CCMDS ad_registers[] =
{
	{ 0x28, 0x01, 0x00, 0x02, I2C_CMDTYPE_RW },        // AD: read the registers
    { 0x28, 0x01, 0x00, 0x02, I2C_CMDTYPE_RW },        // AD: read the registers
    { 0xff, 0xff, 0xff, 0xff, 0xff           },        // Terminate the List
};


//
// { SlaveAddr, CmdReg, CmdReg2, NumBytes, Type }
//
static I2CCMDS ad_regs_w1[2] =
{
    { 0x28, 0x00, 0x00, 0x01, I2C_CMDTYPE_WRITEONLY },        // AD: write 1 register, value is in ad_wdata1
    { 0xff, 0xff, 0xff, 0xff, 0xff                  },        // Terminate the List
};

static u8  ad_wdata1[1] = { 0x01 };

extern void swap( u16 *sptr );



void AD_Init( void )
{
	/* u32 chan = 15;

    GPIO_ResetBits( GPIOB, MUX1_EN_Pin );
    GPIO_ResetBits( GPIOB, MUX2_EN_Pin );
    GPIO_ResetBits( GPIOB, MUXBIT_0_Pin | MUXBIT_1_Pin | MUXBIT_2_Pin );

    (chan & 1) ? GPIO_ResetBits( GPIOB, MUXBIT_0_Pin) : GPIO_SetBits( GPIOB, MUXBIT_0_Pin);
    (chan & 2) ? GPIO_ResetBits( GPIOB, MUXBIT_1_Pin) : GPIO_SetBits( GPIOB, MUXBIT_1_Pin);
    (chan & 4) ? GPIO_ResetBits( GPIOB, MUXBIT_2_Pin) : GPIO_SetBits( GPIOB, MUXBIT_2_Pin);
    (chan & 8) ? GPIO_SetBits  ( GPIOB,  MUX2_EN_Pin) : GPIO_SetBits( GPIOB,  MUX1_EN_Pin); */
}


void AD_Get( void )
{
	memset( ad_rbuf, 0, 10 );
    I2C_master_SendCmd( ad_rbuf, ad_wdata1, &ad_i2c1_rcompl, ad_registers );
}






/*
void swap( u16 *sptr )
{
	u8  *bptr = (u8 *)sptr;
    u8  tmp   = bptr[0];

    bptr[0] = bptr[1];
    bptr[1] = tmp;
}
*/





//
//   OSC=64Mhz, I2C=400kHz:  time=371.5us
//   OSC=64Mhz, I2C=100kHz:  time=1.094ms
//
//   OSC=32Mhz, I2C=400kHz:  time=544.20us
//   OSC=32Mhz, I2C=100kHz:  time=1.209ms
//
u8 AD_Show( void )
{
    u8  retv  = ad_i2c1_rcompl;
    u16 *sptr = (u16 *)&ad_rbuf[0];

    if( retv != I2C_COMPLETION_BUSY )
    {
        if( retv == I2C_COMPLETION_TIMEOUT )
        {
            UD_PrintSTR( "Timeout AD!!" );
            UD_Print8  ( "    buf[0]: ", ad_rbuf[0] );
        }
        else                                               // ELSE completed OK
        {
            swap( sptr); UD_Print16( "[0]: ", *sptr++ );
            swap( sptr); UD_Print16( "[1]: ", *sptr++ );
            swap( sptr); UD_Print16( "[2]: ", *sptr++ );
            swap( sptr); UD_Print16( "[3]: ", *sptr++ );
            swap( sptr); UD_Print16( "[4]: ", *sptr   );
        }
    }
    return retv;
}



void AD_Set30hz( void )
{
    I2C_master_SendCmd( 0, ad_wdata1, &ad_i2c1_wcompl, ad_regs_w1);
}

u8 AD_Set30hzComplete( void )
{
    u8  retv = ad_i2c1_wcompl;

    if( retv != I2C_COMPLETION_BUSY )
    {
        if( retv == I2C_COMPLETION_TIMEOUT )
        {
            UD_PrintSTR( "Timeout AD!!" );
        }
        else
        {
            UD_PrintSTR( "Set Done" );
        }
    }
    return retv;
}



/*

void xRTC_ShowTime_Loop( void )
{   
    switch( rtc_loop_state )
    {
    case STATE_GET_TIME:
        
        rtc_loop_timer = GetSysTick();
        xRTC_GetTime();
        rtc_loop_state = STATE_SHOW_TIME;
        break;
        
    case STATE_SHOW_TIME:

        if( xRTC_ShowTime() )
        {
            UD_PrintSTR( "\n\r" );
            rtc_loop_state = STATE_DO_WAIT;
        }
        break;
        
    case STATE_DO_WAIT:
        
        if( GetSysDelta(rtc_loop_timer) >= 500 )
            rtc_loop_state = STATE_GET_TIME;
        break;
    }
} */
