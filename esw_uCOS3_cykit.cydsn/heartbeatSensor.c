#include "heartbeatSensor.h"

/* Function Prototypes */
void MAX30102_Init(void);
static void max30102_Task(void *p_arg);
void processSignals(CPU_INT32U raw_ir, CPU_INT32U raw_red);
void updateHeartRate(void);
void updateSpO2(void);
void enqueueHeartRate(CPU_INT32S heart_rate);
void enqueueSpO2(CPU_INT32S spO2);
CPU_INT32S calculateSpO2(CPU_INT32S ac_red, CPU_INT32S dc_red, CPU_INT32S ac_ir, CPU_INT32S dc_ir);
CPU_INT16S averageDCEstimator(CPU_INT32S *p, CPU_INT32U x);
CPU_INT16S lowPassFIRFilter(CPU_INT16S din);
CPU_INT32S mul16(CPU_INT16S x, CPU_INT16S y);

/* I2C Buffer */
CPU_INT08U i2c_rx_buffer[I2C_BUFFER_SIZE]; // Buffer for incoming I2C data

/* Heart Rate and SpO2 Variables */
CPU_INT32U last_peak_timestamp = 0;                    // Timestamp of last detected positive crossing
CPU_INT32S IR_AC_Max = 0;                              // Maximum AC signal value for peak detection
CPU_INT32S IR_AC_Min = 0;                              // Minimum AC signal value for peak detection
CPU_INT32S IR_AC_Signal_Current = 0;                   // Current AC signal value
CPU_INT32S IR_AC_Signal_Previous = 0;                  // Previous AC signal value
CPU_INT32S IR_DC_Signal_Current = 0;                   // Current DC signal value
CPU_INT32S RED_AC_Signal_Current = 0;                  // Current AC signal value for red channel
CPU_INT32S RED_DC_Signal_Current = 0;                  // Current DC signal value for red channel
CPU_INT32S ir_avg_reg = 0;                             // Average DC signal value for IR baseline
CPU_INT32S red_avg_reg = 0;                            // Average DC signal value for red channel baseline
CPU_INT32S spO2 = -1;                                  // SpO2 result
CPU_BOOLEAN cycle_started = DEF_FALSE;                 // Track the start of a cycle

/* Heart rate history */
#define MAX_HEART_RATE_HISTORY 15
CPU_INT32S heart_rate_history[MAX_HEART_RATE_HISTORY] = {0};
CPU_INT32U heart_rate_index = 0;
CPU_INT32S heart_rate_sum = 0;



/* Task Control Block and Stack Declaration for MAX30102 Task */
OS_TCB MAX30102_Task_TCB;                              // Task Control Block for MAX30102 task
CPU_STK MAX30102_TaskStk[APP_CFG_TASK_MAX30102_STK_SIZE]; // Stack for MAX30102 task

/* Initialize the message queue to process send the heart rate & SpO2 values to*/
void initializeMessageQueueOximiter(){

  OS_ERR   os_err;
  
  OSQCreate(&CommQI2CHeartRate, "Message Queue to send heart rate from I2C as Message", MESSAGE_QUEUE_SIZE, &os_err);
  if (os_err == OS_ERR_NONE) {
        Log_Write(LOG_LEVEL_I2C, "I2C Task: I2C Message queue created successfully", 0);
    } else {
        Log_Write(LOG_LEVEL_ERROR, "Error: I2C Message Queue creation failed", os_err);
   }
    
  OSQCreate(&CommQI2CSPO2, "Message Queue to send SPO2 from I2C as Message", MESSAGE_QUEUE_SIZE, &os_err);
  if (os_err == OS_ERR_NONE) {
        Log_Write(LOG_LEVEL_I2C, "I2C Task: I2C Message queue created successfully", 0);
    } else {
        Log_Write(LOG_LEVEL_ERROR, "Error: I2C Message Queue creation failed", os_err);
   }
}

/**
 * @brief Initializes the MAX30102 task and creates it in the RTOS.
 */
void MAX30102_Init(void) {
    OS_ERR os_err;

    Log_Write(LOG_LEVEL_I2C, "MAX30102 Init: Creating MAX30102 task", 0);

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

    if (os_err == OS_ERR_NONE) {
        Log_Write(LOG_LEVEL_I2C, "MAX30102 Task created successfully", 0);
    } else {
        Log_Write(LOG_LEVEL_ERROR, "Error: MAX30102 Task creation failed", os_err);
    }
}

