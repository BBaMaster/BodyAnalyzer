/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include <os.h>
#include "bsp_pwm.h"
#include "log_task.h"
//#include "data_processing_task.h"


#include <Pin_1.h>
#include <Pin_2.h>
#include <push_button.h>

#include <Pin_Blue_Led.h>
#include <Pin_Green_Led.h>
#include <Pin_Red_Led.h>

#define HIGH 1
#define LOW 0

#define PORT1 (1<<0)
#define PORT2 (1<<1)

#define PIN_BLUE_LED  (1<<1)
#define PIN_GREEN_LED (1<<2)
#define PIN_RED_LED   (1<<7)

#define P1_6 (1<<6)
#define P2_0 (1<<0)
#define P2_1 (1<<1)
#define P2_2 (1<<2)


// Knopf-Task
OS_TCB BUT_Task_TCB;
//CPU_STK BUT_TaskStk[APP_CFG_TASK_BUT_STK_SIZE];
static void BUT_Task(void *p_arg);

// LEDs
OS_TCB LedR_Task_TCB;
CPU_STK LedR_TaskStk[APP_CFG_TASK_LED_STK_SIZE];
static void LedR_Task(void *p_arg);

OS_TCB LedG_Task_TCB;
CPU_STK LedG_TaskStk[APP_CFG_TASK_LED_STK_SIZE];
static void LedG_Task(void *p_arg);

OS_TCB LedB_Task_TCB;
CPU_STK LedB_TaskStk[APP_CFG_TASK_LED_STK_SIZE];
static void LedB_Task(void *p_arg);

CPU_INT08U Sauerstoffumrechnung(CPU_INT08U s);
CPU_INT08U Herzschlag(CPU_INT08U h);
CPU_INT08U Passtalles(CPU_INT08U p);

CPU_VOID init_gpio(CPU_VOID);
CPU_INT08U gpio_high(CPU_INT08U port, CPU_INT08U pin);
CPU_INT08U gpio_low(CPU_INT08U port, CPU_INT08U pin);
CPU_INT08S gpio_read(CPU_INT08U port, CPU_INT08U pin);

/* [] END OF FILE */
