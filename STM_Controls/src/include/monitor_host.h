#ifndef INCLUDE_MONITOR_HOST_H_
#define INCLUDE_MONITOR_HOST_H_


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

void MonitorHost_Init( void );
void MonitorHost_Process( void );
bool MonitorHost_IsUp( void );
u32  MonitorHost_GetState( void );
void MonitorHost_IMUP_pakt( void );


#endif /* INCLUDE_MONITOR_HOST_H_ */
