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
// SPI Read Function
static int8_t bsp_spi_read(uint8_t *reg_addr, uint8_t *data, uint32_t len) {
    // Pull CS low to select the BME280
    SPIM_1_WriteTxData(*reg_addr | 0x80); // Send register address with read flag

    // Read data
    for (uint32_t i = 0; i < len; i++) {
        data[i] = SPIM_1_ReadRxData(); // Read data from RX buffer
    }

    // Pull CS high to deselect the BME280
    return 0; // Return success
}

// SPI Write Function
static int8_t bsp_spi_write(uint8_t *reg_addr, const uint8_t *data, uint32_t len) {
    // Pull CS low to select the BME280
    SPIM_1_WriteTxData(*reg_addr & 0x7F); // Send register address without read flag

    // Write data
    for (uint32_t i = 0; i < len; i++) {
        SPIM_1_WriteTxData(data[i]); // Write data to TX buffer
    }

    // Pull CS high to deselect the BME280
    return 0; // Return success
}
