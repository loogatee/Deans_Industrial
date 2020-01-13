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


//    CDCAPI:    cmd byte in the packet going over the I/O channel


//
// originates on CDC, Needs response
//
#define   CDCAPI_DLOOP           22
#define   CDCAPI_DLOOP2          24
#define   CDCAPI_ANCONFIG        28


// originates on CDC, NO response
#define   CDCAPI_BLASTAWAY       102




//
//  originates on Host, Needs response
//
#define   CDCAPI_DLOOP_FHOST     25
#define   CDCAPI_ADGETVALS       29
#define   CDCAPI_ADGETALL        30
#define   CDCAPI_CMDGETVAL       31
#define   CDCAPI_IMUP            103


//
//  originates on Host, Does not need response
//
#define   CDCAPI_SETLED          26
#define   CDCAPI_RESETDEVICE     27

#define   CDCAPI_JRTEST          199



/*----------------------------------------------------------------


#define   CDCAPI_DLOOP           22
#define   CDCAPI_DLOOP2          24
#define   CDCAPI_DLOOP_FHOST     25
#define   CDCAPI_SETLED          26
#define   CDCAPI_RESETDEVICE     27
#define   CDCAPI_ANCONFIG        28
#define   CDCAPI_ADGETVALS       29
#define   CDCAPI_ADGETALL        30
#define   CDCAPI_CMDGETVAL       31
#define   CDCAPI_BLASTAWAY       102
#define   CDCAPI_IMUP            103
#define   CDCAPI_JRTEST          199


----------------------------------------------------------------*/


#endif
