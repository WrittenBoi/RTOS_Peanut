#include "port.h"

/*
 * Applied for ARM Cortex-M3(STM32F103)
 */

// 临界区嵌套计数器
static UBaseType_t uxCriticalNestingCnt = 0;

// 默认Task不能返回
static void prvTaskExitError(void);

// SysTick初始化
static void vPortSetupTimerInterrupt(void);

// 系统中断Handler, 根据需要声明为naked
void xPortPendSVHandler(void) __attribute__ (( naked ));
void xPortSysTickHandler(void);
void vPortSVCHandler(void) __attribute__ (( naked ));

// 启动第一个Task，执行后不再返回
static void prvStartFirstTask(void) __attribute__ (( naked ));

static void prvTaskExitError(void) {
    while (1) {
        ;
    }
}

static void vPortSetupTimerInterrupt(void) {
    // 重新装载计数器
    portSTK_LOAD_REG = (configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;

    // 1. 设置SYSTICK时钟等于内核时钟
    // 2. 使能SYSTICK中断
    // 3. 开始计数
    portSTK_CTRL_REG = (portSTK_CLK_SRC_BIT | portSTK_CLK_INT_BIT
                    | portSTK_CLR_ENABLE_BIT);
}

void xPortPendSVHandler(void) {
    __asm volatile(
                    " mrs r0, psp                           \n" /* 获取PSP指针（旧Task） */
                    " isb                                   \n"
                    "                                       \n"
                    " ldr r3, pxCurrentTCBConst             \n" /* 获取CurrentTCB指针 */
                    " ldr r2, [r3]                          \n" /* 获取CurrentTCB栈顶 */
                    "                                       \n"
                    " stmdb r0!, {r4-r11}                   \n" /* 保存r4-r11 */
                    " str r0, [r2]                          \n" /* 更新CurrentTCB栈顶 */
                    "                                       \n"
                    " stmdb sp!, {r3, r14}                  \n" /* r3,r14压栈，准备调用TaskSwitchContext */
                    "                                       \n"
                    " mov r0, %0                            \n" /* 禁中断 */
                    " msr basepri, r0                       \n"
                    " dsb                                   \n"
                    " isb                                   \n"
                    "                                       \n"
                    " bl vTaskSwitchContext                 \n" /* 调用TaskSwitchContext，进行任务切换 */
                    "                                       \n"
                    " mov r0, #0                            \n" /* 开中断 */
                    " msr basepri, r0                       \n"
                    "                                       \n"
                    " ldmia sp!, {r3, r14}                  \n" /* r3,r14出栈 */
                    "                                       \n"
                    " ldr r1, [r3]                          \n" /* 取得更新后的CurrentTCB指针 */
                    " ldr r0, [r1]                          \n" /* 取得更新后的CurrentTCB栈顶 */
                    " ldmia r0!, {r4-r11}                   \n" /* 恢复r4-r11 */
                    " msr psp, r0                           \n" /* 设定新Task的栈顶 */
                    " isb                                   \n"
                    "                                       \n"
                    " bx r14                                \n" /* 从中断返回 */
                    "                                       \n"
                    " .align 4                              \n"
                    "pxCurrentTCBConst: .word pxCurrentTCB  \n"
                    ::"i"(configMAX_SYSCALL_INTERRUPT_PRIORITY)
    );
}

void vPortSVCHandler(void) {
    __asm volatile(
                    " ldr r0, pxCurrentTCBConst2                        \n" /* 取得指向pxCurrentTCB的地址指针 */
                    " ldr r0, [r0]                                      \n" /* 获取pxCurrentTCB地址 */
                    " ldr r0, [r0]                                      \n" /* 取得栈顶指针 */
                    " ldmia r0!, {r4-r11}                               \n" /* 恢复r4-r11 */
                    " msr psp, r0                                       \n" /* 更新当前栈顶到PSP */
                    " isb                                               \n"
                    " mov r0, #0                                        \n" /* 清空R0 */
                    " msr basepri, r0                                   \n" /* 打开所有中断 */
                    " orr r14, #0xD                                     \n" /* 0xD=1101, 返回线程模式和线程栈 */
                    " bx r14                                            \n"
                    " .align 4                                          \n"
                    "pxCurrentTCBConst2: .word pxCurrentTCB             \n"
    );
}

void xPortSysTickHandler(void) {
    uint32_t ulIntStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    if (pdTRUE == xTaskIncrementTick()) {
        taskYIELD();
    }
    portCLR_INTERRUPT_MASK_FROM_ISR(ulIntStatus);
}

static void prvStartFirstTask(void) {
    __asm volatile(
                    " ldr r0, =0xE000ED08   \n" /* Use the NVIC offset register to locate the stack. */
                    " ldr r0, [r0]          \n"
                    " ldr r0, [r0]          \n"
                    " msr msp, r0           \n" /* Set the msp back to the start of the stack. */
                    " cpsie i               \n" /* Globally enable interrupts. */
                    " cpsie f               \n"
                    " dsb                   \n"
                    " isb                   \n"
                    " svc 0                 \n" /* System call to start first task. */
                    " nop                   \n"
    );
}

StackType_t* pxPortInitialiseStack(StackType_t *pxTopOfStack,
                TaskFunction_t pxCode, void *const pvParameters) {
    pxTopOfStack--;
// xPSR设定， bit24(T)必须置1
    *pxTopOfStack = portINITIAL_XPSR;
    pxTopOfStack--;

// 返回地址设定，最低位必须是0
    *pxTopOfStack = ((StackType_t) pxCode) & portSTART_ADDRESS_MASK;
    pxTopOfStack--;

// LR设定
    *pxTopOfStack = (StackType_t) prvTaskExitError;

// R12, R3-R1设定
    pxTopOfStack -= 5;

// R0(函数参数设定）
    *pxTopOfStack = (StackType_t) pvParameters;

// R11, R10, R9, R8, R7, R6, R5 and R4
    pxTopOfStack -= 8;

    return pxTopOfStack;
}

BaseType_t xPortStartScheduler(void) {
    // 设置PENDSV和SYSTICK中断为最低优先级
    portSCB_SYSPRI3_REG |= portSCB_PENDSV_PRI;
    portSCB_SYSPRI3_REG |= portSCB_SYSTICK_PRI;

    // 开启SYSTICK
    vPortSetupTimerInterrupt();

    // 启动第一个Task后，不再返回
    prvStartFirstTask();

    return pdFALSE;
}

void vPortEnterCritical() {
    portDISABLE_INTERRUPTS();
    uxCriticalNestingCnt++;
    if (uxCriticalNestingCnt >= 1) {
        configASSERT(0 == (portSCB_INT_CTR_STAT_REG & portSCB_VECTACTIVE_MAS));
    }
}

void vPortExitCritical() {
    uxCriticalNestingCnt--;
    if (uxCriticalNestingCnt == 0) {
        portENABLE_INTERRUPTS();
    }
}
