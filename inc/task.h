#ifndef __PEANUT_TASK_H
#define __PEANUT_TASK_H

#include "port_macro.h"
#include "rtos_config.h"
#include "projdefs.h"
#include "list.h"
#include "port.h"

// 任务切换
#define taskYIELD()         portYIELD()

typedef struct stTaskControlBlock {
    volatile StackType_t *pxTopOfStack;
    ListNode_t xStateListNode;
    StackType_t *pxTack;
    TickType_t xDelayTicks;
    UBaseType_t uxPriority;
    portCHAR pcTaskName[configMAX_TASK_NAME_LEN];
} TCB_t;

typedef void *TaskHandle_t;

// 不能用于中断的临界区保护函数，可嵌套使用
#define taskENTER_CRITICAL()            portENTER_CRITICAL()
#define taskEXIT_CRITICAL()             portEXIT_CRITICAL()

// 能用于中断的临界区保护函数，可嵌套使用
#define taskENTER_CRITICAL_FROM_ISR()   portSET_INTERRUPT_MASK_FROM_ISR()
#define taskEXIT_CRITICAL_FROM_ISR(x)   portCLR_INTERRUPT_MASK_FROM_ISR((x))

// 无中断保护的中断屏蔽函数，不可嵌套使用
#define taskDISABLE_INTERRUPTS()        portDISABLE_INTERRUPTS()
#define taskENABLE_INTERRUPTS()         portENABLE_INTERRUPTS()

#if configSUPPORT_STATIC_ALLOCATION
// 静态创建Task
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
                const portCHAR *const pcTaskName, const uint32_t ulStackDepth,
                void *const pvParameters, UBaseType_t uxPriority,
                StackType_t *const pxStackBuffer,
                TCB_t *const pxTaskBuffer);
#endif

// 启动调度器
void vTaskStartScheduler(void);

// 任务延时
void vTaskDelay(const TickType_t xDelayTicks);

// 更新系统时基
BaseType_t xTaskIncrementTick();

#endif // __PEANUT_TASK_H
