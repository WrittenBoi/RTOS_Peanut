#ifndef __PEANUT_PORT_MACRO_H
#define __PEANUT_PORT_MACRO_H

#include <stdio.h>
#include <stdint.h>

#if 0
#define portCHAR char
#define portLONG long
#define portSHORT short
#define portSTACK_TYPE uint32_t
#define portBASE_TYPE long
#define portUBASE_TYPE unsigned long
#else
typedef char portCHAR;
typedef long portLONG;
typedef short portSHORT;
typedef long portBASE_TYPE;
typedef unsigned long portUBASE_TYPE;
typedef uint32_t portSTACK_TYPE;
#endif

typedef portSTACK_TYPE StackType_t;
typedef portBASE_TYPE BaseType_t;
typedef portUBASE_TYPE UBaseType_t;

#define portMAX_DELAY   ((TickType_t)0xffffffffUL)
typedef uint32_t TickType_t;

#endif // __PEANUT_PORT_MACRO_H
