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
#include "PacketDrivers.h"
#include "PacketSendThread.h"

#define  SAFE_MEM_ADDR           (&Globals.cmds_safe)


#define  CMDSM_WAITFORLINE       0
#define  CMDSM_MEMDUMP           1
#define  CMDSM_DMA_DONE          2
#define  CMDSM_AD_SETDONE        3

#define  DO_INIT                 0
#define  DO_PROCESS              1


typedef struct
{
    bool   cmds_input_ready;
    char  *cmds_InpPtr;
    u32    cmds_state_machine;
    u32    cmds_completion;
    u32    cmds_word1;
    u32    cmds_count1;
    u8     cmds_TA[6];
    u32    cmds_safe;
 volatile u32    cmds_xtest;
} GLOBALS_t;


static const char *gWeeks[7] =
{
   (const char *)"Mon  ",     // 1
   (const char *)"Tue  ",     // 2
   (const char *)"Wed  ",
   (const char *)"Thu  ",
   (const char *)"Fri  ",
   (const char *)"Sat  ",     // 6
   (const char *)"Sun  "      // 7
};




static GLOBALS_t  Globals;
//static u32        dbgcount;

static u8   dbgbuffer[12];

static bool cmds_A ( u32 state );
//static bool cmds_Z ( u32 state );
static bool cmds_R ( void );
static bool cmds_T ( void );
static bool cmds_B ( void );
static bool cmds_MD( u32 state );
static bool cmds_SC( void );
static bool cmds_rtc( void );
static bool cmds_ST( void );
static bool cmds_ch( void );


extern void jSem_give(void);
extern void show_packetrecv_state( void );
extern u32  MonitorHost_GetState( void );

void CMDS_Init(void)
{
    Globals.cmds_input_ready   = FALSE;
    Globals.cmds_state_machine = CMDSM_WAITFORLINE;
    Globals.cmds_xtest         = 0x11223398;

    *((uint32_t *)SAFE_MEM_ADDR) = 0xBC00BC99;
}


void CMDS_Process(void)
{
    GLOBALS_t  *G           = &Globals;
    bool        signal_done = TRUE;
    char       *S           = G->cmds_InpPtr;
    
    switch( G->cmds_state_machine )
    {
    case CMDSM_WAITFORLINE:
        
        if( G->cmds_input_ready == FALSE ) { return; }
        G->cmds_input_ready = FALSE;
        
        if     ( S[0] == 'a' )                                signal_done = cmds_A( DO_INIT );
        else if( S[0] == 'b' )                                signal_done = cmds_B();
        else if( S[0] == 'c' && S[1] == 'h')                  signal_done = cmds_ch();
        else if( S[0] == 'm' && S[1] == 'd')                  signal_done = cmds_MD( DO_INIT );
        else if( S[0] == 'r' && S[1] == 't' && S[2] == 'c')   signal_done = cmds_rtc();
        else if( S[0] == 'r' )                                signal_done = cmds_R();
        else if( S[0] == 's' && S[1] == 'c')                  signal_done = cmds_SC();
        else if( S[0] == 's' && S[1] == 't')                  signal_done = cmds_ST();
        else if( S[0] == 't' )                                signal_done = cmds_T();
        else if( S[0] == 'v' )                                signal_done = CMDS_DisplayVersion();
        else if( S[0] == 'z' )                                /*signal_done = cmds_Z( DO_INIT ) */;
        break;
        
    case CMDSM_MEMDUMP:      signal_done = cmds_MD( DO_PROCESS );    break;
    case CMDSM_DMA_DONE:     signal_done = cmds_A ( DO_PROCESS );    break;
  //case CMDSM_AD_SETDONE:   signal_done = cmds_Z ( DO_PROCESS );    break;
        
    }

    if( signal_done == TRUE ) { UDInp_SignalCmdDone(); }
}








bool CMDS_DisplayVersion(void)
{
    UD_PrintSTR("\r\n");
    UD_Print8N (VERSION_STR, VERSION_MINOR);
    UD_PrintSTR(", ");
    UD_PrintSTR(VERSION_DATE);
    
    return TRUE;
}


void CMDS_SetInputStr(char *StrInp)
{
    Globals.cmds_InpPtr      = StrInp;
    Globals.cmds_input_ready = TRUE;
}


