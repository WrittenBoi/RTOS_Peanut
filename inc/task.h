#ifndef __PEANUT_TASK_H
#define __PEANUT_TASK_H

#include "port_macro.h"
#include "rtos_config.h"
#include "projdefs.h"
#include "list.h"
#include "port.h"

typedef struct stTaskControlBlock {
    volatile StackType_t *pxTopOfStack;
    ListNode_t xStateListNode;
    StackType_t *pxTack;
    portCHAR pcTaskName[configMAX_TASK_NAME_LEN];
} TCB_t;

typedef void *TaskHandle_t;

#if configSUPPORT_STATIC_ALLOCATION
// 静态创建Task
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
        const portCHAR *const pcTaskName, const uint32_t ulStackDepth,
        void *const pvParameters, StackType_t *const pxStackBuffer,
        TCB_t *const pxTaskBuffer);
#endif

// 启动调度器
void vTaskStartScheduler(void);

#endif // __PEANUT_TASK_H
