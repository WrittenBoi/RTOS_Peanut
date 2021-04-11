#include "task.h"

// 就绪列表
static List_t pxReadyTaskLists[configMAX_PRIORITIES] = { 0 };

// 当前任务
TCB_t *volatile pxCurrentTCB = NULL;

// 初始化任务栈
static TaskHandle_t prvInitialiseNewTask(TaskFunction_t pxTaskCode,
        const portCHAR *const pcTaskName, const uint32_t ulStackDepth,
        void *const pvParameters, TCB_t *const pxNewTCB);

// 初始化就绪列表
static void prvInitialiseTaskLists(void);

static void prvInitialiseTaskLists(void) {
    for (UBaseType_t cnt = 0; cnt < (UBaseType_t) configMAX_PRIORITIES; cnt++) {
        vListInit(&pxReadyTaskLists[cnt]);
    }
}

static TaskHandle_t prvInitialiseNewTask(TaskFunction_t pxTaskCode,
        const portCHAR *const pcTaskName, const uint32_t ulStackDepth,
        void *const pvParameters, TCB_t *const pxNewTCB) {
    StackType_t *pxTopOfStack; //栈顶指针

    //计算栈顶（需要8字节对齐），这里默认栈向低地址增长
    pxTopOfStack = pxNewTCB->pxTack + (ulStackDepth - (uint32_t) 1);
    pxTopOfStack =
            (StackType_t*) (((uint32_t) pxTopOfStack) & (~((uint32_t) 7)));

    //拷贝TaskName
    for (UBaseType_t cnt = 0; cnt < (UBaseType_t) (configMAX_TASK_NAME_LEN - 1);
            cnt++) {
        pxNewTCB->pcTaskName[cnt] = pcTaskName[cnt];
        if ('\0' == pcTaskName[cnt]) {
            break;
        }
    }
    pxNewTCB->pcTaskName[configMAX_TASK_NAME_LEN - 1] = '\0';

    //初始化任务List节点
    vListInitNode(&(pxNewTCB->xStateListNode));
    vListSetNodeOwner(&(pxNewTCB->xStateListNode), (void* ) pxNewTCB);

    //初始化任务栈，为从中断返回做好准备
    pxNewTCB->pxTopOfStack = pxPortInitialiseStack(pxTopOfStack, pxTaskCode,
            pvParameters);

    return (TaskHandle_t) pxNewTCB;

}

TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
        const portCHAR *const pcTaskName, const uint32_t ulStackDepth,
        void *const pvParameters, StackType_t *const pxStackBuffer,
        TCB_t *const pxTaskBuffer) {

    TCB_t *pxNewTCB;
    TaskHandle_t xReturn;

    if ((NULL != pxTaskBuffer) && (NULL != pxStackBuffer)) {
        pxNewTCB = pxTaskBuffer;
        pxNewTCB->pxTack = pxStackBuffer;
        xReturn = prvInitialiseNewTask(pxTaskCode, pcTaskName, ulStackDepth,
                pvParameters, pxNewTCB);
    } else {
        xReturn = NULL;
    }
    return xReturn;
}

void vTaskStartScheduler(void) {
    extern TCB_t Task1TCB;
    pxCurrentTCB = &Task1TCB;

    prvInitialiseTaskLists();

    if (pdFALSE != xPortStartScheduler()) {
        ; // 如果调度器初始化成功，不会执行到这里
    }
}
