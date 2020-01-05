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
#include <stdio.h>
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
#include "i2c.h"


/*
 *    Notes:
 *
 *       Consider a packet for "Coefficient override: done on a per channel basis"
 *           struct
 *              u32 chan
 *              u32 coef1
 *              u32 coef2
 *              u32 coef3
 *              u32 coef4
 *              u32 coef5       // 6 fields * 4 bytes/per  = 24 bytes in this packet
 */


// BitMasks for AtoD channels
#define BM_ATOD_CHAN1   0x0001
#define BM_ATOD_CHAN2   0x0002
#define BM_ATOD_CHAN3   0x0004
#define BM_ATOD_CHAN4   0x0008
#define BM_ATOD_CHAN5   0x0010
#define BM_ATOD_CHAN6   0x0020
#define BM_ATOD_CHAN7   0x0040
#define BM_ATOD_CHAN8   0x0080
#define BM_ATOD_CHAN9   0x0100
#define BM_ATOD_CHAN10  0x0200
#define BM_ATOD_CHAN11  0x0400
#define BM_ATOD_CHAN12  0x0800
#define BM_ATOD_CHAN13  0x1000
#define BM_ATOD_CHAN14  0x2000
#define BM_ATOD_CHAN15  0x4000
#define BM_ATOD_CHAN16  0x8000

// Name Translation Entries
#define AD_NTENTRYS   19

#define I0_CDv    1
#define I0_SYp    2
#define I0_CSp    3
#define I0_CSt    4
#define I0_CBt    5
#define I0_TWt    6
#define I0_TWl    7
#define I0_SYw    8
#define I0_CPw    9
#define I0_CLt    10
#define I0_WSt    11
#define A1_SYw    12
#define A1_BLw    13
#define A1_CPw    14
#define A2_SYw    15
#define A2_BLw    16
#define A2_CPw    17
#define V5_Ref    18
#define GndRef    19


#define  ANI_TYPE_V_CORRECTED                     1
#define  ANI_TYPE_V_LINEAR_CORRECTION             3
#define  ANI_TYPE_V_LINEAR_RATIO_METRIC           5





typedef struct
{
    char       ConfigName[10];
    u32        ConfigNum;
    bool       Enabled;
    u32        Type;
    float      Coefs[5];
    u16        ValRaw;
    float      ValCooked;
} ADInputs_t;


typedef struct
{
    const char  *Nptr;
    u32          Nval;
} AD_NameTransTable;


typedef struct
{
    u8    an_Name;     // 'I0_CDv 1'  to   'GndRef  19'                   (1..19)
    u8    an_Enbl;     // 0=disabled, 1=enabled                           (0,1)
    u8    an_Type;     // ANI_TYPE_V_CORRECTED to V_LINEAR_RATIO_METRIC   (1,3,5)
} AN_MSG1;

typedef struct
{
    AN_MSG1    ad_fields[NUM_AD_CHANS];
    u32        coef1;
    u32        coef2;
} AD_MSGPKT;                  // There is not a variable of this, its the data format of an incoming 'config packet'




GLOBALLY_VISIBLE  AD_SignalDef_t CSVar[CONTROLSIGCOUNT];           // for convenience.          Per Signal

static ADInputs_t     AtoD_Data[NUM_AD_CHANS];                     // AtoD Data + Config info.  Per Channel
static u16            AtoD_ConfiguredChannels;                     // bitmap of configured channels: BM_ATOD_CHAN1 to BM_ATOD_CHAN16

static AD_NameTransTable   AD_NTtable[AD_NTENTRYS] =
{
    { "I0.CDv", I0_CDv  },
    { "I0.SYp", I0_SYp  },
    { "I0.CSp", I0_CSp  },
    { "I0.CSt", I0_CSt  },
    { "I0.CBt", I0_CBt  },
    { "I0.TWt", I0_TWt  },
    { "I0.TWl", I0_TWl  },
    { "I0.SYw", I0_SYw  },
    { "I0.CPw", I0_CPw  },
    { "I0.CLt", I0_CLt  },
    { "I0.WSt", I0_WSt  },
    { "A1.SYw", A1_SYw  },
    { "A1.BLw", A1_BLw  },
    { "A1.CPw", A1_CPw  },
    { "A2.SYw", A2_SYw  },
    { "A2.BLw", A2_BLw  },
    { "A2.CPw", A2_CPw  },
    { "5v_Ref", V5_Ref  },
    { "GndRef", GndRef  }
};


