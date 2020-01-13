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
#include <stdio.h>
#include "task.h"
#include "semphr.h"
#include "proj_common.h"
#include "Uart.h"
#include "Cmds.h"
#include "string.h"
#include "gpio_CDC3.h"
#include "timer.h"
#include "i2c.h"
#include "AtoDs.h"
#include "DataBuffer.h"
#include "monitor_host.h"
#include "PacketRecvThread.h"
#include "PacketSendThread.h"
#include "PacketDrivers.h"




typedef struct
{
    uint32_t           Ntime;
    TaskHandle_t       thand1;
    TaskHandle_t       thand2;
    SemaphoreHandle_t  jSem;
    u32                ticker;
} GLOBALS_t;



static GLOBALS_t   Globals;



static void      init_hw(void);
static void      TestThread(void const * argument);



//static void      dbgShowADchan( void );
//static u32       dbgchan;




GLOBALLY_VISIBLE int main(void)
{
    Globals.ticker = 0;
    Globals.Ntime  = 0;
  //dbgchan        = 15;

    UD_Init();
    UDInp_Init();
    CMDS_Init();
    init_hw();
    init_timer();
    I2C_1_master_Init();
    AtoD_Init();
    DataBuffer_Init();
    PacketSendDriver_Init();
    PacketRecvDriver_Init();
    MonitorHost_Init();

    CMDS_DisplayVersion();

    Globals.jSem = xSemaphoreCreateBinary();

    xTaskCreate((TaskFunction_t)TestThread,      (const char * const)"TestThread",  128, 0, 1, &Globals.thand2);

    PacketRecv_TaskCreate();
    PacketSend_TaskCreate();

    vTaskStartScheduler();
}

//
//   *** Called from the IDLE task
//
GLOBALLY_VISIBLE void vApplicationIdleHook( void )
{
    GPIO_ResetBits(GPIOE, DO1_Pin);

    UD_Process();
    UDInp_Process();
    CMDS_Process();
    PacketSendDriver_Process();
    PacketRecvDriver_Process();
    MonitorHost_Process();

    //if( GetSysDelta(Globals.Ntime) >= 2000 )                // number is in miilliseconds
    //{
    //  //GPIO_ToggleBits(GPIOE, DO1_Pin);                    //     Will do something every 100ms
    //    Globals.Ntime = GetSysTick();                       //     re-init the counter
    //    dbgShowADchan();
    //}

    GPIO_SetBits(GPIOE, DO1_Pin);
}

//
//   *** Called from the System Ticker
//
GLOBALLY_VISIBLE void vApplicationTickHook( void )
{
    if( ++Globals.ticker >= 500 )
    {
        Globals.ticker = 0;
        GPIO_ToggleBits(GPIOE, HB_LED_Pin);    //   Toggles the User Led per Delta interval
    }

    Increment_SysTicks();
    I2C_1_master_Process();
}



static void init_gpios(void)
{
    GPIO_InitTypeDef gpioX;
    GPIO_InitTypeDef UsartX_gpio;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,  ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,  ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,  ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,  ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,   ENABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA1,   ENABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA1,   DISABLE);



    GPIO_StructInit(&UsartX_gpio);
      UsartX_gpio.GPIO_Pin   = CTRL_P_TX_Pin | CTRL_P_RX_Pin | TX_CONSOLE_Pin | RX_CONSOLE_Pin;            // CONSOLE: A9-TX, A10-RX = USART1      CTRL_P: A2-TX, A3-RX = USART2
      UsartX_gpio.GPIO_Mode  = GPIO_Mode_AF;
      UsartX_gpio.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOA, &UsartX_gpio);


    GPIO_StructInit(&gpioX);
      gpioX.GPIO_Pin   = DO1_Pin | HB_LED_Pin;                                // GPIOE:  Pin9=DigOut1.  Pin15 = D21 = Heartbeat LED for Control proc
      gpioX.GPIO_Mode  = GPIO_Mode_OUT;
      gpioX.GPIO_PuPd  = GPIO_PuPd_UP;
      gpioX.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &gpioX);

    //#define SVALVE_ON_Pin        GPIO_Pin_6
    //#define SVALVE_ON_GPIO_Port  GPIOA
    //#define WPUMP_ON_Pin         GPIO_Pin_7
    //#define WPUMP_ON_GPIO_Port   GPIOA
    //#define FAN1_ON_Pin          GPIO_Pin_12
    //#define FAN1_ON_GPIO_Port    GPIOA
    GPIO_StructInit(&gpioX);
      gpioX.GPIO_Pin   = CTRL_P_RTS_Pin | FAN2_ON_Pin | FAN1_ON_Pin | SVALVE_ON_Pin | WPUMP_ON_Pin;
      gpioX.GPIO_Mode  = GPIO_Mode_OUT;                                     // GPIOA: , P1=CTRL_P_RTS,  Pin11=D7=FAN2,  Pin12=D6=FAN1
      gpioX.GPIO_PuPd  = GPIO_PuPd_UP;                                      //                          Pin6=D4=SVALVE, Pin7=D5=WPUMP
      gpioX.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioX);

    GPIO_StructInit(&gpioX);
      gpioX.GPIO_Pin   = CTRL_to_COMMS_Pin;                                   // GPIOD: , Pin6 = CTRL_to_COMMS_Pin
      gpioX.GPIO_Mode  = GPIO_Mode_OUT;
      gpioX.GPIO_PuPd  = GPIO_PuPd_UP;
      gpioX.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &gpioX);

    GPIO_StructInit(&gpioX);
      gpioX.GPIO_Pin   = COMMS_to_CTRL_Pin;                                   // GPIOB: Pin5 = COMMS_to_CTRL_Pin
      gpioX.GPIO_Mode  = GPIO_Mode_IN;
      gpioX.GPIO_PuPd  = GPIO_PuPd_NOPULL;
      gpioX.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpioX);

    GPIO_StructInit(&gpioX);
      gpioX.GPIO_Pin   = MUXBIT_0_Pin | MUXBIT_1_Pin | MUXBIT_2_Pin | MUX1_EN_Pin | MUX2_EN_Pin;
      gpioX.GPIO_Mode  = GPIO_Mode_OUT;
      gpioX.GPIO_PuPd  = GPIO_PuPd_UP;
      gpioX.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpioX);



    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9,  GPIO_AF_USART1);      // Pin9  = Usart1 TX
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);      // Pin10 = Usart1 RX

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2,  GPIO_AF_USART2);      // Pin2 = Usart2 TX
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3,  GPIO_AF_USART2);      // Pin3 = Usart2 RX
}