static bool cmds_ch( void )
{
    u32         chan = 0;
    GLOBALS_t  *G    = &Globals;


    if( strlen(G->cmds_InpPtr) > 3 )
    {
        chan = HtoI(&G->cmds_InpPtr[3]) & 0x0000000F;
    }

    UD_Print32( "channel: ", chan );

    GPIO_ResetBits( GPIOB, MUX1_EN_Pin );
    GPIO_ResetBits( GPIOB, MUX2_EN_Pin );
    GPIO_ResetBits( GPIOB, MUXBIT_0_Pin | MUXBIT_1_Pin | MUXBIT_2_Pin );

    (chan & 1) ? GPIO_ResetBits( GPIOB, MUXBIT_0_Pin) : GPIO_SetBits( GPIOB, MUXBIT_0_Pin);
    (chan & 2) ? GPIO_ResetBits( GPIOB, MUXBIT_1_Pin) : GPIO_SetBits( GPIOB, MUXBIT_1_Pin);
    (chan & 4) ? GPIO_ResetBits( GPIOB, MUXBIT_2_Pin) : GPIO_SetBits( GPIOB, MUXBIT_2_Pin);
    (chan & 8) ? GPIO_SetBits  ( GPIOB,  MUX2_EN_Pin) : GPIO_SetBits( GPIOB,  MUX1_EN_Pin);

    return TRUE;
}


static bool cmds_R( void )
{
    u32  tmpw;
    char ch1 = Globals.cmds_InpPtr[1];
    
    if( ch1 == 'p' )
    {
        UD_Print32( "PWR->CR  ", (u32)PWR->CR );
        UD_Print32( "PWR->CSR ", (u32)PWR->CSR );
    }
    else if( ch1 == 'm' )
    {
        tmpw = HtoI(&Globals.cmds_InpPtr[3]) & 0xFFFFFFFC;                              // bits 0 and 1 forced to 0
        UD_Print32N( "0x", tmpw );
        UD_Print32( ": ", (u32)*((u32 *)tmpw) );
    }
    
    return TRUE;
}


static bool cmds_T( void )
{
    int  tmpI;
    u32  tmp32;
    char ch1 = Globals.cmds_InpPtr[1];
    
    if( ch1 == '1' )
    {
        tmpI = AtoI("452");
        UD_Print32( "452: 0x", (u32)tmpI );
        
        tmpI = AtoI("-4392");
        if( tmpI == -4392 )
            UD_PrintSTR( "-4392 Good\r\n" );
        else
            UD_PrintSTR( "Conversion did not yield -4392\r\n" );
    }
    else if( ch1 == '2' )
    {
        tmp32 = (u32)*((u32 *)SAFE_MEM_ADDR);
        UD_Print32( "*SAFE_MEM_ADDR = ", tmp32 );
    }
    else if( ch1 == '3' )
    {
        tmp32 = 1234567;
        printf("testing, tmp32 = %d\n\r",(int)tmp32);
        printf("This is Only Just a Test %s   %d    0x%x\n\r","hello world",(int)tmp32,0x1234567);
    }
    
    return TRUE;
}




//
//   From _rtc.c:
//
//   To enable access to the RTC Domain and RTC registers, proceed as follows:
//       (+) Enable the Power Controller (PWR) APB1 interface clock using the
//              RCC_APB1PeriphClockCmd() function.
//       (+) Enable access to RTC domain using the PWR_BackupAccessCmd() function.
//       (+) Select the RTC clock source using the RCC_RTCCLKConfig() function.
//       (+) Enable RTC Clock using the RCC_RTCCLKCmd() function.
//
static void init_rtc_stuff(void)
{
    RTC_InitTypeDef   Irtc;

    RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR,  ENABLE );
    PWR_BackupAccessCmd( ENABLE );

    RCC_LSICmd( DISABLE );
    RCC_LSEConfig( RCC_LSE_ON );

    RCC_RTCCLKConfig( RCC_RTCCLKSource_LSE );
    RCC_RTCCLKCmd( ENABLE );

    Irtc.RTC_HourFormat   = RTC_HourFormat_24;
    Irtc.RTC_AsynchPrediv = 127;
    Irtc.RTC_SynchPrediv  = 255;
    RTC_Init( &Irtc );
}



static bool cmds_B( void )
{
    char ch1 = Globals.cmds_InpPtr[1];

    if( ch1 == '1' )
    {
        jSem_give();
    }
    else if( ch1 == '2' )
    {
        AtoD_dbgShowStuff();
    }
    else if( ch1 == '3' )
    {
        GPIO_ResetBits(GPIOA, CTRL_P_RTS_Pin);
        UD_PrintSTR( "yRTS_Pin=0\r\n" );
    }
    else if( ch1 == '4' )
    {
        GPIO_SetBits(GPIOA, CTRL_P_RTS_Pin);
        UD_PrintSTR( "yRTS_Pin=1\r\n" );
    }
    else if( ch1 == '5' )
    {
        GPIO_ResetBits(GPIOD, GPIO_Pin_6);
        UD_PrintSTR( "xctrl_to_comms=0\r\n" );
    }
    else if( ch1 == '6' )
    {
        GPIO_SetBits(GPIOD, GPIO_Pin_6);
        UD_PrintSTR( "xctrl_to_comms=1\r\n" );
    }
    else if( ch1 == '7' )
    {
        UD_Print32( "MonitorHost state = ",  MonitorHost_GetState());
    }
    else if( ch1 == '8' )
    {
        PacketSend_ADconfig( 2, dbgbuffer );
    }

    return TRUE;
}





