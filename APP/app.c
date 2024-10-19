/*
*********************************************************************************************************
*
*                                       MES1 Embedded Software (RTOS)
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : Beneder Roman
                  
*********************************************************************************************************
*/

#include <project.h>
#include <includes.h>
#include "log_task.h"  /* Include the log task header */
#include "max30102_interface.h"
#include <max30102/driver_max30102.h>
/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/

#define  USE_BSP_TOGGLE                             1u

/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/
static OS_TCB App_TaskStartTCB;
static CPU_STK App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE];

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void App_TaskStart(void *p_arg);

/*
*********************************************************************************************************
*                                                main()
*********************************************************************************************************
*/

int main(void)
{
    OS_ERR os_err;

    BSP_PreInit();
    CPU_Init();
    /* Init OS */
    OSInit(&os_err);

    /* Start OS multitasking */
    OSTaskCreate((OS_TCB      *)&App_TaskStartTCB,              
                 (CPU_CHAR    *)"Start",
                 (OS_TASK_PTR  )App_TaskStart, 
                 (void        *)0,
                 (OS_PRIO      )APP_CFG_TASK_START_PRIO,
                 (CPU_STK     *)&App_TaskStartStk[0],
                 (CPU_STK_SIZE )APP_CFG_TASK_START_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE )APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY   )0u,
                 (OS_TICK      )0u,
                 (void        *)0,
                 (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR      *)&os_err);

    OSStart(&os_err);  /* Start multitasking */
}


/*
*********************************************************************************************************
*                                          App_TaskStart()
*********************************************************************************************************
*/
static void App_TaskStart(void *p_arg) {
    OS_ERR err;
    (void)p_arg;
    max30102_handle_t max30102_handle;
    uint16_t raw_temp;
    float temp_celsius;
    BSP_PostInit();
    BSP_CPU_TickInit();
    
    /* Create the semaphore */
    OSSemCreate(&LogTaskSem, "Log Task Semaphore", 0, &err);
    
    Log_Init();    /* Initializes and creates the log task */

    /* Wait for the log task to signal that it's ready */
    OSSemPend(&LogTaskSem, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
    
    Log_Write(LOG_LEVEL_INFO, "App_TaskStart: Log task initialized successfully", 0);

    I2C_Init();    /* Initializes and creates the I2C task */
    OSSemPend(&I2CTaskSem, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_STRICT, &err); // Add a delay before sensor initialization
    if (max30102_sensor_init(&max30102_handle) == 0) {
        Log_Write(LOG_LEVEL_INFO, "MAX30102 sensor initialized successfully", 0);
    } else {
        Log_Write(LOG_LEVEL_ERROR, "MAX30102 sensor initialization failed", 0);
        return;  
    }//*/
    
    while (DEF_TRUE) {
        if (max30102_read(&max30102_handle, &raw_temp, &temp_celsius) == 0) {
            char log_message[LOG_MSG_SIZE];
            snprintf(log_message, LOG_MSG_SIZE, "Temperature: %.2f°C (raw: %u)", temp_celsius, raw_temp);
            Log_Write(LOG_LEVEL_INFO, log_message, 0);
        } else {
            Log_Write(LOG_LEVEL_ERROR, "Failed to read temperature from MAX30102", 0);
        }
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

