#ifndef __PEANUT_PORT_H
#define __PEANUT_PORT_H

#include "port_macro.h"
#include "rtos_config.h"
#include "projdefs.h"
#include "list.h"
#include "task.h"

// 初始化任务栈
StackType_t* pxPortInitialiseStack(StackType_t *pxTopOfStack,
        TaskFunction_t pxCode, void *const pvParameters);

// 启动调度器
BaseType_t xPortStartScheduler(void);

#endif // __PEANUT_PORT_H