/**
 * @brief Main task for handling MAX30102 sensor data acquisition and processing.
 * 
 * This function continuously reads raw data from the MAX30102 sensor, filters it,
 * and processes the signal data if the values exceed a specified threshold.
 * It also implements a delay to control the polling rate.
 * 
 * @param p_arg Pointer to task arguments (unused in this task).
 */
static void max30102_Task(void *p_arg) {
    OS_ERR os_err; // Variable to store error status of OS operations
    (void)p_arg;   // Suppress unused parameter warning

    max30102_handle_t max30102_handle; // Sensor handle for managing sensor operations
    uint32_t raw_red = 0, raw_ir = 0;  // Variables to store raw red and IR readings
    CPU_INT08U data_len = 2;           // Expected number of samples to read from the sensor

    // Initialize the MAX30102 sensor
    // If initialization fails, log an error message and exit the task
    if (max30102_sensor_init(&max30102_handle) != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to initialize MAX30102 sensor", 0);
        return;
    }
    
    Log_Write(LOG_LEVEL_I2C, "Creating message queues...", 0);
    initializeMessageQueueOximiter();

    // Infinite loop for continuous sensor data reading and processing
    while (DEF_TRUE) {
        if (toggle_state) {
        // Attempt to read raw red and IR data from the MAX30102 sensor
        // If the read fails, log an error and continue to the next iteration
        if (max30102_read(&max30102_handle, &raw_red, &raw_ir, &data_len) != 0) {
            Log_Write(LOG_LEVEL_ERROR, "Failed to read from MAX30102", 0);
            continue;
        }
        //Log_Write(LOG_LEVEL_ERROR, "Red: %d, IR: %d",raw_red, raw_ir);

        // Check if the raw IR and red values exceed the threshold (100000)
        // If they do, process the signals for further analysis
        if (raw_ir > INITIAL_THRESHOLD && raw_red > INITIAL_THRESHOLD) {
            processSignals(raw_ir, raw_red);
        }

        // Introduce a delay to control the sensor polling rate
        // POLLING_DELAY_MS defines the delay duration in milliseconds
        OSTimeDlyHMSM(0, 0, 0, POLLING_DELAY_MS, OS_OPT_TIME_HMSM_STRICT, &os_err);
        }
}
}
/**
 * @brief Processes raw IR and red signals to update DC and AC components.
 * 
 * This function estimates the DC (direct current or baseline) component for both IR and red signals 
 * using a DC estimator and removes it to isolate the AC (alternating current) component. The AC component
 * represents pulsatile blood flow. The function then filters these AC signals to smooth them and calls
 * functions to calculate heart rate and SpO₂.
 * 
 * @param raw_ir Raw infrared signal value from the MAX30102 sensor
 * @param raw_red Raw red signal value from the MAX30102 sensor
 */
void processSignals(CPU_INT32U raw_ir, CPU_INT32U raw_red) {
    // Estimate the DC component for the IR signal and update the current IR DC signal
    IR_DC_Signal_Current = averageDCEstimator(&ir_avg_reg, raw_ir);

    // Estimate the DC component for the red signal and update the current red DC signal
    RED_DC_Signal_Current = averageDCEstimator(&red_avg_reg, raw_red);

    // Subtract the DC component from the raw IR signal to get the AC component
    // Apply a low-pass filter to the resulting AC component for the IR channel
    IR_AC_Signal_Current = lowPassFIRFilter(raw_ir - IR_DC_Signal_Current);

    // Subtract the DC component from the raw red signal to get the AC component
    // Apply a low-pass filter to the resulting AC component for the red channel
    RED_AC_Signal_Current = lowPassFIRFilter(raw_red - RED_DC_Signal_Current);
    
    
    // Call function to analyze IR signal and detect heart rate
    updateHeartRate();
    
    // Call function to calculate and update SpO₂ level based on the filtered AC and DC components
    updateSpO2();
}


/**
 * @brief Detects heart rate from IR signal and calculates average heart rate.
 * 
 * This function identifies heartbeats by detecting positive zero-crossings in the IR AC signal, which correspond to peaks in blood flow.
 * It calculates the time between successive peaks to estimate the heart rate. Valid heart rate values (between 50 and 165 BPM) 
 * are stored in a circular buffer for averaging. The averaged heart rate is logged and can be sent to a message queue for further processing.
 */
