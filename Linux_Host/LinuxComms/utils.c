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

#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include "proj_common.h"




typedef struct
{
    const char  *Nptr;
    uint         Nval;
} AD_NameTransTable;

#define AD_NTENTRYS   19

static AD_NameTransTable   AD_NTtable[AD_NTENTRYS] =
{
    { "I0.CDv", 1  },
    { "I0.SYp", 2  },
    { "I0.CSp", 3  },
    { "I0.CSt", 4  },
    { "I0.CBt", 5  },
    { "I0.TWt", 6  },
    { "I0.TWl", 7  },
    { "I0.SYw", 8  },
    { "I0.CPw", 9  },
    { "I0.CLt", 10 },
    { "I0.WSt", 11 },
    { "A1.SYw", 12 },
    { "A1.BLw", 13 },
    { "A1.CPw", 14 },
    { "A2.SYw", 15 },
    { "A2.BLw", 16 },
    { "A2.CPw", 17 },
    { "5v_Ref", 18 },
    { "GndRef", 19 }
};



int  Utils_Get_AD_NameTrans( char *S )
{
    uint i;

    for( i=0; i < AD_NTENTRYS; ++i )
    {
        if(!strcmp(S,AD_NTtable[i].Nptr)) { return AD_NTtable[i].Nval; }
    }

    return -1;
}





