#include "heartbeatSensor.h"

/* I2C Buffer */
CPU_INT08U i2c_rx_buffer[I2C_BUFFER_SIZE]; // Buffer for incoming I2C data

/* Heart rate data storage */
CPU_INT32S heart_rate_history[MAX_HISTORY_SIZE] = {0}; // Circular buffer for storing heart rate values
CPU_INT32U adaptive_threshold = INITIAL_THRESHOLD;     // Threshold to detect presence based on sensor input

/* Heart Rate Variables */
volatile CPU_INT32U current_timestamp = 0;             // Global timestamp for time tracking
CPU_INT32U last_peak_timestamp = INITIAL_LAST_PEAK_TIMESTAMP; // Timestamp of last detected peak
CPU_INT32S IR_AC_Max = INITIAL_IR_AC_MAX;              // Maximum AC signal value
CPU_INT32S IR_AC_Min = INITIAL_IR_AC_MIN;              // Minimum AC signal value
CPU_INT32S IR_AC_Signal_Current = INITIAL_IR_AC_SIGNAL_CURRENT; // Current AC signal value
CPU_INT32S IR_AC_Signal_Previous = INITIAL_IR_AC_SIGNAL_PREVIOUS; // Previous AC signal value
CPU_INT32S ir_avg_reg = INITIAL_IR_AVG_REG;            // Average DC signal value for AC baseline
CPU_INT32S heart_rate_sum = INITIAL_HEART_RATE_SUM;    // Running sum of heart rate values for averaging
CPU_INT32U heart_rate_index = INITIAL_HEART_RATE_INDEX; // Index for circular buffer

/* FIR Coefficients for Low Pass Filter */
static const CPU_INT16U FIRCoeffs[FIR_COEFF_SIZE] = {172, 321, 579, 927, 1360, 1858, 2390, 2916, 3391, 3768, 4012, 4096};

/* Task Control Block and Stack Declaration for MAX30102 Task */
OS_TCB MAX30102_Task_TCB;                              // Task Control Block for MAX30102 task
CPU_STK MAX30102_TaskStk[APP_CFG_TASK_MAX30102_STK_SIZE]; // Stack for MAX30102 task

/*
*********************************************************************************************************
*                                         MAX30102 Task Initialization (Task Creation)
*********************************************************************************************************
* @brief Initializes the MAX30102 task and creates it within the RTOS.
*
* This function initializes and creates the task responsible for handling data from the MAX30102 sensor.
* The task will continuously check for available samples, process the data, and calculate heart rate.
*********************************************************************************************************
*/
void MAX30102_Init(void) {
    OS_ERR os_err;

    Log_Write(LOG_LEVEL_I2C, "MAX30102 Init: Creating MAX30102 task", 0);

    /* Create the MAX30102 task */
    OSTaskCreate((OS_TCB *)&MAX30102_Task_TCB,           // Task Control Block
                 (CPU_CHAR *)"MAX30102 Task",            // Task name for debugging
                 (OS_TASK_PTR)max30102_Task,             // Pointer to task function
                 (void *)0,                              // Task arguments (none)
                 (OS_PRIO)APP_CFG_TASK_I2C_PRIO,         // Task priority
                 (CPU_STK *)&MAX30102_TaskStk[0],        // Stack base
                 (CPU_STK_SIZE)APP_CFG_TASK_MAX30102_STK_SIZE_LIMIT, // Minimum stack size
                 (CPU_STK_SIZE)APP_CFG_TASK_MAX30102_STK_SIZE,        // Stack size
                 (OS_MSG_QTY)0u,                         // No message queue required
                 (OS_TICK)0u,                            // No time slice
                 (void *)0,                              // No extra storage
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), // Stack options
                 (OS_ERR *)&os_err);                     // Error code

    // Log task creation success or failure
    if (os_err == OS_ERR_NONE) {
        Log_Write(LOG_LEVEL_I2C, "MAX30102 Task created successfully", 0);
    } else {
        Log_Write(LOG_LEVEL_ERROR, "Error: MAX30102 Task creation failed", os_err);
    }
}