void updateHeartRate(void) {
    char buffer[64];
    // Uncomment the following line if you want to log raw values via UART.
    // snprintf(buffer, sizeof(buffer), "%d,%d,%d\r\n", IR_AC_Signal_Current, RED_AC_Signal_Current, current_timestamp);
    // UART_1_PutString(buffer);

    const CPU_INT32S PEAK_THRESHOLD = 10;  // Adjusted threshold based on graph observation

    // Check for a positive zero-crossing in the IR AC signal
    if (IR_AC_Signal_Current > PEAK_THRESHOLD && IR_AC_Signal_Previous <= PEAK_THRESHOLD) {
        CPU_INT32U time_since_last_peak = current_timestamp - last_peak_timestamp;

        // Check if the cycle started and meets the minimum interval condition
        if (time_since_last_peak > MIN_PEAK_INTERVAL_MS) {
            cycle_started = DEF_TRUE;
            IR_AC_Min = IR_AC_Signal_Current;

            if (last_peak_timestamp != 0) {
                CPU_INT32U time_between_peaks = time_since_last_peak;
                CPU_INT32S heart_rate = (60 * 1000) / time_between_peaks;

                // Process only valid heart rates in the 50-165 BPM range
                if (heart_rate >= 50 && heart_rate <= 165) {
                    // Update history buffer for averaging heart rate
                    heart_rate_sum -= heart_rate_history[heart_rate_index];
                    heart_rate_history[heart_rate_index] = heart_rate;
                    heart_rate_sum += heart_rate;

                    heart_rate_index = (heart_rate_index + 1) % MAX_HEART_RATE_HISTORY;

                    // Calculate the average heart rate from the buffer
                    CPU_INT32S average_heart_rate = heart_rate_sum / MAX_HEART_RATE_HISTORY;

                    // Log the current and average heart rate to UART as CSV
                    snprintf(buffer, sizeof(buffer), "Heart Rate: %d BPM, Average Heart Rate: %d BPM\r\n", heart_rate, average_heart_rate);
                    UART_1_PutString(buffer);

                    // Send the average heart rate to a message queue for external handling
                    enqueueHeartRate(average_heart_rate);
                }
            }
            last_peak_timestamp = current_timestamp;  // Update the timestamp of the last peak
        }
    }

    // Track the minimum value within the current cycle to refine heart rate detection
    if (cycle_started) {
        if (IR_AC_Signal_Current < IR_AC_Min) {
            IR_AC_Min = IR_AC_Signal_Current;
        }

        // Detect negative zero-crossing to mark the end of the cycle
        if (IR_AC_Signal_Current < PEAK_THRESHOLD && IR_AC_Signal_Previous >= PEAK_THRESHOLD) {
            cycle_started = DEF_FALSE;  // End the current cycle
        }
    }

    // Update the previous IR AC signal value for the next cycle comparison
    IR_AC_Signal_Previous = IR_AC_Signal_Current;
}


/**
 * @brief Calculates SpO₂ (oxygen saturation) and logs the result.
 * 
 * This function estimates the SpO₂ level by calculating the ratio of the AC and DC components of the red and IR signals.
 * The SpO₂ value is computed through an empirical formula, and valid results are logged and sent to a message queue for further handling.
 */
void updateSpO2(void) {
    // Calculate SpO₂ using the AC and DC components of red and IR signals
    spO2 = calculateSpO2(RED_AC_Signal_Current, RED_DC_Signal_Current, IR_AC_Signal_Current, IR_DC_Signal_Current);

    // Check for a valid SpO₂ calculation
    if (spO2 != -1) {
        // Log the calculated SpO₂ value
        //Log_Write(LOG_LEVEL_I2C, "SpO₂: %d%%", spO2);

        // Enqueue the SpO₂ result to a message queue for external processing
        enqueueSpO2(spO2);
    }
}


/**
 * @brief Enqueues heart rate data to the message queue.
 * 
 * @param heart_rate Average heart rate to be enqueued
 */
