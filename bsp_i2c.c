#include "bsp_i2c.h"


/* I2C Buffer */
CPU_INT08U i2c_rx_buffer[I2C_BUFFER_SIZE];
CPU_INT32S heart_rate_history[MAX_HISTORY_SIZE] = {0}; // Circular buffer for heart rate history
CPU_INT32U heart_rate_count = 0; // Number of heart rates recorded
CPU_INT32U adaptive_threshold = INITIAL_THRESHOLD; // Adaptive threshold for contact detection

/* Task Control Block and Stack Declaration for I2C Task */
OS_TCB I2C_Task_TCB;
CPU_STK I2C_TaskStk[APP_CFG_TASK_I2C_STK_SIZE];

/* Heart Rate Variables */
CPU_INT32U last_peak_timestamp = INITIAL_LAST_PEAK_TIMESTAMP; // Timestamp of last detected peak
CPU_INT32S IR_AC_Max = INITIAL_IR_AC_MAX; // Maximum IR AC signal
CPU_INT32S IR_AC_Min = INITIAL_IR_AC_MIN; // Minimum IR AC signal
CPU_INT32S IR_AC_Signal_Current = INITIAL_IR_AC_SIGNAL_CURRENT; // Current AC signal value
CPU_INT32S IR_AC_Signal_Previous = INITIAL_IR_AC_SIGNAL_PREVIOUS; // Previous AC signal value
CPU_INT32S ir_avg_reg = INITIAL_IR_AVG_REG; // Running average of DC component
CPU_INT32S heart_rate_sum = INITIAL_HEART_RATE_SUM; // Running sum for moving average
CPU_INT32U heart_rate_index = INITIAL_HEART_RATE_INDEX; // Current index for circular buffer


/* FIR Coefficients for Low Pass Filter */
static const CPU_INT16U FIRCoeffs[FIR_COEFF_SIZE] = {172, 321, 579, 927, 1360, 1858, 2390, 2916, 3391, 3768, 4012, 4096};

/* Function Prototypes */
static void I2C_Task(void *p_arg);  /* I2C Task Function */

/* Timer Variables */
volatile CPU_INT32U current_timestamp = 0;  // Variable to hold the current timestamp

// Timer ISR to increment timestamp
CY_ISR(Timer_ISR) {
    current_timestamp++;  // Increment the timestamp every millisecond
}

/*
*********************************************************************************************************
*                                         I2C Initialization (Task Creation)
*********************************************************************************************************
*/
void I2C_Init(void) {
    OS_ERR os_err;
    Log_Write(LOG_LEVEL_DEBUG, "I2C Init: Creating I2C task", 0);

    /* Create the I2C task */
    OSTaskCreate((OS_TCB *)&I2C_Task_TCB,
                 (CPU_CHAR *)"I2C Task",
                 (OS_TASK_PTR)I2C_Task,
                 (void *)0,
                 (OS_PRIO)APP_CFG_TASK_I2C_PRIO,
                 (CPU_STK *)&I2C_TaskStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_I2C_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE)APP_CFG_TASK_I2C_STK_SIZE,
                 (OS_MSG_QTY)0u,
                 (OS_TICK)0u,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&os_err);

    if (os_err == OS_ERR_NONE) {
        Log_Write(LOG_LEVEL_INFO, "I2C Task created successfully", 0);
    } else {
        Log_Write(LOG_LEVEL_ERROR, "Error: I2C Task creation failed", os_err);
    }
    
    // Start the timer and enable its ISR
    Timer_1_Start();
    Timer_ISR_StartEx(Timer_ISR);
}

