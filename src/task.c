#include "task.h"

// 初始化任务栈
static TaskHandle_t prvInitialiseNewTask(TaskFunction_t pxTaskCode,
        const portCHAR *const pcTaskName, const uint32_t ulStackDepth,
        void *const pvParameters, TCB_t *const pxNewTCB);

static TaskHandle_t prvInitialiseNewTask(TaskFunction_t pxTaskCode,
        const portCHAR *const pcTaskName, const uint32_t ulStackDepth,
        void *const pvParameters, TCB_t *const pxNewTCB) {
    StackType_t *pxTopOfStack; //栈顶指针

    //计算栈顶（需要8字节对齐），这里默认栈向低地址增长
    pxTopOfStack = pxNewTCB->pxTack + (ulStackDepth - (uint32_t) 1);
    pxTopOfStack =
            (StackType_t*) (((uint32_t) pxTopOfStack) & (~((uint32_t) 7)));

    //拷贝TaskName
    for (UBaseType_t cnt = 0; cnt < (configMAX_TASK_NAME_LEN - 1); cnt++) {
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
