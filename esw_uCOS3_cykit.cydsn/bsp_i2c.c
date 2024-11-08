#include "bsp_i2c.h"
/*
*********************************************************************************************************
*                                         Timer ISR
*********************************************************************************************************
*/
// Timer ISR to increment timestamp
CY_ISR(Timer_ISR) {
    current_timestamp++;
}

/*
*********************************************************************************************************
*                                         I2C Initialization (Task Creation)
*********************************************************************************************************
*/
void I2C_Init(void) {
    OS_ERR os_err;
    
    Log_Write(LOG_LEVEL_I2C, "Starting I2C hardware initialization", 0);
    I2C_1_Start();
    Log_Write(LOG_LEVEL_I2C, "I2C hardware started", 0);

    
    // Start the timer and enable its ISR to track time for I2C operations
    Timer_1_Start();
    Timer_ISR_StartEx(Timer_ISR);
}

/*
*********************************************************************************************************
*                                         I2C Read Function
*********************************************************************************************************
*/
CPU_INT08U bsp_i2c_read(CPU_INT08U addr, CPU_INT08U reg, CPU_INT08U *buf, CPU_INT16U len) {
    CPU_INT08U status;

    // Start I2C communication in write mode to send the register address
    status = I2C_1_MasterSendStart(addr, I2C_1_WRITE_XFER_MODE);
    if (status != I2C_1_MSTR_NO_ERROR) {
        Log_Write(LOG_LEVEL_ERROR, "I2C Read Start Error", status); // Log error if start fails
        return 1; // Exit with error
    }

    // Send the register address to the device
    I2C_1_MasterWriteByte(reg);

    // Restart I2C in read mode to read data from the device
    status = I2C_1_MasterSendRestart(addr, I2C_1_READ_XFER_MODE);
    if (status != I2C_1_MSTR_NO_ERROR) {
        Log_Write(LOG_LEVEL_ERROR, "I2C Read Restart Error", status); // Log error if restart fails
        return 1; // Exit with error
    }

    // Read the specified number of bytes from the device
    for (CPU_INT16U i = 0; i < len; i++) {
        // Read a byte with ACK for all but the last byte, which uses NAK
        buf[i] = I2C_1_MasterReadByte((i == (len - 1)) ? I2C_1_NAK_DATA : I2C_1_ACK_DATA);
    }

    // End the I2C transaction
    I2C_1_MasterSendStop();
    return 0; // Success
}

/*
*********************************************************************************************************
*                                         I2C Write Function
*********************************************************************************************************
*/
CPU_INT08U bsp_i2c_write(CPU_INT08U addr, CPU_INT08U reg, CPU_INT08U *data, CPU_INT16U len) {
    CPU_INT08U status;

    // Start I2C communication in write mode to send data to the device
    status = I2C_1_MasterSendStart(addr, I2C_1_WRITE_XFER_MODE);
    if (status != I2C_1_MSTR_NO_ERROR) {
        Log_Write(LOG_LEVEL_ERROR, "I2C Write Start Error", status); // Log error if start fails
        return 1; // Exit with error
    }

    // Send the register address to the device
    I2C_1_MasterWriteByte(reg);

    // Write the specified number of bytes to the device
    for (CPU_INT16U i = 0; i < len; i++) {
        I2C_1_MasterWriteByte(data[i]); // Write each byte in the data array
    }

    // End the I2C transaction
    I2C_1_MasterSendStop();
    return 0; // Success
}
