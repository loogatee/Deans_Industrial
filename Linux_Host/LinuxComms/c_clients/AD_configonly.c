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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include "proj_common.h"
#include "cdc_cmds.h"
#include "utils.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#undef BUFLEN
#undef LUTILS_FILENAME

#define LUTILS_FILENAME    "/usr/local/bin/lutils.lua"
#define BUFLEN             2048


typedef struct
{
    u8          dbuf[BUFLEN];
    void       *context1,*context2,*context3;
    void       *RcvFromSerial;
    void       *TxToSerial;
    void       *BackChan3;
    lua_State  *LS1;
    uint        TotBytes,TotPacks;
} Globes;



static Globes    Globals;

/*
static void getLSstackval(char *rptr)
{
    lua_next(Globals.LS1, -2);
    const char *rp = lua_tostring(Globals.LS1,-1);
    lua_pop(Globals.LS1, 1);
    strcpy(rptr,rp);
}
*/




//
//
//
static int getstackval(void)
{
    lua_next(Globals.LS1, -2);
    int rv = lua_tointeger(Globals.LS1,-1);
    lua_pop(Globals.LS1, 1);
    return rv;
}


//    CAll a Lua function to do all the 'heavy' lifting.   We need three values
//    for each A/D sensor:
//          - Name
//          - Enbl
//          - Type
//    And we also need 2 coefficients: one for SYw and one for CPw.
//    The Lua function pushes all the needed values onto the stack, in the 
//    form of a Lua Table.   This code then just pops the values from the stack
//    and puts them right into the data packet.
//
//     **** Look for swapping issues on the memcpy for host/devices pairs. ****
//
//    Weirdism with Lua:  even though there are many values on the stack, they
//                        are all contained within a single table, that gets
//                        pushed onto the stack. This means there is only
//                        1 return value from the Lua call.
//
//
//        name,enable,type
//        (u8,  u8,    u8) * 16  = 48 bytes
//        (u32) * 2              =  8 bytes
//                             -------------
//                                 56
//
static void PH_AnConfig( void )
{
    u8       dbuf[BUFLEN];
    int      I1,I2,i;
    u8      *dptr = dbuf;
    Globes  *G = &Globals;

#define SIZE_ANCONFIG_PACKET   56

    *dptr++ = CDCAPI_ANCONFIG;                                  // Byte[0] = Cmd = 28
    *dptr++ = SIZE_ANCONFIG_PACKET;                             // Byte[1] = Len

    lua_getglobal(G->LS1,"lua_Get_AD_Config");                  // This is the name of the Lua function
    lua_pcall(G->LS1,0,1,0);                                    // executes the Lua function, 1 return arg on stack
    lua_pushnil(G->LS1);                                        // precursor needed for reading stack values

    for(i=0; i < 16; ++i)
    {
        *dptr++ = (u8)getstackval();                            // Name
        *dptr++ = (u8)getstackval();                            // Enbl
        *dptr++ = (u8)getstackval();                            // Type
    }

    I1 = getstackval();                                         // Coef for SYw
    I2 = getstackval();                                         // Coef for CPw

    memcpy((void *)dptr,    (const void *)&I1, 4);              // Identical Byte ordering allows this
    memcpy((void *)(dptr+4),(const void *)&I2, 4);              //   this being:  Intel host + Blackfin device
                                                                //   *** might be different for RPI or Beagle ***

//    printf("%02x,%02x   %02x,%02x,%02x   %02x,%02x,%02x\n",dbuf[0],dbuf[1],dbuf[2],dbuf[3],dbuf[4],dbuf[5],dbuf[6],dbuf[7]);

    dptr = &dbuf[2];

    printf("static u8 Canned_AtoD_config[56] = {\n\r");
 
    for( i=0; i<16; ++i)
    {
        printf("   0x%02x, 0x%02x, 0x%02x,\n\r",dptr[0],dptr[1],dptr[2]);
        dptr += 3;
    }

    
    printf("   0x%02x, 0x%02x, 0x%02x, 0x%02x,\n\r",dptr[0],dptr[1],dptr[2],dptr[3]);
    printf("   0x%02x, 0x%02x, 0x%02x, 0x%02x\n\r",dptr[4],dptr[5],dptr[6],dptr[7]);

    printf("};\n\r");
}






int main( void )
{
    //u8   cmd;
    //int  len;

    Globes *G   = &Globals;
    G->TotBytes = 0;
    G->TotPacks = 0;

    G->LS1 = luaL_newstate();                           //   This makes Lua available (like for reading configs!)
    luaL_openlibs(G->LS1);                              //   Book says to also do this

    if( luaL_dofile(G->LS1, LUTILS_FILENAME) != 0 )        //   Can now extend the app with these lua functions
    {
        printf("ERROR opening %s\n", LUTILS_FILENAME);
        exit(1);
    }

    PH_AnConfig();

    return 0;
}








