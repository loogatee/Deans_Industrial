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
#include <fcntl.h>
#include "proj_common.h"
#include "cdc_cmds.h"
#include "utils.h"


int main( void )
{
    int  fd;

    fd = open("/sys/class/gpio/export",           O_WRONLY);  write(fd,"137\n",4);  close(fd); 
    fd = open("/sys/class/gpio/gpio137/direction",O_WRONLY);  write(fd,"out\n",4);  close(fd);
    fd = open("/sys/class/gpio/gpio137/value",    O_WRONLY);  write(fd,"1",    1);  close(fd);
}








