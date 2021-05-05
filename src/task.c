/* Includes ------------------------------------------------------------------*/
#include "task.h"

/* typedef -------------------------------------------------------------------*/

/* define --------------------------------------------------------------------*/

/* macro ---------------------------------------------------------------------*/

#if 1 // Todo: 实现Cortex-M平台的优化算法
// 记录最高任务优先级
#define taskRECORD_READY_PRIORITY(uxPriority) do {  \
    if(uxTopReadyPriority < (uxPriority)) {         \
        uxTopReadyPriority = (uxPriority);          \
    }                                               \
} while (0)

// 选取最高优先级任务
#define taskSELECT_HIGHEST_PRIORITY_TASK()      do {              \
    UBaseType_t uxTopPriority = uxTopReadyPriority;               \
    /* 查找Task最高优先级 */                                           \
    while (listLIST_IS_EMPTY(&pxReadyTaskLists[uxTopPriority])) { \
        uxTopPriority--;                                          \
    }                                                             \
    /* 获取最高优先级Task */                                      \
    listGET_OWNER_OF_NEXT_ENTRY(pxCurrentTCB,                     \
        &pxReadyTaskLists[uxTopPriority]);                        \
    /* 更新Task最高优先级 */                                           \
    uxTopReadyPriority = uxTopPriority;                           \
} while (0)

// 重置最高优先级
#define taskRESET_READY_PRIORITY(uxPriority)

#else

#define taskRECORD_READY_PRIORITY(uxPriority)
#define taskSELECT_HIGHEST_PRIORITY_TASK()
#define taskRESET_READY_PRIORITY(uxPriority)

#endif

// 将任务插入到相应的就绪队列
#define prvAddTaskToReadyList(pxTCB)    do {                    \
    taskRECORD_READY_PRIORITY((pxTCB)->uxPriority);             \
    vListInsertEnd(&(pxReadyTaskLists[(pxTCB)->uxPriority]),    \
        &((pxTCB)->xStateListNode));                            \
} while (0)

// 交换延时任务队列
#define taskSWITCH_DELAYED_LIST()   do {            \
    List_t *pxTmp = pxDelayedTaskList;              \
    pxDelayedTaskList = pxOverflowDelayedTaskList;  \
    pxOverflowDelayedTaskList = pxTmp;              \
    xNumOfOverflows++;                              \
    prvResetNextTaskUnblockTime();                  \
} while (0)

/* variables -----------------------------------------------------------------*/
// 就绪列表
static List_t pxReadyTaskLists[configMAX_PRIORITIES] = { 0 };

// 记录当前就绪任务的最高优先级
static UBaseType_t uxTopReadyPriority = 0;

// IDLE Task任务TCB
static TCB_t xIdleTaskTCB = { 0 };

// IDLE Task任务栈
static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] = { 0 };

// 系统节拍计数器
static volatile TickType_t xTickCount = (TickType_t) configINITIAL_TICK_COUNT;

// 当前任务数量
static volatile UBaseType_t uxCurrentNumberOfTasks = 0;

// 延时任务队列，一个用于保存延时未溢出任务，另一个保存溢出
static List_t xDelayTaskList[2] = { { 0 }, { 0 } };

// 延时任务队列指针（延时未溢出）
static List_t *volatile pxDelayedTaskList = NULL;

// 延时任务队列指针（延时溢出）
static List_t *volatile pxOverflowDelayedTaskList = NULL;

// 系统节拍计数器溢出次数
static BaseType_t xNumOfOverflows = 0;

// 最近延时到期任务的系统节拍数
static volatile TickType_t xNextTaskUnblockTime = 0;

// 当前任务
TCB_t *volatile pxCurrentTCB = NULL;

/* function prototypes -------------------------------------------------------*/
// 初始化任务栈
static TaskHandle_t prvInitialiseNewTask(TaskFunction_t pxTaskCode,
                const portCHAR *const pcTaskName, const uint32_t ulStackDepth,
                void *const pvParameters, UBaseType_t uxPriority,
                TCB_t *const pxNewTCB);

// 初始化就绪列表
static void prvInitialiseTaskLists(void);

