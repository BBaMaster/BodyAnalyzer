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
    CyGlobalIntEnable; // Enable global interrupts
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

    
    BSP_PostInit();
    BSP_CPU_TickInit();
    
    /* Create the semaphore */
    OSSemCreate(&LogTaskSem, "Log Task Semaphore", 0, &err);
    
    Log_Init();    /* Initializes and creates the log task */

    /* Wait for the log task to signal that it's ready */
    OSSemPend(&LogTaskSem, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
    
    Log_Write(LOG_LEVEL_INFO, "App_TaskStart: Log task initialized successfully");

    I2C_Init();    /* Initializes and creates the I2C task */

    while (DEF_TRUE) {
        /* Delay before next read */
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}


