#ifndef BSP_I2C_H
#define BSP_I2C_H

#include "os.h"
#include "os_cfg_app.h"
#include <project.h>
#include <includes.h>
#include <stdint.h>
#include <log_task.h>
#include "I2C_1.h"
#include "Timer_1.h"
extern volatile CPU_INT32U current_timestamp;
/* Buffer size for I2C transactions */
#define I2C_BUFFER_SIZE 64u

/*
*********************************************************************************************************
*                                         Function Prototypes
*********************************************************************************************************
*/

/**
 * @brief Initializes the I2C task and starts the timer for timestamping.
 */
void I2C_Init(void);

/**
 * @brief Reads data from a specified I2C device and register.
 * 
 * @param addr  I2C address of the device
 * @param reg   Register address to read from
 * @param buf   Pointer to the buffer where data will be stored
 * @param len   Number of bytes to read
 * 
 * @return 0 on success, 1 on failure
 */
CPU_INT08U bsp_i2c_read(CPU_INT08U addr, CPU_INT08U reg, CPU_INT08U *buf, CPU_INT16U len);

/**
 * @brief Writes data to a specified I2C device and register.
 * 
 * @param addr  I2C address of the device
 * @param reg   Register address to write to
 * @param data  Pointer to the data to be written
 * @param len   Number of bytes to write
 * 
 * @return 0 on success, 1 on failure
 */
CPU_INT08U bsp_i2c_write(CPU_INT08U addr, CPU_INT08U reg, CPU_INT08U *data, CPU_INT16U len);

/**
 * @brief Timer ISR for incrementing timestamps used in the I2C task.
 */
CY_ISR_PROTO(Timer_ISR);

#endif /* BSP_I2C_H */
