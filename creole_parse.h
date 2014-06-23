/*
* Copyright 2014, carrotsrc.org
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef CREOLE_PARSE_H
#define CREOLE_PARSE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CG_BOLD 1
#define CG_ITALIC 2

#define CG_UL 8
#define CG_OL 16

#define CG_NFO 128

#define CL_CTL 1
#define CL_STR 2
#define CL_TRAIL 4

#define CL_OPEN_HEADER 8
#define CL_HEADER 16
#define CL_LIST 32

#define CL_AHREF 64
#define CL_ATITLE 128

char *creole_parse(char*, const char*, int);

#endif
