#ifndef __PEANUT_PORT_H
#define __PEANUT_PORT_H

#include "port_macro.h"
#include "rtos_config.h"
#include "projdefs.h"
#include "list.h"
#include "task.h"

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR            (0x01000000UL)

/* For strict compliance with the Cortex-M spec the task start address should
 have bit-0 clear, as it is loaded into the PC on exit from an ISR. */
#define portSTART_ADDRESS_MASK      ((StackType_t)0xfffffffeUL)

// 系统中断优先级设置寄存器3
#define portSCB_SYSPRI3_REG         (*((volatile uint32_t*) 0xe000ed20))
// PENDSV中断的优先级
#define portSCB_PENDSV_PRI          (((uint32_t) configKERNEL_INTERRUPT_PRIORITY) << 16UL)
// SYSTICK中断的优先级
#define portSCB_SYSTICK_PRI         (((uint32_t) configKERNEL_INTERRUPT_PRIORITY) << 24UL)

// 中断控制和状态寄存器
#define portSCB_INT_CTR_STAT_REG    (*((volatile uint32_t*) 0xe000ed04))
#define portSCB_PENDSVSET_BIT       (1UL << 28UL)
#define portSCB_VECTACTIVE_MAS      (0x3FF)

// SysTick控制寄存器
#define portSTK_CTRL_REG            (*((volatile uint32_t*) 0xe000e010))
#define portSTK_CLK_SRC_BIT         (1UL << 2UL)
#define portSTK_CLK_INT_BIT         (1UL << 1UL)
#define portSTK_CLR_ENABLE_BIT      (1UL << 0UL)

// SysTick重载寄存器
#define portSTK_LOAD_REG            (*((volatile uint32_t*) 0xe000e014))

// 任务切换
portFORCE_INLINE static void portYIELD() {
    portSCB_INT_CTR_STAT_REG = portSCB_PENDSVSET_BIT;
    __asm volatile(
                    " dsb    \n"
                    " isb    \n"
                    :
                    :
                    : "memory"
    );
}

// 初始化任务栈
StackType_t* pxPortInitialiseStack(StackType_t *pxTopOfStack,
                TaskFunction_t pxCode, void *const pvParameters);

// 启动调度器
BaseType_t xPortStartScheduler(void);

#endif // __PEANUT_PORT_H
