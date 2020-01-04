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

#ifndef INCLUDE_ATODS_H_
#define INCLUDE_ATODS_H_


#define NUM_AD_CHANS              16

// SuperH = CSV[SUCT_T].Value - ComputeSaturationTemp(CSV[SUCT_P].Value);
#define   CONTROLSIGCOUNT 9

#define  SYS_P          0
#define  SUCT_P         1
#define  SUCT_T         2
#define  CAB_T          3
#define  WETSUCT_T      4
#define  CONDLIQ_T      5
#define  WATER_T        6
#define  SYS_POW        7
#define  COM_POW        8

#define  SYS_P_N        "I0.SYp"
#define  SUCT_P_N       "I0.CSp"
#define  SUCT_T_N       "I0.CSt"
#define  CAB_T_N        "I0.CBt"
#define  WETSUCT_T_N    "I0.WSt"
#define  CONDLIQ_T_N    "I0.CLt"
#define  WATER_T_N      "I0.TWt"
#define  SYS_POW_N      "I0.SYw"
#define  COM_POW_N      "I0.CPw"




typedef struct
{
	float Value;
} AD_SignalDef_t;




void  AtoD_Init( void );
u16   AtoD_Get_ConfiguredChannels( void );
void  AtoD_Set_value( u8 channel, u16 raw_val );
void  AtoD_Get_Reading( u8 channel, u16 *raw, float *cooked );
void  AtoD_Set_NewConfig( u8 Len, u8 *Dptr );
void  AtoD_GetAllReadings( u32 *Dptr );

void  AtoD_dbgShowStuff( void );

#endif /* INCLUDE_ATODS_H_ */