// 将新任务插入到就绪列表
static void prvAddNewTaskToReadyList(TCB_t *pxNewTCB);

// Idle Task Function
static void prvIdleTaskFunc(void *arg);

// 更新TaskUnblockTime
static void prvResetNextTaskUnblockTime(void);

// 将当前任务插入延时队列
static void prvAddCurrentTaskToDelayedList(TickType_t xTicksToDelay);

// 更新系统时基
BaseType_t xTaskIncrementTick();

// 任务切换
void vTaskSwitchContext(void);

/* user codes ----------------------------------------------------------------*/

static void prvIdleTaskFunc(void *arg) {
    while (1) {
        ; // Do nothing
    }
}

static void prvInitialiseTaskLists(void) {
    // 初始化就绪任务队列
    for (UBaseType_t cnt = 0; cnt < (UBaseType_t) configMAX_PRIORITIES; cnt++) {
        vListInit(&pxReadyTaskLists[cnt]);
    }

    // 初始化延时任务队列
    pxDelayedTaskList = &xDelayTaskList[0];
    pxOverflowDelayedTaskList = &xDelayTaskList[1];
    vListInit(pxDelayedTaskList);
    vListInit(pxOverflowDelayedTaskList);
}

static void prvAddNewTaskToReadyList(TCB_t *pxNewTCB) {
    taskENTER_CRITICAL();
    // 更新任务数量
    uxCurrentNumberOfTasks++;

    // 更新currentTCB指针
    if (NULL == pxCurrentTCB) {
        pxCurrentTCB = pxNewTCB;
        // 初始化就绪列表
        if ((BaseType_t) 1U == uxCurrentNumberOfTasks) {
            prvInitialiseTaskLists();
        }
    } else {
        if (pxCurrentTCB->uxPriority < pxNewTCB->uxPriority) {
            pxCurrentTCB = pxNewTCB;
        }
    }

    prvAddTaskToReadyList(pxNewTCB);
    taskEXIT_CRITICAL();
}

static TaskHandle_t prvInitialiseNewTask(TaskFunction_t pxTaskCode,
                const portCHAR *const pcTaskName, const uint32_t ulStackDepth,
                void *const pvParameters, UBaseType_t uxPriority,
                TCB_t *const pxNewTCB) {
    StackType_t *pxTopOfStack; //栈顶指针

    //计算栈顶（需要8字节对齐），这里默认栈向低地址增长
    pxTopOfStack = pxNewTCB->pxTack + (ulStackDepth - (uint32_t) 1);
    pxTopOfStack =
                    (StackType_t*) (((uint32_t) pxTopOfStack)
                                    & (~((uint32_t) 7)));

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

    //初始化优先级，这里会将非法的任务优先级统一设定成最高优先级！
    if (uxPriority >= (UBaseType_t) (configMAX_PRIORITIES)) {
        uxPriority = (UBaseType_t) (configMAX_PRIORITIES) - (UBaseType_t) 1U;
    }
    pxNewTCB->uxPriority = uxPriority;

    //初始化任务栈，为从中断返回做好准备
    pxNewTCB->pxTopOfStack = pxPortInitialiseStack(pxTopOfStack, pxTaskCode,
                    pvParameters);

    return (TaskHandle_t) pxNewTCB;

}

TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
                const portCHAR *const pcTaskName, const uint32_t ulStackDepth,
                void *const pvParameters, UBaseType_t uxPriority,
                StackType_t *const pxStackBuffer,
                TCB_t *const pxTaskBuffer) {

    TCB_t *pxNewTCB;
    TaskHandle_t xReturn;

    if ((NULL != pxTaskBuffer) && (NULL != pxStackBuffer)) {
        pxNewTCB = pxTaskBuffer;
        pxNewTCB->pxTack = pxStackBuffer;
        // 初始化新任务
        xReturn = prvInitialiseNewTask(pxTaskCode, pcTaskName, ulStackDepth,
                        pvParameters, uxPriority, pxNewTCB);
        // 加入到就绪列表
        if (NULL != xReturn) {
            prvAddNewTaskToReadyList(pxNewTCB);
        }
    } else {
        xReturn = NULL;
    }
    return xReturn;
}

