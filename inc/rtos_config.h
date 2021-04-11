#ifndef __PEANUT_RTOS_CONFIG_H
#define __PEANUT_RTOS_CONFIG_H

// 最长任务名
#define configMAX_TASK_NAME_LEN                     (16)

// 静态分配系统资源
#define configSUPPORT_STATIC_ALLOCATION             (1)

// 优先级个数
#define configMAX_PRIORITIES                        (5)

/* Cortex-M specific definitions. */
#define configPRIO_BITS                             (4)

/* The lowest interrupt priority that can be used in a call to a "set priority"
 function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY     (15)

/* The highest interrupt priority that can be used by any interrupt service
 routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
 INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
 PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* Interrupt priorities used by the kernel port layer itself.  These are generic
 to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY             \
        (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define xPortPendSVHandler                          PendSV_Handler
#define xPortSysTickHandler                         SysTick_Handler
#define vPortSVCHandler                             SVC_Handler

#endif // __PEANUT_RTOS_CONFIG_H