static bool cmds_MD( u32 state )
{
    u32  i;
    bool retv = FALSE;
    GLOBALS_t  *G = &Globals;
    
    switch( state )
    {
    case DO_INIT:

        if( strlen(G->cmds_InpPtr) > 2 ) { G->cmds_word1 = HtoI(&G->cmds_InpPtr[3]) & 0xFFFFFFFC; }
        G->cmds_count1        = 0;
        G->cmds_state_machine = CMDSM_MEMDUMP;
        G->cmds_completion    = 1;
        FALL_THRU;
        
    case DO_PROCESS:
    
        if( G->cmds_completion == 1 )
        {
            UD_Print32N( "0x", G->cmds_word1 );
            UD_Print8N( ": ", (u8)*((u8 *)G->cmds_word1++) );

            for( i=0; i < 15; i++ )
            {
                UD_Print8N( " ", (u8)*((u8 *)G->cmds_word1++) );
            }

            UD_Send( SERO_TYPE_STR, (char *)"\r\n", &G->cmds_completion, 0 );

            if( ++G->cmds_count1 == 4 )
            {
                G->cmds_state_machine = CMDSM_WAITFORLINE;
                ItoH( G->cmds_word1, &G->cmds_InpPtr[2] );
                retv = TRUE;
            }
        }
        break;
    }
    
    return retv;
}




static bool cmds_A( u32 state )
{
    char ch1 = Globals.cmds_InpPtr[1];
    u32  val;
    //GLOBALS_t  *G = &Globals;

if( state == DO_INIT )
{
    if( ch1 == '1' )
    {
        val = DMA1->HISR;           UD_Print32("HISR: ",val);
        val = DMA1_Stream6->CR;     UD_Print32("6_CR: ",val);
        val = DMA1_Stream6->FCR;    UD_Print32(" FCR: ",val);
        val = DMA1_Stream6->M0AR;   UD_Print32("M0AR: ",val);
        val = DMA1_Stream6->M1AR;   UD_Print32("M1AR: ",val);
        val = DMA1_Stream6->NDTR;   UD_Print32("NDTR: ",val);
        val = DMA1_Stream6->PAR;    UD_Print32("PAR:  ",val);
    }
    else if( ch1 == '2' )
    {
        val = DMA1->HISR;           UD_Print32("HISR: ",val);
        val = DMA1_Stream5->CR;     UD_Print32("6_CR: ",val);
        val = DMA1_Stream5->FCR;    UD_Print32(" FCR: ",val);
        val = DMA1_Stream5->M0AR;   UD_Print32("M0AR: ",val);
        val = DMA1_Stream5->M1AR;   UD_Print32("M1AR: ",val);
        val = DMA1_Stream5->NDTR;   UD_Print32("NDTR: ",val);
        val = DMA1_Stream5->PAR;    UD_Print32("PAR:  ",val);;
        show_packetrecv_state();
    }
    else if( ch1 == '3' )
    {
        /* DMA_Cmd( DMA1_Stream6, ENABLE );
        USART_DMACmd( USART2, USART_DMAReq_Tx, ENABLE );
        G->cmds_state_machine = CMDSM_DMA_DONE;
        dbgcount = 0;
        return FALSE; */
    }
    else if( ch1 == '4' )
    {
        //DMA_ClearFlag( DMA1_Stream6, DMA_FLAG_TCIF6 );
        //USART2->SR = (uint16_t)~USART_FLAG_TC;
    }
    else if( ch1 == '5' )
    {
        ;
    }
    else if( ch1 == '6' )
    {
        ;
    }
}
else
{
      //if( !(USART2->SR & USART_FLAG_TC) )  { return FALSE; }
      //if( !(DMA1->HISR & DMA_FLAG_TCIF6) ) { return FALSE; }

      //DMA_ClearFlag( DMA1_Stream6, DMA_FLAG_TCIF6 | DMA_FLAG_HTIF6 | DMA_FLAG_FEIF6 );
      //USART2->SR = (uint16_t)~USART_FLAG_TC;
      //G->cmds_state_machine = CMDSM_WAITFORLINE;


      //DMA1_Stream6->M0AR = (u32)tstdata;
      //DMA1_Stream6->NDTR = 54;

      //dmaStruct.DMA_Memory0BaseAddr    = (u32)tstdata;
      //dmaStruct.DMA_BufferSize         = 54;
      //DMA_Init( DMA1_Stream6, &dmaStruct );

      //UD_Print32("dbgcount:  ",(u32)dbgcount);
      FALL_THRU;
}

    return TRUE;
}


