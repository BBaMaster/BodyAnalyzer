#include "bsp_i2c.h"
#include "log_task.h"
#include "project.h"
#include "cyapicallbacks.h"
#include "I2C_1.h"

/* I2C Buffer */
CPU_INT08U i2c_rx_buffer[I2C_BUFFER_SIZE];

/* Task Control Block and Stack Declaration for I2C Task */
OS_TCB I2C_Task_TCB;
CPU_STK I2C_TaskStk[APP_CFG_TASK_I2C_STK_SIZE];
OS_SEM I2CTaskSem;  /* Semaphore to ensure I2C is initialized before usage */

/* Function Prototypes */
static void I2C_Task(void *p_arg);  /* Forward declaration for I2C Task */

/*
*********************************************************************************************************
*                                         I2C Initialization (Task Creation)
*********************************************************************************************************
*/

void I2C_Init(void) {
    OS_ERR os_err;

    Log_Write(LOG_LEVEL_DEBUG, "I2C Init: Creating I2C task", 0);

    /* Create the I2C task */
    OSTaskCreate((OS_TCB      *)&I2C_Task_TCB,
                 (CPU_CHAR    *)"I2C Task",
                 (OS_TASK_PTR  )I2C_Task,
                 (void        *)0,
                 (OS_PRIO      )APP_CFG_TASK_I2C_PRIO,
                 (CPU_STK     *)&I2C_TaskStk[0],
                 (CPU_STK_SIZE )APP_CFG_TASK_I2C_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE )APP_CFG_TASK_I2C_STK_SIZE,
                 (OS_MSG_QTY   )0u,
                 (OS_TICK      )0u,
                 (void        *)0,
                 (OS_OPT       )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR      *)&os_err);

    if (os_err == OS_ERR_NONE) {
        Log_Write(LOG_LEVEL_INFO, "I2C Task created successfully", 0);
    } else {
        Log_Write(LOG_LEVEL_ERROR, "Error: I2C Task creation failed", os_err);
    }
}

/*
*********************************************************************************************************
*                                         I2C Task Function
*********************************************************************************************************
*/

/* Inside I2C Task Function */
static void I2C_Task(void *p_arg) {
    OS_ERR os_err;
    (void)p_arg;

    Log_Write(LOG_LEVEL_DEBUG, "I2C Task: Starting I2C hardware initialization", 0);

    /* Initialize the I2C hardware and signal completion */
    I2C_1_Start();
    Log_Write(LOG_LEVEL_INFO, "I2C Task: I2C hardware started", 0);

    /* Check if the bus is busy */
    if (I2C_1_MasterStatus() & I2C_1_MSTAT_XFER_INP) {
        Log_Write(LOG_LEVEL_ERROR, "I2C Task: I2C bus is busy, initialization failed", 0);
        return;  // Exit task due to initialization error
    }

    Log_Write(LOG_LEVEL_DEBUG, "I2C Task: Posting semaphore for initialization completion", 0);

    /* Post the semaphore to indicate that the I2C task is initialized and ready */
    OSSemPost(&I2CTaskSem, OS_OPT_POST_1, &os_err);
    
    if (os_err == OS_ERR_NONE) {
        Log_Write(LOG_LEVEL_DEBUG, "I2C Task: Semaphore posted successfully, task ready", 0);
    } else {
        Log_Write(LOG_LEVEL_ERROR, "I2C Task: Error posting semaphore", os_err);
    }

    /* I2C task main loop */
    while (DEF_TRUE) {
        Log_Write(LOG_LEVEL_DEBUG, "I2C Task: Running main loop...", 0);
        /* Placeholder for actual I2C operations */
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
        //TODO send values to MSQ
        
        
        
        
        if (os_err != OS_ERR_NONE) {
            Log_Write(LOG_LEVEL_ERROR, "I2C Task delay error", os_err);
        }
    }
}

/*
*********************************************************************************************************
*                                         I2C Read Function
*********************************************************************************************************
*/

uint8_t bsp_i2c_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
    OS_ERR os_err;

    uint8_t status;

    /* Start communication and send the address + register */
    status = I2C_1_MasterSendStart(addr, I2C_1_WRITE_XFER_MODE);
    if (status != I2C_1_MSTR_NO_ERROR) {
        Log_Write(LOG_LEVEL_ERROR, "I2C Read Start Error", status);
        return 1;
    }

    I2C_1_MasterWriteByte(reg);

    /* Send a restart condition to switch to read mode */
    status = I2C_1_MasterSendRestart(addr, I2C_1_READ_XFER_MODE);
    if (status != I2C_1_MSTR_NO_ERROR) {
        UART_1_PutString("I2C Read Restart Error\r\n");
        Log_Write(LOG_LEVEL_ERROR, "I2C Read Restart Error", status);
        return 1;
    }

    /* Read the requested data */
    for (uint16_t i = 0; i < len; i++) {
        buf[i] = I2C_1_MasterReadByte((i == (len - 1)) ? I2C_1_NAK_DATA : I2C_1_ACK_DATA);
    }

    I2C_1_MasterSendStop();

    return 0;  // Success
}


/*
*********************************************************************************************************
*                                         I2C Write Function
*********************************************************************************************************
*/

uint8_t bsp_i2c_write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len) {
    OS_ERR os_err;
    UART_1_PutString("bsp_i2c_write");
    uint8_t status;

    /* Start communication and send the address */
    status = I2C_1_MasterSendStart(addr, I2C_1_WRITE_XFER_MODE);
    if (status != I2C_1_MSTR_NO_ERROR) {
        Log_Write(LOG_LEVEL_ERROR, "I2C Write Start Error", status);
        return 1;
    }

    /* Log register write */
    Log_Write(LOG_LEVEL_DEBUG, "I2C writing register", 0);

    /* Write the register address */
    I2C_1_MasterWriteByte(reg);

    /* Log data write progress */
    Log_Write(LOG_LEVEL_DEBUG, "I2C Write: Writing data...", 0);

    /* Write the data */
    for (uint16_t i = 0; i < len; i++) {
        I2C_1_MasterWriteByte(data[i]);
    }

    /* Stop the communication */
    I2C_1_MasterSendStop();


    return 0;  // Success
}

/* End of bsp_i2c.c */

