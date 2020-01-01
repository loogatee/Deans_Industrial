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
#include <string.h>




static char hexv[] =  { '0', '1', '2', '3', '4', '5', '6', '7',
                        '8', '9' ,'A', 'B', 'C', 'D', 'E', 'F' };



void BtoH( u8 val, char *S )
{
    s8 k;

    for( k=1; k >= 0; --k )
    {
        S[k]   = hexv[val & 0xF];
        val  >>= 4;
    }

    S[2] = 0;
}

void BtoHnz( u8 val, char *S )
{
    s8 k;

    for( k=1; k >= 0; --k )
    {
        S[k]   = hexv[val & 0xF];
        val  >>= 4;
    }
}




void ItoH( u32 val, char *S )
{
    s8  k;

    for( k=7; k >= 0; --k )
    {
        S[k]   = hexv[val & 0xF];
        val  >>= 4;
    }

    S[8] = 0;
}


void ItoH16( u16 val, char *S )
{
    s8  k;

    for( k=3; k >= 0; --k )
    {
        S[k]   = hexv[val & 0xF];
        val  >>= 4;
    }

    S[4] = 0;
}



uint32_t HtoI( const char *ptr )
{
    uint32_t  value = 0;
    char      ch      = *ptr;

    while (ch == ' ' || ch == '\t')
        ch = *(++ptr);

    for (;;) {

        if (ch >= '0' && ch <= '9')
            value = (value << 4) + (ch - '0');
        else if (ch >= 'A' && ch <= 'F')
            value = (value << 4) + (ch - 'A' + 10);
        else if (ch >= 'a' && ch <= 'f')
            value = (value << 4) + (ch - 'a' + 10);
        else
            return value;
        ch = *(++ptr);
    }
}


int AtoI( const char *p )
{
    int n,f;

    n=f=0;

    for( ; ; p++ )
    {
        switch( *p )
        {
            case ' ':
            case '\t':    continue;
            case '-':     f++;
            case '+':     p++;
        }
        break;
    }

    while( *p >= '0' && *p <= '9')
        n = n*10 + *p++ - '0';

    return(f ? -n : n);
}


uint16_t HtoU16( char *pstr )
{
    uint8_t    i,j;
    uint16_t   ch,sum;

    switch( strlen(pstr) )
    {
        case 1:   j = 0;   break;
        case 2:   j = 4;   break;
        case 3:   j = 8;   break;
        default:  j = 12;  break;
    }

    for( i=0,sum=0; pstr[i] && i < 4; ++i,j-=4 )
    {
        ch = pstr[i];

        if     ( ch >= '0' && ch <= '9' ) ch -= '0';
        else if( ch >= 'A' && ch <= 'F' ) ch -= '7';
        else if( ch >= 'a' && ch <= 'f' ) ch -= 'W';
        else                              ch  = 0;

        sum = sum | ( ch << j );
    }
    return sum;
}


void Hammer( u8 nparm )
{
        ;
}


void swap( u16 *sptr )
{
    u8  *bptr = (u8 *)sptr;
    u8  tmp   = bptr[0];

    bptr[0] = bptr[1];
    bptr[1] = tmp;
}









