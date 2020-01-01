#ifndef PROJ_COMMON_H_
#define PROJ_COMMON_H_

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


#define VERSION_STR     "Deans Industrial  V1_"
#define VERSION_MAJOR   1
#define VERSION_MINOR   1
#define VERSION_DATE    "1/1/2020 12:21\r\n"





#define true   1
#define True   1
#define TRUE   1
#define false  0
#define False  0
#define FALSE  0



#define ASCII_BACKSPACE         8
#define ASCII_LINEFEED          10
#define ASCII_CARRIAGERETURN    13
#define ASCII_SPACE             32
#define ASCII_TILDE             126
#define ASCII_DELETE            127


#define FALL_THRU
#define GLOBALLY_VISIBLE
#define STM_REGISTER
#define STATIC              static


typedef unsigned int     Bool;
typedef unsigned int     bool;
typedef unsigned int     BOOL;


union uW2B
{
    u16  w;
    u8   b[2];
};

typedef union uW2B   UW2B;



#endif /* PROJ_COMMON_H_ */


