void vTaskStartScheduler(void) {
    // 创建Idle Task
    xTaskCreateStatic((TaskFunction_t) prvIdleTaskFunc, "Idle Task",
    configMINIMAL_STACK_SIZE,
    NULL, 0, uxIdleTaskStack, &xIdleTaskTCB);

    // 更新系统节拍计数器
    xNextTaskUnblockTime = (TickType_t) portMAX_DELAY;
    xTickCount = (TickType_t) 0U;

    if (pdFALSE != xPortStartScheduler()) {
        ; // 如果调度器初始化成功，不会执行到这里
    }
}

void vTaskSwitchContext(void) {
    taskSELECT_HIGHEST_PRIORITY_TASK();
}

static void prvResetNextTaskUnblockTime() {
    if (listLIST_IS_EMPTY(pxDelayedTaskList)) {
        xNextTaskUnblockTime = portMAX_DELAY;
    }
    else {
        xNextTaskUnblockTime = listGET_LIST_ITEM_VALUE(
                        listGET_HEAD_ENTRY(pxDelayedTaskList));
    }
}

static void prvAddCurrentTaskToDelayedList(TickType_t xTicksToDelay) {
    const TickType_t xConstSysTick = xTickCount;
    const TickType_t xTimeToWait = xConstSysTick + xTicksToDelay;

    // 将当前任务从就绪队列删除，若删除后该队列为空，则重置uxTopReadyPriority
    if ((UBaseType_t) 0U == uxListRemove(&(pxCurrentTCB->xStateListNode))) {
        taskRESET_READY_PRIORITY();
    }

    // 更新任务节点Value
    listSET_LIST_ITEM_VALUE(&(pxCurrentTCB->xStateListNode), xTimeToWait);

    // 插入等待队列
    if (xTimeToWait >= xConstSysTick) {
        // 未溢出的情况
        vListInsert(pxDelayedTaskList, &(pxCurrentTCB->xStateListNode));
        if (xTimeToWait < xNextTaskUnblockTime) {
            xNextTaskUnblockTime = xTimeToWait;
        }
    } else {
        // 溢出的情况
        vListInsert(pxOverflowDelayedTaskList, &(pxCurrentTCB->xStateListNode));
    }

}

void vTaskDelay(const TickType_t xDelayTicks) {
    // xDelayTicks为0时只进行调度
    if (xDelayTicks > (TickType_t) 0) {
        prvAddCurrentTaskToDelayedList(xDelayTicks);
    }
    taskYIELD();
}

BaseType_t xTaskIncrementTick() {
    const TickType_t xConstTickCount = xTickCount + (TickType_t) 1;
    BaseType_t xSwitchReq = pdFALSE;

    // 更新系统节拍
    xTickCount = xConstTickCount;

    // 如果节拍溢出则切换延时队列
    if ((TickType_t) 0 == xConstTickCount) {
        taskSWITCH_DELAYED_LIST();
    }

    // 任务延时到期处理
    while (1) {
        if (listLIST_IS_EMPTY(pxDelayedTaskList)) {
            xNextTaskUnblockTime = portMAX_DELAY;
            break;
        }
        else {
            TCB_t *pxTaskIter = (TCB_t*) listGET_OWNER_OF_HEAD_ENTRY(
                            pxDelayedTaskList);
            TickType_t xTaskDelayTicks = (TickType_t) listGET_LIST_ITEM_VALUE(
                            &(pxTaskIter->xStateListNode));

            // 就绪所有到期的任务
            if (xTaskDelayTicks > xConstTickCount) {
                xNextTaskUnblockTime = xTaskDelayTicks;
                break;
            }
            uxListRemove(&(pxTaskIter->xStateListNode));
            prvAddTaskToReadyList(pxTaskIter);

            if (pxTaskIter->uxPriority >= pxCurrentTCB->uxPriority) {
                xSwitchReq = pdTRUE;
            }
        }
    }

    // 时间片切换，时间片等于SysTick的周期
    if (listGET_LIST_LENGTH(&pxReadyTaskLists[pxCurrentTCB->uxPriority])
                    > (UBaseType_t) 1U) {
        xSwitchReq = pdTRUE;
    }

    return xSwitchReq;
}