static u8 Canned_AtoD_config[56] = {
   0x01, 0x01, 0x01,
   0x02, 0x00, 0x01,
   0x03, 0x00, 0x05,
   0x0c, 0x00, 0x03,
   0x0d, 0x00, 0x03,
   0x06, 0x00, 0x03,
   0x07, 0x00, 0x03,
   0x08, 0x00, 0x03,
   0x09, 0x00, 0x03,
   0x0a, 0x00, 0x03,
   0x0b, 0x00, 0x03,
   0x04, 0x00, 0x03,
   0x05, 0x00, 0x03,
   0x0e, 0x00, 0x03,
   0x12, 0x00, 0x01,
   0x13, 0x00, 0x01,
   0x96, 0x62, 0x00, 0x00,
   0x1f, 0x27, 0x00, 0x00
};


static void        AD_DoConfig( u8 *dptr );
static const char *Get_AD_NameTrans( uint iNum );
static float       Get_Chan0_ValCooked( void );




//
GLOBALLY_VISIBLE void AtoD_Init( void )
{
    int i;

    AD_DoConfig( Canned_AtoD_config );

    for( i=0; i < CONTROLSIGCOUNT; ++i )
    {
        CSVar[i].Value = 0.0;
    }

    for( i=0; i < NUM_AD_CHANS; ++i )
    {
        AtoD_Data[i].ValRaw    = 0;
        AtoD_Data[i].ValCooked = 0.0;
    }
}


GLOBALLY_VISIBLE void AtoD_dbgShowStuff( void )
{
    ADInputs_t  *AD = &AtoD_Data[12];                         // random, just pick one     0..15

    UD_Print32( "AtoD_ConfiguredChannels = ", (u32)AtoD_ConfiguredChannels );

    printf    ( "cd[%d].ConfigName = %s\n\r", 12, AD->ConfigName );
    UD_Print32( "cd[x].ConfigNum   = ",          AD->ConfigNum);
    UD_Print32( "cd[x].enabled     = ",          AD->Enabled);
    UD_Print32( "cd[x].type        = ",          AD->Type);

    printf("WETSUCT_T:  %d\n\r", (int)(CSVar[WETSUCT_T].Value * 100.0));
    printf("SUCT_T:     %d\n\r", (int)(CSVar[SUCT_T].Value    * 100.0));
    printf("CAB_T:      %d\n\r", (int)(CSVar[CAB_T].Value     * 100.0));
}


GLOBALLY_VISIBLE u16  AtoD_Get_ConfiguredChannels( void )
{
    return ( AtoD_ConfiguredChannels | 1 );          // OR with 1 means Chan 0 is ALWAYS configured
}


//
//   channel:  0..15
//
GLOBALLY_VISIBLE void AtoD_Set_value( u8 channel, u16 raw_val )
{
    float        C0,C1,C2,Fv_avg,tmpf;
    ADInputs_t  *AD;

    if( channel >= NUM_AD_CHANS ) { return; }         // the stupid test

    AD     = &AtoD_Data[channel];
    C0     = AD->Coefs[0];
    C1     = AD->Coefs[1];
    Fv_avg = (float)raw_val / (float)6553.6;

    switch( AD->Type )
    {
       case ANI_TYPE_V_LINEAR_CORRECTION:    { tmpf = (Fv_avg * C0) + C1;                            } break;

       case ANI_TYPE_V_LINEAR_RATIO_METRIC:  { C2   = AD->Coefs[2];
                                               tmpf = ((Fv_avg / Get_Chan0_ValCooked()) * C1) + C2;  } break;
       case ANI_TYPE_V_CORRECTED:
       default:                              { tmpf = Fv_avg;                                        } break;
    }

    AD->ValCooked = tmpf;
    AD->ValRaw    = raw_val;

    // *** Note: no default.
    //           not all AtoD's are stored as signals.   Only the following 9 signals get stored.
    switch( AD->ConfigNum )
    {
        case I0_SYp:  CSVar[SYS_P].Value     = tmpf;   break;
        case I0_CSp:  CSVar[SUCT_P].Value    = tmpf;   break;
        case I0_CSt:  CSVar[SUCT_T].Value    = tmpf;   break;
        case I0_CBt:  CSVar[CAB_T].Value     = tmpf;   break;
        case I0_TWt:  CSVar[WATER_T].Value   = tmpf;   break;
        case I0_SYw:  CSVar[SYS_POW].Value   = tmpf;   break;
        case I0_CPw:  CSVar[COM_POW].Value   = tmpf;   break;
        case I0_CLt:  CSVar[CONDLIQ_T].Value = tmpf;   break;
        case I0_WSt:  CSVar[WETSUCT_T].Value = tmpf;   break;
    }
}






static const char *Get_AD_NameTrans( uint iNum )
{
    uint i;

    for( i=0; i < AD_NTENTRYS; ++i )
    {
        if( AD_NTtable[i].Nval == iNum ) { return AD_NTtable[i].Nptr; }
    }

    return AD_NTtable[0].Nptr;      // failsafe
}


