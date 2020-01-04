#ifndef _CDC_CMDS_H_
#define _CDC_CMDS_H_

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


//    CDCCMD:    commands between Host App and PacketCentral
//
//    CDCAPI:    cmd byte in the packet going over the I/O channel


#define   CDCCMD_DLOOP         0x0001
#define   CDCCMD_SETLED        0x0002
#define   CDCCMD_RESETDEV      0x0003
#define   CDCCMD_ADGETVALS     0x0004
#define   CDCCMD_ADGETALLAVGS  0x0005




//
// originates on CDC, Needs response
//
#define   CDCAPI_DLOOP           22
#define   CDCAPI_DLOOP2          24
#define   CDCAPI_ANCONFIG        28


// originates on CDC, NO response
#define   CDCAPI_BLASTAWAY       0x66


//
//  originates on Host, Needs response
//
#define   CDCAPI_DLOOP_FHOST     25
#define   CDCAPI_ADGETVALS       29
#define   CDCAPI_ADGETALL        30
#define   CDCAPI_IMUP            0x67


//
//  originates on Host, Does not need response
//
#define   CDCAPI_SETLED          26
#define   CDCAPI_RESETDEVICE     27






#define   CDCAPI_JRTEST          199





#endif
