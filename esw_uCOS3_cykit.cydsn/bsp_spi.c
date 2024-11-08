/*
*********************************************************************************************************
*
*                                      SPI BSP - Implementation File
*
*********************************************************************************************************
*/

#include "bsp_spi.h"
#include "log_task.h"
#include "SPIM_1.h"  // Include your SPI component header for required functions and definitions


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
    Log_Write(LOG_LEVEL_SPI, "Creating SPI task...", 0);
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
    Log_Write(LOG_LEVEL_SPI, "SPI Initialized", 0);
}

/*
*********************************************************************************************************
*                                         SPI Task Function
*********************************************************************************************************
*/

void SPI_Task(void *p_arg) {
    OS_ERR os_err;
    (void)p_arg;
    struct bme280_dev dev;
    
    /* Start the SPI hardware block */
    SPIM_1_Start();  
    CS_1_Write(1);
    #if defined(SPIM_1_RX_SOFTWARE_BUF_ENABLED)
    Log_Write(LOG_LEVEL_INFO, "SPI using software buffer for RX");
    #else
    Log_Write(LOG_LEVEL_INFO, "SPI using hardware FIFO for RX");
    #endif
    Log_Write(LOG_LEVEL_SPI, "Initializing BME280 sensor...", 0);
    read_bme280_calibration_data();
    read_bme280_chip_id2();
    
    while (DEF_TRUE) {
        // Log task cycle
        // Delay between cycles
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &os_err);
    }
}

/*
*********************************************************************************************************
*                                         SPI Write
*********************************************************************************************************
*/

int8_t bsp_spi_write(uint8_t reg_addr, const uint8_t *data, uint32_t len) {
    Log_Write(LOG_LEVEL_SPI, "Starting SPI write to register: 0x%02X", reg_addr);
    
    // Assert CS to select the slave device
    CS_1_Write(0);

    // Send the register address with MSB cleared (for write operation)
    SPIM_1_WriteTxData(reg_addr & 0x7F);

    // Send the data bytes
    for (uint32_t i = 0; i < len; i++) {
        while (!(SPIM_1_ReadTxStatus() & SPIM_1_STS_TX_FIFO_NOT_FULL)) {
            // Wait until TX FIFO is not full
        }
        SPIM_1_WriteTxData(data[i]);  // Send data byte
        Log_Write(LOG_LEVEL_SPI, "Wrote byte[%d]: 0x%02X", i, data[i]);
    }

    // Wait for SPI transmission to complete
    while (!(SPIM_1_ReadTxStatus() & SPIM_1_STS_SPI_DONE)) {
        // Wait for the transaction to finish
    }

    // De-assert CS to deselect the slave device
    CS_1_Write(1);

    Log_Write(LOG_LEVEL_SPI, "SPI write completed successfully", 0);
    return BME280_INTF_RET_SUCCESS;
}

