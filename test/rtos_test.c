#include "main.h"
#include "task.h"

#define TASK1_STACK_SIZE    (128)
#define TASK2_STACK_SIZE    (128)

static StackType_t Task1Stack[TASK1_STACK_SIZE];
//static StackType_t Task2Stack[TASK2_STACK_SIZE];
TCB_t Task1TCB;
//TCB_t Task2TCB;
TaskHandle_t Task1Handle;

static uint32_t flg1;
//static uint32_t flg2;
//
extern void list_test(void);
static void Task1_Entry(void *pArg);
//static void Task2_Entry(void *pArg);

void Task1_Entry(void *pArg) {
    while (1) {
        HAL_Delay(1000);
        flg1 = ~flg1;
    }
}

//void Task2_Entry(void *pArg) {
//    while (1) {
//        __delay();
//        flg2 = ~flg2;
//    }
//}
//TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
//        const portCHAR *const pcTaskName, const uint32_t ulStackDepth,
//        void *const pvParameters, StackType_t *const pxStackBuffer,
//        TCB_t *const pxTaskBuffer);

void rtos_test(void) {
    Task1Handle = xTaskCreateStatic(Task1_Entry, "Test Task",
    TASK1_STACK_SIZE, (void*) 0xa5, Task1Stack, &Task1TCB);
//    list_test();
}
