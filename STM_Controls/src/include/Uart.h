#ifndef _UART_H_
#define _UART_H_

/**
 * Copyright 2019 john reed
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


#define USE_FTDI_PORT




#define SERO_TYPE_ONECHAR    0x01
#define SERO_TYPE_STR        0x02
#define SERO_TYPE_32         0x03
#define SERO_TYPE_32N        0x04
#define SERO_TYPE_8          0x05
#define SERO_TYPE_8N         0x06
#define SERO_TYPE_16         0x07
#define SERO_TYPE_16N        0x08



// Uart2 output
void UD_Init    (void);
void UD_Process (void);
void UD_PrintCH (char ch);
void UD_PrintSTR(const char *pstr);
void UD_Print32 (const char *pstr,  uint32_t val);
void UD_Print32N(const char *pstr,  uint32_t val);
void UD_Print8  (const char *pstr,  uint8_t  val);
void UD_Print8N (const char *pstr,  uint8_t  val);
void UD_Print16 (const char *pstr,  uint16_t val);
void UD_Print16N(const char *pstr,  uint16_t val);
void UD_Send(uint32_t otype, char *sptr, uint32_t *completionptr, uint32_t aval);

// Uart2 input
void UDInp_Init(void);
void UDInp_Process(void);
void UDInp_SignalCmdDone(void);




#endif