/*
*********************************************************************************************************
*                                         MAX30102 Task Function
*********************************************************************************************************
* @brief Task that reads from the MAX30102 sensor, processes signals, and calculates heart rate.
*
* This task continuously polls the MAX30102 sensor for new data, filters the IR signal, and detects
* heart rate peaks based on signal characteristics.
*********************************************************************************************************
*/
static void max30102_Task(void *p_arg) {
    OS_ERR os_err;
    (void)p_arg; // Suppress unused parameter warning

    max30102_handle_t max30102_handle;   // Handle for interacting with the MAX30102 sensor
    uint32_t raw_red = 0, raw_ir = 0;    // Raw readings for red and IR channels
    CPU_INT08U data_len = 2;             // Expected number of samples

    // Initialize the MAX30102 sensor
    if (max30102_sensor_init(&max30102_handle) != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to initialize MAX30102 sensor", 0);
        return; // Exit if initialization fails
    }

    while (DEF_TRUE) {
        CPU_BOOLEAN samples_available = MAX30102_BOOL_FALSE;

        // Check if new samples are available from the sensor
        max30102_get_interrupt(&max30102_handle, MAX30102_INTERRUPT_PPG_RDY_EN, &samples_available);

        // Delay to prevent excessive polling
        OSTimeDlyHMSM(0, 0, 0, POLLING_DELAY_MS, OS_OPT_TIME_HMSM_STRICT, &os_err);

        // Continue loop if no samples are available
        if (samples_available != MAX30102_BOOL_TRUE) {
            Log_Write(LOG_LEVEL_I2C, "No new samples available to read", 0);
            continue;
        }

        // Read data from the MAX30102 sensor
        CPU_INT08U res = max30102_read(&max30102_handle, &raw_red, &raw_ir, &data_len);
        if (res != 0) {
            Log_Write(LOG_LEVEL_ERROR, "Failed to read from MAX30102", res);
            continue;
        }

        // Check if current readings exceed the threshold for valid data
        if (raw_red <= adaptive_threshold || raw_ir <= adaptive_threshold) {
            IR_AC_Signal_Previous = 0; // Reset signal if contact is lost
            continue;
        }

        // Calculate the filtered IR AC signal
        IR_AC_Signal_Current = raw_ir - averageDCEstimator(&ir_avg_reg, raw_ir);
        IR_AC_Signal_Current = lowPassFIRFilter(IR_AC_Signal_Current);

        // Detect heartbeat based on zero-crossing in filtered signal
        CPU_BOOLEAN is_zero_crossing = (IR_AC_Signal_Previous < 0 && IR_AC_Signal_Current >= 0) ||
                                (IR_AC_Signal_Previous > 0 && IR_AC_Signal_Current <= 0);
        IR_AC_Signal_Previous = IR_AC_Signal_Current; // Update previous signal value

        if (!is_zero_crossing) {
            continue; // Skip if no zero-crossing detected
        }

        // Ensure that AC signal variation is within expected range
        if ((IR_AC_Max - IR_AC_Min) <= 20 || (IR_AC_Max - IR_AC_Min) >= 1000) {
            continue;
        }

        // Calculate time since last peak
        CPU_INT32U elapsed_time = current_timestamp - last_peak_timestamp;
        if (elapsed_time <= DEBOUNCE_TIME_MS) {
            continue; // Skip if debounce period has not passed
        }

        last_peak_timestamp = current_timestamp;

        // Calculate beats per minute (BPM) based on elapsed time
        CPU_INT32S heart_rate_bpm = (60000 / elapsed_time);

        // Update circular buffer and running sum for moving average
        heart_rate_sum -= heart_rate_history[heart_rate_index];
        heart_rate_history[heart_rate_index] = heart_rate_bpm;
        heart_rate_sum += heart_rate_bpm;
        heart_rate_index = (heart_rate_index + 1) % MAX_HISTORY_SIZE;

        // Calculate the average heart rate
        CPU_INT32S average_heart_rate = heart_rate_sum / MAX_HISTORY_SIZE;
        Log_Write(LOG_LEVEL_I2C, "Average Heart Rate: %d BPM", average_heart_rate);

        // Update threshold based on average heart rate
        adaptive_threshold = average_heart_rate * 100;
    }
}

/*
*********************************************************************************************************
*                                         DC Estimator Function
*********************************************************************************************************
* @brief Estimates the DC component of the IR signal.
*
* This function applies a low-pass filter to estimate the DC component of the IR signal.
* Used to subtract DC baseline from the raw signal to extract the AC component.
*
* @param p  Pointer to the running DC average
* @param x  New IR sample
* @return Filtered DC component
*********************************************************************************************************
*/
CPU_INT16S averageDCEstimator(CPU_INT32S *p, CPU_INT32U x) {
    *p += ((((CPU_INT32S)x << 15) - *p) >> 4); // Update average with new sample
    return (*p >> 15); // Return estimated DC component
}

/*
*********************************************************************************************************
*                                         FIR Low Pass Filter
*********************************************************************************************************
* @brief Applies a low-pass FIR filter to the AC component of the signal.
*
* This function filters the AC component to smooth the signal and remove high-frequency noise.
*
* @param din Input AC signal value
* @return Filtered AC component
*********************************************************************************************************
*/
CPU_INT16S lowPassFIRFilter(CPU_INT16S din) {  
    static CPU_INT16S cbuf[CIRCULAR_BUFFER_SIZE]; // Circular buffer for filter input
    static CPU_INT08U offset = 0;                 // Current position in circular buffer
    CPU_INT32S z = 0;                             // Filtered output

    // Store current sample in buffer
    cbuf[offset] = din;

    // Apply FIR filter coefficients to circular buffer
    z += mul16(FIRCoeffs[FIR_COEFF_SIZE - 1], cbuf[(offset - FIR_COEFF_SIZE + 1) & (CIRCULAR_BUFFER_SIZE - 1)]);
    for (CPU_INT08U i = 0; i < FIR_COEFF_SIZE - 1; i++) {
        z += mul16(FIRCoeffs[i], cbuf[(offset - i) & (CIRCULAR_BUFFER_SIZE - 1)]);
    }

    // Increment and wrap offset
    offset = (offset + 1) % CIRCULAR_BUFFER_SIZE;

    return (z >> 15); // Return filtered value
}

/*
*********************************************************************************************************
*                                         Integer Multiplication Function
*********************************************************************************************************
* @brief Multiplies two 16-bit integers and returns a 32-bit result.
*
* Used in FIR filter calculations.
*
* @param x First operand
* @param y Second operand
* @return Product of x and y
*********************************************************************************************************
*/
CPU_INT32S mul16(CPU_INT16S x, CPU_INT16S y) {
    return ((CPU_INT32S)x * (CPU_INT32S)y);
}
