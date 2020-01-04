#ifndef INCLUDE_PACKETSENDTHREAD_H_
#define INCLUDE_PACKETSENDTHREAD_H_


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


void PacketSend_TaskCreate( void );
void PacketSend_xfer_Done( void );

//void PacketSend_imup2( u8 len, u8 *dataptr );
//void PacketSend_ADconfig( u8 len, u8 *dataptr );
//void PacketSend_DloopResp( u8 len, u8 *dataptr );
//void PacketSend_AllADResp( u8 len, u8 *dataptr, u32 f2 );

void PacketSend_pakt( u8 cmd, u8 len, u8 *dataptr, u32 zero_or_retBuf );


#endif /* INCLUDE_PACKETSENDTHREAD_H_ */
