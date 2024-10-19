#ifndef BSP_I2C_H
#define BSP_I2C_H

// Include necessary headers
#include "os.h"
#include "os_cfg_app.h"
#include <project.h>
#include <includes.h>
#include <stdint.h>
#include <max30102_interface.h>
#include "I2C_1.h"
#include "Timer_1.h"
#include <log_task.h>

// Threshold constants
#define INITIAL_THRESHOLD           1500000 // Initial threshold for contact detection
#define DEBOUNCE_TIME_MS           400      // Minimum time between heartbeats in milliseconds

// Sizing constants
#define MAX_HISTORY_SIZE            20        // Maximum number of heart rates to store for averaging
#define FIR_COEFF_SIZE             12       // FIR filter coefficients size
#define CIRCULAR_BUFFER_SIZE        32       // Size for FIR filter circular buffer
#define BUFFER_SIZE                 128      // Size for UART output buffer
#define HR_BUFFER_SIZE              64       // Size for heart rate output buffer
#define POLLING_DELAY_MS           38       // Delay for polling in milliseconds

// Initial values for heart rate detection variables
#define INITIAL_IR_AC_MAX          20       // Maximum IR AC signal
#define INITIAL_IR_AC_MIN         -20       // Minimum IR AC signal
#define INITIAL_LAST_PEAK_TIMESTAMP 0       // Initial value for last peak timestamp
#define INITIAL_IR_AC_SIGNAL_CURRENT 0       // Initial current AC signal value
#define INITIAL_IR_AC_SIGNAL_PREVIOUS 0      // Initial previous AC signal value
#define INITIAL_IR_AVG_REG         0         // Initial running average of DC component
#define INITIAL_HEART_RATE_SUM     0         // Initial running sum for moving average
#define INITIAL_HEART_RATE_INDEX    0        // Initial index for circular buffer

/* Buffer size for I2C data */
#define I2C_BUFFER_SIZE             64u

/* Function prototypes */
void I2C_Init(void);                      
CPU_INT08U bsp_i2c_read(CPU_INT08U addr, CPU_INT08U reg, CPU_INT08U *buf, CPU_INT16U len);  /* I2C read */
CPU_INT08U bsp_i2c_write(CPU_INT08U addr, CPU_INT08U reg, CPU_INT08U *data, CPU_INT16U len); /* I2C write */
CPU_INT32S mul16(CPU_INT16S x, CPU_INT16S y);
CPU_INT16S lowPassFIRFilter(CPU_INT16S din);
CPU_INT16S averageDCEstimator(CPU_INT32S *p, CPU_INT32U x);

#endif /* BSP_I2C_H */
