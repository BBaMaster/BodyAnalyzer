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

#ifndef GPIO_TASKS_H
#define GPIO_TASKS_H

#include "bsp_pwm.h"
#include "log_task.h"
#include "data_processing_task.h"

// Button-Task
OS_TCB BUT_Task_TCB;
CPU_STK BUT_TaskStk[APP_CFG_TASK_BUTTON_STK_SIZE];
static void BUT_Task(void *p_arg);
static CPU_BOOLEAN button_variable;

// LEDs
OS_TCB LedRB_Task_TCB;
CPU_STK LedRB_TaskStk[APP_CFG_TASK_LED_STK_SIZE];
static void LedRB_Task(void *p_arg);
OS_TCB LedG_Task_TCB;
CPU_STK LedG_TaskStk[APP_CFG_TASK_LED_STK_SIZE];
static void LedG_Task(void *p_arg);

CPU_INT08U allCorrCalc(CPU_INT08U h);  

CPU_VOID init_gpio(CPU_VOID);

#endif


/* [] END OF FILE */
