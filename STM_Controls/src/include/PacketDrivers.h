#ifndef INCLUDE_PACKETDRIVERS_H_
#define INCLUDE_PACKETDRIVERS_H_

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

#define PAKTMAG1i  0
#define PAKTMAG2i  1
#define PAKTSEQNi  2
#define PAKTLENi   3
#define PAKTCMDi   4
#define PAKTSODi   5


void PacketSendDriver_Init( void );
void PacketSendDriver_Process( void );
void PacketSendDriver_Go( u8 *Packetptr, u32 parm );

void PacketRecvDriver_Init( void );
void PacketRecvDriver_Process( void );

#endif /* INCLUDE_PACKETDRIVERS_H_ */
