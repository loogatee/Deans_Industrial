#ifndef I2C_H
#define I2C_H

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



#define I2C_CMDTYPE_RW                   0
#define I2C_CMDTYPE_WRITEONLY            1
#define I2C_CMDTYPE_CMDREG2              0x80


#define I2C_COMPLETION_BUSY              0
#define I2C_COMPLETION_OK                1
#define I2C_COMPLETION_TIMEOUT           2


typedef struct
{
    u8      ct_slaveaddr;         // i2c Slave Address
    u8      ct_cmdreg;            // i2c Command Register
    u8      ct_cmdreg2;           // i2c 2nd Command Register
    u8      ct_numbytes;          // Number of bytes to Read or Write
    u8      ct_cmdtype;           // I2C_CMDTYPE_xx
} I2CCMDS;




void I2C_1_master_Init(void);
void I2C_1_master_Process(void);
void I2C_1_master_NewConfig(void);

//extern void I2C_2master_Init(u8 baudrate);
//extern void I2C_2master_Process(void);
//extern bool I2C_2_SendCmd(u8 *dptr, u8 *wptr, u8 *complptr, I2CCMDS *lptr);


#endif

