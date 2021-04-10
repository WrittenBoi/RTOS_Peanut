#ifndef __PEANUT_PORT_H
#define __PEANUT_PORT_H

#include "port_macro.h"
#include "rtos_config.h"
#include "projdefs.h"
#include "list.h"

StackType_t* pxPortInitialiseStack(StackType_t *pxTopOfStack,
        TaskFunction_t pxCode, void *const pvParameters);

#endif // __PEANUT_PORT_H
