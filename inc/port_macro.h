#ifndef __PEANUT_PORT_MACRO_H
#define __PEANUT_PORT_MACRO_H

#include <stdio.h>
#include <stdint.h>

#include <rtos_config.h>

#if 0
#define portCHAR char
#define portLONG long
#define portSHORT short
#define portSTACK_TYPE uint32_t
#define portBASE_TYPE long
#define portUBASE_TYPE unsigned long
#else
typedef char portCHAR;
typedef long portLONG;
typedef short portSHORT;
typedef long portBASE_TYPE;
typedef unsigned long portUBASE_TYPE;
typedef uint32_t portSTACK_TYPE;
#endif

typedef portSTACK_TYPE StackType_t;
typedef portBASE_TYPE BaseType_t;
typedef portUBASE_TYPE UBaseType_t;

#define portMAX_DELAY   ((TickType_t)0xffffffffUL)
typedef uint32_t TickType_t;

#define portINLINE  __inline
#define portFORCE_INLINE inline __attribute__(( always_inline))

// 无中断保护的中断屏蔽函数
#define portDISABLE_INTERRUPTS()            vPortRaiseBASEPRI()
#define portENABLE_INTERRUPTS()             vPortSetBASEPRI(0)

// 有中断保护的中断屏蔽函数
#define portSET_INTERRUPT_MASK_FROM_ISR()   ulPortRaiseBASEPRI()
#define portCLR_INTERRUPT_MASK_FROM_ISR(x)  vPortSetBASEPRI((x))

// 无保护可嵌套的临界区保护函数
extern void vPortEnterCritical();
extern void vPortExitCritical();
#define portENTER_CRITICAL()                vPortEnterCritical()
#define portEXIT_CRITICAL()                 vPortExitCritical()

portFORCE_INLINE static void vPortRaiseBASEPRI() {
    __asm volatile(
            " mov r0, %0            \n" //将RTOS管理的最高优先级保存至r0
            " msr basepri, r0       \n" //设置basepri，屏蔽中断
            " dsb                   \n"
            " isb                   \n"
            :
            : "i" (configMAX_SYSCALL_INTERRUPT_PRIORITY)
            : "memory"
    );
}

portFORCE_INLINE static uint32_t ulPortRaiseBASEPRI() {
    uint32_t ulReturn = 0;
    __asm volatile(
            " mrs %0, basepri       \n"
            " mov r0, %1            \n"
            " msr basepri, r0       \n"
            " dsb                   \n"
            " isb                   \n"
            : "=r" (ulReturn)
            : "i" (configMAX_SYSCALL_INTERRUPT_PRIORITY)
            : "memory"
    );
    return ulReturn;
}

portFORCE_INLINE static void vPortSetBASEPRI(uint32_t ulNewBasepri) {
    __asm volatile(
            " msr basepri, %0       \n"
            :
            : "r" (ulNewBasepri)
            : "memory"
            );
}

#endif // __PEANUT_PORT_MACRO_H
