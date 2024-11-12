#ifndef GPIO_TASKS_H
#define GPIO_TASKS_H

#include "log_task.h"                // Logging functionality
#include "data_processing_task.h"     // Data processing task definitions
#include <heartbeatSensor.h>          // Heartbeat sensor functionality
#include "bsp_pwm.h"                  // PWM functions
#include "includes.h"                 // General includes for the project
#include <cpu.h>                      // CPU-specific functions
#include <environmentSensor.h>        // Environment sensor definitions

// GPIO definitions
#define P1_2 (1 << 2)                 // Bit mask for P1_2 GPIO pin

// External toggle state variable, modified by the button task
extern CPU_BOOLEAN toggle_state;

// Function Prototypes
void init_gpio(void);                 // Initializes GPIOs, PWM, and creates tasks

// Button Task
OS_TCB BUT_Task_TCB;           // Task Control Block for button task
CPU_STK BUT_TaskStk[APP_CFG_TASK_BUTTON_STK_SIZE];
static void BUT_Task(void *p_arg);    // Button Task function prototype

// Red/Blue LED Task
OS_TCB LedRB_Task_TCB;         // Task Control Block for Red/Blue LED task
CPU_STK LedRB_TaskStk[APP_CFG_TASK_LED_STK_SIZE];
static void LedRB_Task(void *p_arg);  // Red/Blue LED Task function prototype

// Green LED Task
OS_TCB LedG_Task_TCB;          // Task Control Block for Green LED task
CPU_STK LedG_TaskStk[APP_CFG_TASK_LED_STK_SIZE];
static void LedG_Task(void *p_arg);   // Green LED Task function prototype

#endif // GPIO_TASKS_H

/* [] END OF FILE */