/*
static bool cmds_Z( u32 state )
{
    bool retv = FALSE;
    GLOBALS_t  *G = &Globals;
    
    switch( state )
    {
    case DO_INIT:
        
        AD_Set30hz();
        G->cmds_state_machine = CMDSM_AD_SETDONE;
        break;
        
    case DO_PROCESS:

        if( AD_Set30hzComplete() != I2C_COMPLETION_BUSY )
        {
            G->cmds_state_machine = CMDSM_WAITFORLINE;
            retv = TRUE;
        }
        break;
    }
    
    return retv;
}
*/


// Set Time
//
//  012345678901234567890
//  st 38 12 03 31 01 17
//
static bool cmds_ST( void )
{
    u8                i,k;
    RTC_TimeTypeDef   Trtc;
    RTC_DateTypeDef   Drtc;
    GLOBALS_t  *G = &Globals;
    
    if( strlen(G->cmds_InpPtr) == 2 )
    {
        UD_PrintSTR( "st mins hrs wkday day mon yr\r\n" );
    }
    else
    {
        for( i=3,k=0; i < 20; i += 3 )
        {
            G->cmds_InpPtr[i+2] = 0;
            G->cmds_TA[k++]     = (u8)HtoU16( &G->cmds_InpPtr[i] );
        }

        Trtc.RTC_H12     = 0;
        Trtc.RTC_Hours   = G->cmds_TA[1];
        Trtc.RTC_Minutes = G->cmds_TA[0];
        Trtc.RTC_Seconds = 0;

        Drtc.RTC_Date    = G->cmds_TA[3];
        Drtc.RTC_Month   = G->cmds_TA[4];
        Drtc.RTC_WeekDay = G->cmds_TA[2];
        Drtc.RTC_Year    = G->cmds_TA[5];


        init_rtc_stuff();

        RTC_WriteProtectionCmd(DISABLE);
        RTC_SetTime(RTC_Format_BCD, &Trtc);
        RTC_SetDate(RTC_Format_BCD, &Drtc);
        RTC_WriteProtectionCmd(ENABLE);
    }
    
    return TRUE;
}



// show clock
//
//    debug:  shows some clock registers
//
static bool cmds_SC( void )
{
    uint32_t tmp;
    RCC_ClocksTypeDef  rclocks;

    tmp = RCC->CFGR & RCC_CFGR_SWS;
    if( tmp == 0 )
        UD_PrintSTR("HSI\n\r");
    else if( tmp == 4 )
        UD_PrintSTR("HSE\n\r");
    else
        UD_PrintSTR("PLL\n\r");

    SystemCoreClockUpdate();
    RCC_GetClocksFreq(&rclocks);

    UD_Print32( "SYSCLK: ", rclocks.SYSCLK_Frequency );
    UD_Print32( "HCLK:   ", rclocks.HCLK_Frequency   );
    UD_Print32( "PCLK1:  ", rclocks.PCLK1_Frequency  );
    UD_Print32( "PCLK2:  ", rclocks.PCLK2_Frequency  );

    UD_Print32( "RCC->APB1ENR:  ", RCC->APB1ENR  );

    return TRUE;
}




static bool cmds_rtc( void )
{
    RTC_TimeTypeDef   Trtc;
    RTC_DateTypeDef   Drtc;
    u32               dow;
    char              B1[3],B2[3],B3[3];
    char              ch3 = Globals.cmds_InpPtr[3];

    if( ch3 == 'x' )
    {
        UD_Print32("RTC_CR:     ", RTC->CR);
        UD_Print32("RTC_ISR:    ", RTC->ISR);
        UD_Print32("RTC_PRER:   ", RTC->PRER);
        UD_Print32("RTC_WUTR:   ", RTC->WUTR);
        UD_Print32("RTC_CALIBR: ", RTC->CALIBR);
        UD_Print32("RTC_WPR:    ", RTC->WPR);
    }
    else if( ch3 == 0 )
    {
        RTC_GetTime(RTC_Format_BCD, &Trtc);
        RTC_GetDate(RTC_Format_BCD, &Drtc);

        dow = (Drtc.RTC_WeekDay - 1);     // Monday is 1, Sunday is 7
        if( dow > 7 ) { dow = 0; }

        BtoH( Trtc.RTC_Hours,   B1 );     // BtoH gives 2 digits.  tiny printf doesn't work with %02x
        BtoH( Trtc.RTC_Minutes, B2 );
        BtoH( Trtc.RTC_Seconds, B3 );

        printf("%s%x/%x/20%x   %s:%s:%s",gWeeks[dow],Drtc.RTC_Month,Drtc.RTC_Date,Drtc.RTC_Year,B1,B2,B3);
    }

    return TRUE;
}
