void enqueueHeartRate(CPU_INT32S heart_rate) {
  OS_ERR os_err;
  OS_MSG_QTY entries;
  
  static CPU_INT32S previous_heart_rate;
  
  if (previous_heart_rate == heart_rate){
    //Log_Write(LOG_LEVEL_ERROR, "I2C Task: Value is same as previous value. Do not send it.", 0);
    return;
  }else{
    // Try to post to the queue
    OSQPost(&CommQI2CHeartRate, &heart_rate, sizeof(heart_rate), OS_OPT_POST_FIFO + OS_OPT_POST_ALL, &os_err); 
    if (os_err != OS_ERR_NONE) {
      if (os_err == OS_ERR_Q_MAX) {
        // Log an error if the queue is full
        Log_Write(LOG_LEVEL_ERROR, "I2C Task: queue is full, can't send heart rate value to processing task...", 0);   
        entries = OSQFlush(&CommQI2CHeartRate, &os_err);  // Flush queue if full
        if(os_err == OS_ERR_NONE){
          Log_Write(LOG_LEVEL_DATA_PROCESSING, "Queue is flushed .. ");
        }
      } else {
        // Log any other errors that occur when posting to the queue
        Log_Write(LOG_LEVEL_ERROR, "Error occurred at I2C Task, posting to queue failed...", 0);
      }
    } else {
      // Log a message when the SPO2 value is successfully posted to the queue
      Log_Write(LOG_LEVEL_I2C, "Sent heart rate value to processing task...", 0);
      previous_heart_rate = heart_rate;
    }
  }
}

/**
 * @brief Enqueues SpO2 data to the message queue.
 * 
 * @param spO2 SpO2 value to be enqueued
 */
void enqueueSpO2(CPU_INT32S spO2) {
  OS_ERR os_err;
  static CPU_INT32S previous_spO2_rate = -1;
  OS_MSG_QTY entries;

  if (previous_spO2_rate == spO2){
    //Log_Write(LOG_LEVEL_ERROR, "I2C Task: Value is same as previous value. Do not send it.", 0);
    return;
  }else{
    // Try to post to the queue
    OSQPost(&CommQI2CSPO2, &spO2, sizeof(spO2), OS_OPT_POST_FIFO + OS_OPT_POST_ALL, &os_err); 
    if (os_err != OS_ERR_NONE) {
      if (os_err == OS_ERR_Q_MAX) {
        // Log an error if the queue is full
        Log_Write(LOG_LEVEL_ERROR, "I2C Task: queue is full, can't send SPO2 value to processing task...", 0);
        entries = OSQFlush(&CommQI2CSPO2, &os_err);  // Flush queue if full
        if(os_err == OS_ERR_NONE){
          Log_Write(LOG_LEVEL_DATA_PROCESSING, "Queue is flushed .. ");
        }
      } else {
        // Log any other errors that occur when posting to the queue
        Log_Write(LOG_LEVEL_ERROR, "Error occurred at I2C Task, posting to queue failed...", 0);
      }
    } else {
      // Log a message when the SPO2 value is successfully posted to the queue
      //Log_Write(LOG_LEVEL_I2C, "Sent SPO2 value to processing task...", 0);
      previous_spO2_rate = spO2;
    }
  }
}

/**
 * @brief Calculates oxygen saturation (SpO₂) based on AC and DC components of red and IR signals.
 * 
 * This function estimates the SpO₂ percentage using the ratio of AC (pulsatile) and DC (steady) components from red and IR signals.
 * A common empirical formula is applied to calculate SpO₂, providing an estimated percentage of oxygen saturation.
 * 
 * @param ac_red AC component of the red light signal, indicating pulsatile blood flow
 * @param dc_red DC component of the red light signal, representing steady background absorption
 * @param ac_ir AC component of the IR signal, indicating pulsatile blood flow
 * @param dc_ir DC component of the IR signal, representing steady background absorption
 * 
 * @return Estimated SpO₂ percentage as an integer between 0 and 100. Returns -1 if the calculation is invalid (e.g., due to zero division).
 */
CPU_INT32S calculateSpO2(CPU_INT32S ac_red, CPU_INT32S dc_red, CPU_INT32S ac_ir, CPU_INT32S dc_ir) {
    // Return -1 if any DC component is zero, avoiding division by zero
    if (dc_red == 0 || dc_ir == 0 || ac_ir == 0) {
        return -1;
    }

    // Scale AC red component by 1000 to maintain precision in the ratio calculation
    CPU_INT32S scaled_ac_red = ac_red * 1000;

    // Calculate the ratio of AC and DC components for red and IR, a key metric for SpO₂ estimation
    CPU_INT32S scaled_ratio = (scaled_ac_red * dc_ir) / (dc_red * ac_ir);

    // Calculate SpO₂ using an empirical formula. The constants 110 and 25 are derived from empirical calibration data.
    CPU_INT32S spO2 = 110 - (25 * scaled_ratio / 1000);

    // Return the calculated SpO₂ only if it's in the valid range (0-100%); otherwise, return -1
    return (spO2 >= 0 && spO2 <= 100) ? spO2 : -1;
}


