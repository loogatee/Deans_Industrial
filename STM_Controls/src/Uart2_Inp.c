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
#include "Utils.h"





#define SERI_STATE_GETCHARS    0
#define SERI_STATE_WAITDONE    1

#define SERI_MAX_CHARS         21



typedef struct
{
    u32    seri_state_machine;              //   state of Serial machine, Input
    bool   seri_CmdDone;                    //   signal for CMD completion
    u32    seri_cnt;                        //   input: count of chars in buffer
    u8     seri_ch;                         //   input: character just received
    char   seri_dat[SERI_MAX_CHARS];        //   input: data buffer
} GLOBALS_t;



static GLOBALS_t   Globals;

static const char   *sCRLF   = "\n\r";     // carriage-return, line-feed
static const char   *sPROMPT = ">> ";      // prompt string



void UDInp_Init( void )
{
    Globals.seri_cnt           = 0;
    Globals.seri_state_machine = SERI_STATE_GETCHARS;
}


void UDInp_Process( void )
{
    GLOBALS_t  *G = &Globals;

    switch( G->seri_state_machine )
    {
    case SERI_STATE_GETCHARS:

        if( !(USART1->SR & USART_FLAG_RXNE) ) { return; }                             // RXNE=1 when a byte is available
        G->seri_ch = (USART1->DR & 0xff);

        //if( !(USART2->SR & USART_FLAG_RXNE) ) { return; }                             // RXNE=1 when a byte is available
        //G->seri_ch = (USART2->DR & 0xff);

        if( G->seri_ch == ASCII_CARRIAGERETURN || G->seri_ch == ASCII_LINEFEED )
        {
            UD_PrintSTR(sCRLF);
            G->seri_dat[G->seri_cnt] = 0;

            if( G->seri_cnt > 0 )
            {
                G->seri_cnt           = 0;
                G->seri_CmdDone       = FALSE;
                G->seri_state_machine = SERI_STATE_WAITDONE;
                CMDS_SetInputStr(G->seri_dat);
            }
            else
            {
                UD_PrintSTR(sPROMPT);
            }
        }
        else if( G->seri_ch == ASCII_BACKSPACE || G->seri_ch == ASCII_DELETE )
        {
            if( G->seri_cnt > 0 )
            {
                UD_PrintSTR((const char *)"\b \b");
                --G->seri_cnt;
            }
        }
        else if(( G->seri_ch >= ASCII_SPACE ) && ( G->seri_ch <= ASCII_TILDE ))
        {
            UD_PrintCH(G->seri_ch);
            G->seri_dat[G->seri_cnt] = G->seri_ch;
            if( ++G->seri_cnt >= SERI_MAX_CHARS ) { --G->seri_cnt; }
        }

        break;

    case SERI_STATE_WAITDONE:

        if( G->seri_CmdDone == TRUE )
        {
            UD_PrintSTR(sCRLF);
            UD_PrintSTR(sPROMPT);
            G->seri_state_machine = SERI_STATE_GETCHARS;
        }
        break;
    }
}


void UDInp_SignalCmdDone( void )
{
    Globals.seri_CmdDone = TRUE;
}












