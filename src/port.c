#include "port.h"

/*
 * Applied for ARM Cortex-M4(STM32F103)
 */

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR        (0x01000000UL)

/* For strict compliance with the Cortex-M spec the task start address should
 have bit-0 clear, as it is loaded into the PC on exit from an ISR. */
#define portSTART_ADDRESS_MASK  ((StackType_t)0xfffffffeUL)

static void prvTaskExitError(void);

static void prvTaskExitError(void) {
    while (1) {
        ;
    }
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
