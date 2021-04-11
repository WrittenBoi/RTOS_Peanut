#include "port.h"

/*
 * Applied for ARM Cortex-M4(STM32F103)
 */

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR        (0x01000000UL)

/* For strict compliance with the Cortex-M spec the task start address should
 have bit-0 clear, as it is loaded into the PC on exit from an ISR. */
#define portSTART_ADDRESS_MASK  ((StackType_t)0xfffffffeUL)
#define portSCB_SYSPRI3_REG     (*((volatile uint32_t*) 0xe000ed20))
#define portNVIC_PENDSV_PRI     (((uint32_t) configKERNEL_INTERRUPT_PRIORITY) << 16UL)
#define portNVIC_SYSTICK_PRI    (((uint32_t) configKERNEL_INTERRUPT_PRIORITY) << 24UL)

// 默认Task不能返回
static void prvTaskExitError(void);

// 启动第一个Task，执行后不再返回
static void prvStartFirstTask(void) __attribute__ (( naked ));

static void prvTaskExitError(void) {
    while (1) {
        ;
    }
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
    portSCB_SYSPRI3_REG |= portNVIC_PENDSV_PRI;
    portSCB_SYSPRI3_REG |= portNVIC_SYSTICK_PRI;

// 启动第一个Task后，不再返回
    prvStartFirstTask();

    return pdFALSE;
}
