#ifndef _UTILS_H_
#define _UTILS_H_

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


void     BtoH( u8 val, char *S );
void     BtoHnz( u8 val, char *S );
void     ItoH( u32 val, char *S );
void     ItoH16( u16 val, char *S );
uint32_t HtoI( const char *ptr );
int      AtoI( const char *p );
uint16_t HtoU16( char *pstr );
void     Hammer( u8 nparm );
void     swap( u16 *sptr );

#endif
