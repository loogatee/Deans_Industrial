#ifndef INCLUDE_DATABUFFER_H_
#define INCLUDE_DATABUFFER_H_


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


void  DataBuffer_Init  ( void     );
u8   *DataBuffer_Get   ( void     );
void  DataBuffer_Return( u8 *Dptr );


#endif /* INCLUDE_DATABUFFER_H_ */