/*
*********************************************************************************************************
*                                         I2C Task Function
*********************************************************************************************************
*/
static void I2C_Task(void *p_arg) {
    OS_ERR os_err;
    (void)p_arg; // Prevent unused parameter warning
    max30102_handle_t max30102_handle;
    CPU_INT32U raw_red = 0, raw_ir = 0; // Raw readings from the sensor
    CPU_INT08U data_len = 2;  // Number of samples to read

    Log_Write(LOG_LEVEL_DEBUG, "I2C Task: Starting I2C hardware initialization", 0);

    // Initialize I2C hardware
    I2C_1_Start();
    Log_Write(LOG_LEVEL_INFO, "I2C Task: I2C hardware started", 0);
    
    // Initialize the MAX30102 sensor
    if (max30102_sensor_init(&max30102_handle) != 0) {
        UART_1_PutString("Failed to initialize MAX30102 sensor\n");
        return;  // Exit if sensor initialization fails
    }

    // Main task loop
    while (DEF_TRUE) {
        max30102_bool_t samples_available = MAX30102_BOOL_FALSE;
        max30102_get_interrupt(&max30102_handle, MAX30102_INTERRUPT_PPG_RDY_EN, &samples_available);
        
        // Small delay to prevent excessive polling
        OSTimeDlyHMSM(0, 0, 0, POLLING_DELAY_MS, OS_OPT_TIME_HMSM_STRICT, &os_err); // Delay for a defined period

        if (samples_available == MAX30102_BOOL_TRUE) {
            // Read data from MAX30102
            CPU_INT08U res = max30102_read(&max30102_handle, &raw_red, &raw_ir, &data_len);
            if (res == 0) {
                // Check for contact
                if (raw_red > adaptive_threshold && raw_ir > adaptive_threshold) {
                
                    // Process the current IR value to detect heartbeat
                    IR_AC_Signal_Current = raw_ir - averageDCEstimator(&ir_avg_reg, raw_ir);
                    IR_AC_Signal_Current = lowPassFIRFilter(IR_AC_Signal_Current);

                    // Detect heartbeat based on changes in the signal
                    if ((IR_AC_Signal_Previous < 0 && IR_AC_Signal_Current >= 0) || 
                        (IR_AC_Signal_Previous > 0 && IR_AC_Signal_Current <= 0)) {
                        
                        // Ensure significant change in AC signal
                        if ((IR_AC_Max - IR_AC_Min) > 20 && (IR_AC_Max - IR_AC_Min) < 1000) {
                            CPU_INT32U elapsed_time = current_timestamp - last_peak_timestamp; // In milliseconds
                            if (elapsed_time > DEBOUNCE_TIME_MS) { // Debounce period
                                last_peak_timestamp = current_timestamp;

                                // Calculate BPM
                                CPU_INT32S heart_rate_bpm = (60000 / elapsed_time); // Calculate BPM
                                heart_rate_sum -= heart_rate_history[heart_rate_index]; // Remove the oldest reading
                                heart_rate_history[heart_rate_index] = heart_rate_bpm; // Add new reading
                                heart_rate_sum += heart_rate_bpm; // Update running sum
                                heart_rate_index = (heart_rate_index + 1) % MAX_HISTORY_SIZE; // Circular index

                                // Calculate and print average heart rate
                                CPU_INT32S average_heart_rate = heart_rate_sum / MAX_HISTORY_SIZE;
                                CPU_INT08U hr_buffer[HR_BUFFER_SIZE];
                                Log_Write(LOG_LEVEL_DEBUG, "Average Heart Rate: %d BPM\n", average_heart_rate);
                                /*
                                !
                                !
                                !
                                !
                                PUT THE MESSAGE QUEUE SEND HERE (average_heart_rate)
                                !
                                !
                                !
                                !
                                */
                                // Adjust adaptive threshold based on average heart rate
                                adaptive_threshold = average_heart_rate * 100;
                            }
                        }
                    } 
                    
                    // Store current readings for the next iteration
                    IR_AC_Signal_Previous = IR_AC_Signal_Current; // Update previous signal
                } else {
                    // No contact detected; reset previous values
                    IR_AC_Signal_Previous = 0;
                }
            } else {
                UART_1_PutString("Failed to read from MAX30102\n");
                Log_Write(LOG_LEVEL_ERROR, "Failed to read from MAX30102", res);
            }
        } else {
            UART_1_PutString("No new samples available to read\n");
            Log_Write(LOG_LEVEL_WARNING, "No new samples available to read", 0);
        }
    }
}
/* Average DC Estimator */
CPU_INT16S averageDCEstimator(CPU_INT32S *p, CPU_INT32U x) {
    *p += ((((CPU_INT32S)x << 15) - *p) >> 4); // Update the running average with new sample
    return (*p >> 15); // Return the estimated average
}