static void init_usarts()
{
    USART_InitTypeDef U1;
    USART_InitTypeDef U2;


    USART_StructInit( &U1 );
      U1.USART_BaudRate            = 9600;
      U1.USART_WordLength          = USART_WordLength_8b;
      U1.USART_StopBits            = USART_StopBits_1;
      U1.USART_Parity              = USART_Parity_No;
      U1.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
      U1.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init( USART1, &U1    );
    USART_Cmd ( USART1, ENABLE );

    USART_StructInit( &U2 );
      U2.USART_BaudRate            = 9600;
      U2.USART_WordLength          = USART_WordLength_8b;
      U2.USART_StopBits            = USART_StopBits_1;
      U2.USART_Parity              = USART_Parity_No;
      U2.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
      U2.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init( USART2, &U2    );
    USART_Cmd ( USART2, ENABLE );
}


void init_gpioI2C(void)
{
    GPIO_InitTypeDef I2CX_gpio;
    I2C_InitTypeDef  I1;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,   ENABLE);

    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,   ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,   DISABLE);

    GPIO_StructInit(&I2CX_gpio);
      I2CX_gpio.GPIO_Pin    = GPIO_Pin_6 | GPIO_Pin_7;                         // I2C1: B6=SCL, B7=SDA
      I2CX_gpio.GPIO_Mode   = GPIO_Mode_AF;
      I2CX_gpio.GPIO_Speed  = GPIO_Speed_100MHz;
      I2CX_gpio.GPIO_OType  = GPIO_OType_OD;
    GPIO_Init(GPIOB, &I2CX_gpio);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6,  GPIO_AF_I2C1);        // Pin6 = I2C1 SCL
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7,  GPIO_AF_I2C1);        // Pin7 = I2C1 SDA

    I2C_StructInit( &I1 );
      I1.I2C_ClockSpeed          = 100000;
      I1.I2C_Mode                = I2C_Mode_I2C;
      I1.I2C_DutyCycle           = I2C_DutyCycle_2;        // only matters in fast mode
      I1.I2C_OwnAddress1         = 0;                      // only matters in slave mode: we are the master
      I1.I2C_Ack                 = I2C_Ack_Enable;
      I1.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init( I2C1, &I1    );
    I2C_Cmd ( I2C1, ENABLE );
}




static void init_hw()
{
    RCC_ClocksTypeDef  rclocks;

    SystemCoreClockUpdate();
    RCC_GetClocksFreq(&rclocks);

    FLASH->ACR |= FLASH_ACR_ICEN;      // Flash Instruction Cache Enable
    FLASH->ACR |= FLASH_ACR_DCEN;      // Flash Data Cache Enable
    FLASH->ACR |= FLASH_ACR_PRFTEN;    // Flash Pre-Fetch Buffer Enable

    init_gpios();
    init_usarts();
    init_gpioI2C();
}




static void TestThread(void const * argument)
{
    while(1)
    {
        xSemaphoreTake( Globals.jSem, portMAX_DELAY);
        GPIO_ToggleBits(GPIOA, FAN2_ON_Pin);
    }
}

GLOBALLY_VISIBLE void jSem_give( void )
{
    xSemaphoreGive( Globals.jSem );
}




/*

static void show_vals( u16 numchans, float *fvals )
{
    int  i,A;

    for( i=0; i < numchans; ++i )
        printf("%d,",(int)(fvals[i]*100.0));

    A = (int)GPIO_ReadInputDataBit(GPIOB, COMMS_to_CTRL_Pin);
    printf("%d",A);

    UD_PrintSTR("\n\r");
}

static void dbgShowADchan( void )
{
    u16   rawval,totchans;
    float cooked, vals[16];
    int   thechan;

    totchans = 0;
    thechan  = -1;

    while(1)
    {
        do
        {
            if( ++thechan == NUM_AD_CHANS )
            {
                show_vals(totchans,vals);
                return;
            }
        } while (!(AtoD_Get_ConfiguredChannels() & (1 << thechan)));

        AtoD_Get_Reading( thechan, &rawval, &cooked );
        vals[totchans++] = cooked;
    }
}

*/



