/*
*********************************************************************************************************
*                                         SPI Read
*********************************************************************************************************
*/
int8_t bsp_spi_read(uint8_t reg_addr, uint8_t *data, uint32_t len) {
    Log_Write(LOG_LEVEL_SPI, "Starting SPI read from register: 0x%02X", reg_addr);
    Log_Write(LOG_LEVEL_SPI, "Length of data to read: %d", len);

    // Assert CS to select the slave device
    Log_Write(LOG_LEVEL_SPI, "Asserting CS line to 0");
    CS_1_Write(0);

    // Send the register address with the MSB set to indicate a read
    Log_Write(LOG_LEVEL_SPI, "Writing register address 0x%02X with read bit set", reg_addr | 0x80);
    while (!(SPIM_1_ReadTxStatus() & SPIM_1_STS_TX_FIFO_NOT_FULL)) {
        // Wait until TX FIFO is not full
    }
    SPIM_1_WriteTxData(reg_addr | 0x80);

    // Wait for the TX FIFO to send the register address
    Log_Write(LOG_LEVEL_SPI, "Waiting for TX to complete for the register address");
    while (!(SPIM_1_ReadTxStatus() & SPIM_1_STS_SPI_DONE)) {
        // Wait for the SPI transaction to complete
    }

    // Debugging TX status before sending dummy data
    Log_Write(LOG_LEVEL_SPI, "TX status after sending register address: 0x%02X", SPIM_1_ReadTxStatus());

    // Add a delay to ensure the sensor is ready to respond (if necessary)
    OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, 0);

    // Send dummy data and read the sensor's response
    for (uint32_t i = 0; i < len; i++) {
        Log_Write(LOG_LEVEL_SPI, "Reading byte %d from sensor", i);

        // Send dummy data to generate clock pulses for reading
        while (!(SPIM_1_ReadTxStatus() & SPIM_1_STS_TX_FIFO_NOT_FULL)) {
            // Wait until TX FIFO is not full
        }
        Log_Write(LOG_LEVEL_SPI, "Sending dummy byte (0xFF) to generate clock pulses");
        SPIM_1_WriteTxData(0xFF);

        // Wait for RX FIFO to receive the response
        while (!(SPIM_1_ReadRxStatus() & SPIM_1_STS_RX_FIFO_NOT_EMPTY)) {
            // Wait for RX FIFO to contain data
        }

        // Read the data from the RX FIFO
        data[i] = SPIM_1_ReadRxData();
        Log_Write(LOG_LEVEL_SPI, "Read byte[%d]: 0x%02X", i, data[i]);
    }

    // De-assert CS to deselect the slave device
    Log_Write(LOG_LEVEL_SPI, "De-asserting CS line");
    CS_1_Write(1);

    Log_Write(LOG_LEVEL_SPI, "SPI read completed successfully");
    return BME280_INTF_RET_SUCCESS;
}


/*
*********************************************************************************************************
*                                         Delay Function
*********************************************************************************************************
*/

void Delay_MS(uint32_t ms) {
    OS_ERR os_err;
    Log_Write(LOG_LEVEL_SPI, "Delaying for %u milliseconds", ms);
    OSTimeDlyHMSM(0, 0, 0, ms, OS_OPT_TIME_HMSM_STRICT, &os_err);
}

/*
*********************************************************************************************************
*                                      Read BME280 Chip ID
*********************************************************************************************************
*/

int8_t read_bme280_chip_id2() {
    uint8_t chip_id = 0;
    
    // Soft reset the BME280 first
    uint8_t reset_cmd = 0xB6;
    bsp_spi_write(0xE0, &reset_cmd, 1);  // Soft reset the sensor
    Delay_MS(5);  // Delay for sensor reset

    // Read the Chip ID from register 0xD0
    if (bsp_spi_read(0xD0, &chip_id, 1) != BME280_INTF_RET_SUCCESS) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to read BME280 chip ID");
        return -1;
    }
    
    Log_Write(LOG_LEVEL_INFO, "BME280 Chip ID: 0x%02X", chip_id);
    
    if (chip_id != 0x60) {
        Log_Write(LOG_LEVEL_ERROR, "Invalid BME280 Chip ID: Expected 0x60, got 0x%02X", chip_id);
        return -4;  // Error code for invalid chip ID
    }
    
    return BME280_INTF_RET_SUCCESS;
}

/*
*********************************************************************************************************
*                                      Read BME280 Calibration Data
*********************************************************************************************************
*/

int8_t read_bme280_calibration_data() {
    uint8_t calib_data[3];

    // Read 3 bytes of calibration data from register 0x88
    if (bsp_spi_read(0x88, calib_data, 3) != BME280_INTF_RET_SUCCESS) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to read BME280 calibration data");
        return -1;
    }

    // Log the calibration data
    Log_Write(LOG_LEVEL_INFO, "BME280 Calibration Data: 0x%02X, 0x%02X, 0x%02X",
              calib_data[0], calib_data[1], calib_data[2]);

    return BME280_INTF_RET_SUCCESS;
}
