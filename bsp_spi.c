/*
*********************************************************************************************************
*
*                                      SPI BSP - Implementation File
*
*********************************************************************************************************
*/

#include "bsp_spi.h"
#include "log_task.h"


/* SPI Buffer */
CPU_INT08U spi_rx_buffer[SPI_BUFFER_SIZE];

/* Task Control Block and Stack Declaration for SPI Task */
OS_TCB SPI_Task_TCB;
CPU_STK SPI_TaskStk[APP_CFG_TASK_SPI_STK_SIZE];

/*
*********************************************************************************************************
*                                         SPI Initialization
*********************************************************************************************************
*/

void SPI_Init(void) {
    OS_ERR os_err;
    

    /* Create SPI Task */
    OSTaskCreate((OS_TCB      *)&SPI_Task_TCB,
                 (CPU_CHAR    *)"SPITask",
                 (OS_TASK_PTR  )SPI_Task,
                 (void        *)0,
                 (OS_PRIO      )APP_CFG_TASK_SPI_PRIO,
                 (CPU_STK     *)&SPI_TaskStk[0],
                 (CPU_STK_SIZE )APP_CFG_TASK_SPI_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE )APP_CFG_TASK_SPI_STK_SIZE,
                 (OS_MSG_QTY   )0u,
                 (OS_TICK      )0u,
                 (void        *)0,
                 (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR      *)&os_err);

    /* Log the initialization */
    Log_Write(LOG_LEVEL_INFO, "SPI Initialized", 0);
}

/*
*********************************************************************************************************
*                                         SPI Task Function
*********************************************************************************************************
*/

void SPI_Task(void *p_arg) {
    OS_ERR os_err;
    (void)p_arg;
    
    /* Start the SPI hardware block */
    SPIM_1_Start();  

    while (DEF_TRUE) {

        /* Log task cycle */
        Log_Write(LOG_LEVEL_INFO, "SPI Task Cycle Completed", 0);
        //TODO Send data to MSQ
        /* Delay between cycles */
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
    }
}



/*
*********************************************************************************************************
*                                         SPI Read
*********************************************************************************************************
*/

void SPI_Read(void) {
    

    /* Log the read operation */
    Log_Write(LOG_LEVEL_INFO, "SPI Read Started", 0);
}

/*
*********************************************************************************************************
*                                         SPI Write
*********************************************************************************************************
*/

void SPI_Write(CPU_INT08U* data, CPU_INT16U size) {
    
    /* Implementation for SPI Write - To be defined based on actual project needs */
}