/* Low Pass FIR Filter */
CPU_INT16S lowPassFIRFilter(CPU_INT16S din) {  
    static CPU_INT16S cbuf[CIRCULAR_BUFFER_SIZE]; // Circular buffer for FIR filter
    static CPU_INT08U offset = 0; // Current index for the circular buffer
    CPU_INT32S z = 0; // Accumulated result for FIR filtering

    // Store the current input value in the circular buffer
    cbuf[offset] = din;

    // Apply the FIR filter coefficients to the circular buffer
    z += mul16(FIRCoeffs[FIR_COEFF_SIZE - 1], cbuf[(offset - FIR_COEFF_SIZE + 1) & (CIRCULAR_BUFFER_SIZE - 1)]);
    
    // Calculate the filtered output by summing up the products
    for (CPU_INT08U i = 0; i < FIR_COEFF_SIZE - 1; i++) {
        z += mul16(FIRCoeffs[i], cbuf[(offset - i) & (CIRCULAR_BUFFER_SIZE - 1)]);
    }

    // Increment the offset for the next sample and wrap around
    offset++;
    offset %= CIRCULAR_BUFFER_SIZE; // Wrap condition for the circular buffer

    return (z >> 15); // Return the filtered value
}

// Integer multiplier function for FIR calculations
CPU_INT32S mul16(CPU_INT16S x, CPU_INT16S y) {
    return ((CPU_INT32S)x * (CPU_INT32S)y); // Perform multiplication and return result
}

/* I2C Read Function */
CPU_INT08U bsp_i2c_read(CPU_INT08U addr, CPU_INT08U reg, CPU_INT08U *buf, CPU_INT16U len) {
    CPU_INT08U status;
    status = I2C_1_MasterSendStart(addr, I2C_1_WRITE_XFER_MODE); // Initiate I2C start condition
    if (status != I2C_1_MSTR_NO_ERROR) {
        Log_Write(LOG_LEVEL_ERROR, "I2C Read Start Error", status);
        return 1; // Return error code if starting failed
    }

    I2C_1_MasterWriteByte(reg); // Send the register address to read from

    status = I2C_1_MasterSendRestart(addr, I2C_1_READ_XFER_MODE); // Restart for reading
    if (status != I2C_1_MSTR_NO_ERROR) {
        Log_Write(LOG_LEVEL_ERROR, "I2C Read Restart Error", status);
        return 1; // Return error code if restarting failed
    }

    // Read bytes from the I2C device
    for (CPU_INT16U i = 0; i < len; i++) {
        buf[i] = I2C_1_MasterReadByte((i == (len - 1)) ? I2C_1_NAK_DATA : I2C_1_ACK_DATA); // Read bytes with ACK or NACK
    }

    I2C_1_MasterSendStop(); // End the I2C transaction
    return 0; // Success
}

/* I2C Write Function */
CPU_INT08U bsp_i2c_write(CPU_INT08U addr, CPU_INT08U reg, CPU_INT08U *data, CPU_INT16U len) {
    CPU_INT08U status;
    status = I2C_1_MasterSendStart(addr, I2C_1_WRITE_XFER_MODE); // Initiate I2C start condition
    if (status != I2C_1_MSTR_NO_ERROR) {
        Log_Write(LOG_LEVEL_ERROR, "I2C Write Start Error", status);
        return 1; // Return error code if starting failed
    }

    I2C_1_MasterWriteByte(reg); // Send the register address to write to

    // Write bytes to the I2C device
    for (CPU_INT16U i = 0; i < len; i++) {
        I2C_1_MasterWriteByte(data[i]); // Write each byte
    }

    I2C_1_MasterSendStop(); // End the I2C transaction
    return 0; // Success
}