static void AD_DoConfig( unsigned char *dptr )
{
    const char  *Sptr;
    AD_MSGPKT   *MSGptr;
    ADInputs_t  *AD;
    u32          i;

    MSGptr = (AD_MSGPKT *)dptr;                   // points to Start of data in the packet

    AtoD_ConfiguredChannels = 0;

    for( i=0; i < NUM_AD_CHANS; ++i)
    {
        AD = &AtoD_Data[i];

        AD->ConfigNum = (u32)MSGptr->ad_fields[i].an_Name;

        Sptr = Get_AD_NameTrans(AD->ConfigNum);
        strcpy(AD->ConfigName, Sptr);

        AD->Enabled = false;
        if( MSGptr->ad_fields[i].an_Enbl == 1 )
        {
            AtoD_ConfiguredChannels |= (1 << i);
            AD->Enabled = true;
        }

        AD->Type = MSGptr->ad_fields[i].an_Type;

        if( AD->Type == ANI_TYPE_V_LINEAR_RATIO_METRIC )
        {
            AD->Coefs[0] = 1.0;
            AD->Coefs[1] = 250.0;
            AD->Coefs[2] = -25.0;
            AD->Coefs[3] = 1.0;
            AD->Coefs[4] = 1.0;
        }
        else if( AD->Type == ANI_TYPE_V_LINEAR_CORRECTION )
        {
            AD->Coefs[0] = 100.0;
            AD->Coefs[1] = 0.0;
            AD->Coefs[2] = 1.0;
            AD->Coefs[3] = 1.0;
            AD->Coefs[4] = 1.0;
        }
        else    // default to ANI_TYPE_V_CORRECTED
        {
            AD->Coefs[0] = 1.0;
            AD->Coefs[1] = 1.0;
            AD->Coefs[2] = 1.0;
            AD->Coefs[3] = 1.0;
            AD->Coefs[4] = 1.0;
        }

        if     ( AD->ConfigNum == I0_SYw )  { AD->Coefs[0] = ((float)MSGptr->coef1 / (float)100.0); }
        else if( AD->ConfigNum == I0_CPw )  { AD->Coefs[0] = ((float)MSGptr->coef2 / (float)100.0); }
    }

    if( AtoD_ConfiguredChannels == 0 ) { AtoD_ConfiguredChannels = BM_ATOD_CHAN1; }
}


//
//  safety:  prevents divide by 0
static float Get_Chan0_ValCooked( void )
{
    float V = AtoD_Data[0].ValCooked;

    if( V < 0.1 )
        return 5.0;
    else
        return V;
}



GLOBALLY_VISIBLE void  AtoD_Set_NewConfig( u8 Len, u8 *Dptr )
{
    // c u8 Canned_AtoD_config[56] = {

    memcpy( (void *)Canned_AtoD_config, (void *)Dptr, 56 );        // copy local
    I2C_1_master_NewConfig();                                      // provide trigger, so it's synced up with the thread
}


GLOBALLY_VISIBLE void  AtoD_Get_Reading( u8 channel, u16 *raw, float *cooked )
{
    if( channel >= NUM_AD_CHANS )         // if channel out-of-bounds, force to channel 0
        channel = 0;                      //   so that something valid is returned

    *raw    = AtoD_Data[channel].ValRaw;
    *cooked = AtoD_Data[channel].ValCooked;
}


//
GLOBALLY_VISIBLE void AtoD_GetAllReadings( u32 *Dptr )
{
    u8           i;
    ADInputs_t  *AD;
    u16          Lraw;
    float        Lcooked;


    for( i=0; i < 16; ++i )
    {
        AD = &AtoD_Data[i];

        if( AD->Enabled == true )
        {
            AtoD_Get_Reading( i, &Lraw, &Lcooked );
            Dptr[i] = (u32)(Lcooked * 1000.0);
        }
        else
        {
            Dptr[i] = 0;
        }
    }
}



/*----------------------------------------------------------------------
void AD_GetConfig( void )
{
    char           dbuf[4];
    VDK_MessageID  AD_ConfigMsg;
    int            i;

    for( i=0; i < 4; ++ i)
    {
        dbuf[0] = 0x45;      // really just a marker for looking at the data on the host side
        dbuf[1] = 0x91+i;    //   same

        AD_ConfigMsg = VDK_CreateMessage(SPI_SENDMSG_AD_CONFIG, 2, (void *)dbuf);
        VDK_PostMessage(kSpiSendThread_ID, AD_ConfigMsg, MSG_CHANNEL_SENDSPI);

        if (VDK_PendSemaphore(kADConfigPacketArrived, 20000 | VDK_kNoTimeoutError) == false)
        {
            dbg_print( "AD_GetConfig, Timeout waiting on response", false, 0 );
            VDK_Sleep(3000);
        }
        else
        {
            dbg_print( "AD_GetConfig, GOOD!", false, 0 );
            break;
        }
    }
} */