/**
 * @brief DC estimator to compute a smoothed or "average" DC component from incoming signal samples.
 * 
 * This function gradually adjusts the DC (steady) component of the input signal, represented as a "running average" (*p),
 * towards the current sample value (x). By doing so, it acts as a low-pass filter, smoothing out any rapid changes and
 * isolating the slowly varying or constant portion of the signal. This is particularly useful for removing fluctuations
 * in the AC (pulsatile) component, leaving only the baseline DC level.
 * 
 * @param p Pointer to a running average for the DC component, which is updated with each new sample. The initial value of *p
 *          should ideally be set to the first sample value or an approximate baseline of the signal to begin with.
 * @param x The new sample value (e.g., a sensor reading), which will influence the running average to produce a DC estimate.
 * 
 * @return The estimated DC component after applying the filtering step, derived from the updated running average (*p).
 */
CPU_INT16S averageDCEstimator(CPU_INT32S *p, CPU_INT32U x) {
    // 1. Scale up the input sample (x << 8), shifting its bit representation to the left by 8 bits.
    //    This amplifies the precision of the input sample by a factor of 256, preventing precision loss during intermediate steps.
    //
    // 2. Subtract the current running average (*p) from this scaled sample. The difference is an error term that indicates how
    //    much the sample deviates from the running average. This deviation is essential in adjusting the average toward the new value.
    //
    // 3. Right-shift the difference by 1 (effectively dividing it by 2) to produce a weighted adjustment term. By halving the error,
    //    we ensure a gradual adjustment to the running average. This makes the filter behave like a low-pass filter by smoothing
    //    any rapid changes, thus preserving only the slowly varying part of the signal.
    //
    // 4. Add the scaled, weighted adjustment to the running average (*p), bringing the average incrementally closer to the new sample.
    *p += ((((CPU_INT32S)x << 8) - *p) >> 1);

    // 5. Finally, return the current estimate of the DC component by scaling down the updated running average (*p) to its original range.
    //    This is achieved by right-shifting the running average by 8 bits, essentially reversing the initial amplification step. 
    //    The resulting value represents the DC component of the input signal, smoothed and stabilized against rapid fluctuations.
    return (*p >> 8);
}


/**
 * @brief Low-pass FIR filter for smoothing the AC component of the signal.
 *
 * This function implements a Finite Impulse Response (FIR) low-pass filter, which smooths out high-frequency fluctuations
 * in the AC component of a signal, preserving the low-frequency trends. The filter operates by applying a set of predefined
 * coefficients to a circular buffer of past signal samples, creating an output that reflects the overall trend without abrupt changes.
 *
 * @param din Input AC signal sample, representing the latest value in the sequence to be filtered.
 * @return Filtered AC component after applying the FIR filter.
 */
CPU_INT16S lowPassFIRFilter(CPU_INT16S din) {
    static CPU_INT16S cbuf[CIRCULAR_BUFFER_SIZE]; // Circular buffer to store past samples
    static CPU_INT08U offset = 0;                 // Tracks the current position in the circular buffer
    CPU_INT32S z = 0;                             // Accumulates the weighted sum of the filtered result

    // Store the latest input sample into the circular buffer at the current offset position
    cbuf[offset] = din;

    // Apply FIR filter coefficients to the circular buffer to create a weighted sum
    // The last coefficient is applied directly here to reduce loop iterations
    z += mul16(FIRCoeffs[FIR_COEFF_SIZE - 1], cbuf[(offset - FIR_COEFF_SIZE + 1) & (CIRCULAR_BUFFER_SIZE - 1)]);

    // For each FIR coefficient (except the last one applied above), calculate the product with corresponding buffer sample
    for (CPU_INT08U i = 0; i < FIR_COEFF_SIZE - 1; i++) {
        // Multiply each coefficient by the corresponding sample in the circular buffer
        z += mul16(FIRCoeffs[i], cbuf[(offset - i) & (CIRCULAR_BUFFER_SIZE - 1)]);
    }

    // Move the offset to the next buffer position, wrapping around if necessary
    offset = (offset + 1) % CIRCULAR_BUFFER_SIZE;

    // Return the final filtered output by scaling down to the original signal's range
    // The shift operation (>> 15) compensates for the scaling effect of the FIR coefficients
    return (z >> 15);
}


/**
 * @brief Multiplies two 16-bit integers and returns a 32-bit result.
 * 
 * @param x First operand
 * @param y Second operand
 * @return Product of x and y
 */
CPU_INT32S mul16(CPU_INT16S x, CPU_INT16S y) {
    return ((CPU_INT32S)x * (CPU_INT32S)y);
}