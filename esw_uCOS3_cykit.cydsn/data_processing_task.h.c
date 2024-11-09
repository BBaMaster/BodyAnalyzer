#include "data_processing_task.h"
#include <app_cfg.h>
#include "log_task.h"
#include "os.h"

/* Task Control Block and Stack */
OS_TCB Data_Processing_TCB;
CPU_STK Data_Processing_TaskStk[APP_CFG_TASK_DATA_PROCESSING_STK_SIZE];


static void Data_Processing_Task(void *p_arg);


void initMessageQueues(){

  OS_ERR   os_err;
  OSQCreate(&CommQProcessDataOximeter, "Creating message queue to send processed oximeter values", DATA_PROCESSING_QUEUE_SIZE, &os_err);
  if (os_err == OS_ERR_NONE) {
    Log_Write(LOG_LEVEL_DATA_PROCESSING, "Queue created for oximeter data successfully", 0);
  } else {
    Log_Write(LOG_LEVEL_ERROR, "Error: Queue creation failed", os_err);
  }
  
  OSQCreate(&CommQProcessDataEnvironment, "Creating message queue to send processed environment values", DATA_PROCESSING_QUEUE_SIZE, &os_err);
  if (os_err == OS_ERR_NONE) {
    Log_Write(LOG_LEVEL_DATA_PROCESSING, "Queue created for environment data successfully", 0);
  } else {
    Log_Write(LOG_LEVEL_ERROR, "Error: Queue creation failed", os_err);
  }
}

/*
* To convert the timestamp (CPU_TS) into a human-readable format, we’ll need a function that translates the tick count into a more understandable time format 
* like hours, minutes, seconds, and milliseconds. Assuming timestamp is in clock ticks, you’ll need the clock tick rate to determine the exact time in seconds.
*/
ELAPSED_TIME_SET conversionOfTimestamp(CPU_TS *start_timestamp) {
    // Retrieve the current timestamp
    CPU_TS current_timestamp = OS_TS_GET();

    // Calculate the elapsed time (delta) in ticks from the passed start timestamp
    CPU_TS delta_ticks = current_timestamp - *start_timestamp;

    // Confirm tick rate accurately represents system ticks per second
    CPU_INT32U tick_rate = OSCfg_TickRate_Hz;
    if (tick_rate == 0) {
        Log_Write(LOG_LEVEL_ERROR, "Error: Tick rate is zero, cannot compute time.\n");
        return (ELAPSED_TIME_SET){0, 0, 0, 0};;
    }

    // Convert delta ticks to total seconds and milliseconds
    CPU_INT32U total_seconds = delta_ticks / tick_rate;
    CPU_INT32U milliseconds = (delta_ticks % tick_rate) * 1000 / tick_rate;

    // Breakdown total seconds into hours, minutes, and seconds
    ELAPSED_TIME_SET elapsed_time;
    elapsed_time.hours = (total_seconds / 3600) % 24;  // Wrap around after 24 hours
    elapsed_time.minutes = (total_seconds % 3600) / 60;
    elapsed_time.seconds = total_seconds % 60;
    elapsed_time.milliseconds = milliseconds;

    // Log the stopwatch time in human-readable format
    Log_Write(LOG_LEVEL_DATA_PROCESSING, "Time: %02u:%02u:%02u.%03u", 
        elapsed_time.hours, elapsed_time.minutes, 
        elapsed_time.seconds, elapsed_time.milliseconds);
    
    return elapsed_time; // Return the elapsed time structure
}


/* Function to create the Log Task */
void Data_Processing_Init(void) {
    OS_ERR os_err;

    /* Create the log task, but leave the initialization to the task itself */
    OSTaskCreate((OS_TCB     *)&Data_Processing_TCB,
                 (CPU_CHAR   *)"Data Processing Task",
                 (OS_TASK_PTR )Data_Processing_Task,  
                 (void       *)0,
                 (OS_PRIO     )APP_CFG_TASK_DATA_PROCESSING_PRIO,
                 (CPU_STK    *)&Data_Processing_TaskStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_DATA_PROCESSING_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE)APP_CFG_TASK_DATA_PROCESSING_STK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&os_err);


    if (os_err == OS_ERR_NONE) {
        Log_Write(LOG_LEVEL_DATA_PROCESSING, "Data Processing Task created successfully", 0);
    } else {
        Log_Write(LOG_LEVEL_ERROR, "Error: Data Processing Task creation failed", os_err);
    }
}

static void Data_Processing_Task(void *p_arg) {
    
  OS_ERR os_err;
  OS_MSG_QTY entries;
  DATA_SET_PACKAGE_OXIMETER processed_data_oximeter;
  DATA_SET_PACKAGE_ENVIRONMENT processd_data_environment;
  (void)p_arg;

  Log_Write(LOG_LEVEL_DATA_PROCESSING, "Data Processing Task: Initializing message queues ...", 0);
  initMessageQueues();
  
  while(DEF_ON) {
    //Process data only if new data is received from queues ...
    if(ProcessRawOximeterData(&processed_data_oximeter)){

      // Determine the acceptance rates category
      acceptanceRatesCategory category = isDataInAccceptableRange(processed_data_oximeter.average_heart_rate_in_bpm);

      switch (category) {
        case HEART_RATE_GOOD:
            Log_Write(LOG_LEVEL_DATA_PROCESSING, "Heart rate is in good range.", 0);
            break;
        case HEART_RATE_NORMAL:
            Log_Write(LOG_LEVEL_DATA_PROCESSING, "Heart rate is in normal range.", 0);
            break;
        case HEART_RATE_CRITICAL:
            Log_Write(LOG_LEVEL_DATA_PROCESSING, "Heart rate is in critical range!", 0);
            // Additional handling for critical range if necessary
            break;
        default:
            Log_Write(LOG_LEVEL_DATA_PROCESSING, "Heart rate is out of expected range...", 0);
            break;
      }

      // Post to queue for further processing
      OSQPost(&CommQProcessDataOximeter, &processed_data_oximeter, sizeof(processed_data_oximeter), OS_OPT_POST_FIFO, &os_err);
      if (os_err == OS_ERR_Q_MAX) {
          Log_Write(LOG_LEVEL_ERROR, "Data Processing Task: Queue is full, can't send the value to task...", 0);
          entries = OSQFlush(&CommQProcessDataOximeter, &os_err);  // Flush queue if full
          if(os_err == OS_ERR_NONE){
            Log_Write(LOG_LEVEL_DATA_PROCESSING, "Queue is flushed .. ");
          }
      } else if (os_err != OS_ERR_NONE) {
          Log_Write(LOG_LEVEL_ERROR, "Error occurred at Data Processing Task...", 0);
      } else {
          Log_Write(LOG_LEVEL_DATA_PROCESSING, "Sent value to LED control task...", 0);
      }

      // Delay to allow for periodic processing
      OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
    } else if(ProcessRawEnvironmentData(&processd_data_environment)){
      Log_Write(LOG_LEVEL_DATA_PROCESSING, "...environment sensor data should be sent from here ..");
    }
  }
  OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
}

CPU_BOOLEAN ProcessRawOximeterData(DATA_SET_PACKAGE_OXIMETER *data_oximeter_package){
    
  OS_ERR os_err;
  CPU_INT32S *p_msg_I2C;
  OS_MSG_SIZE  msg_size;
  CPU_TS raw_timestamp = OS_TS_GET();
  
  // Initialize package set to store informations to transmit
  *data_oximeter_package = (DATA_SET_PACKAGE_OXIMETER) {0, 0,{0, 0, 0, 0}};

  // Wait for data from the I2C queue
  p_msg_I2C = OSQPend(&CommQI2C, 100 ,OS_OPT_PEND_BLOCKING, &msg_size, &raw_timestamp, &os_err);

  // Give a hint when error occures (not by OS_ERR_TIMEOUT)
  if (os_err == OS_ERR_TIMEOUT) {
     // No errors: Queue was empty, no hints necessary
  } else if (os_err != OS_ERR_NONE) {
      Log_Write(LOG_LEVEL_ERROR, "Data Processing Task: OSQPend() operation failed with error code", os_err);
      
  } else {
      // Received message -> convert and scale value from pointer to CPU_INT08S value && and keep least significant 8-bits
      data_oximeter_package->average_heart_rate_in_bpm = (CPU_INT08S)(*p_msg_I2C & 0xFF); 
      
      // Call conversionOfTimestamp and store the result in elapsed_time field
      data_oximeter_package->elapsed_time = conversionOfTimestamp(&raw_timestamp);
      
      // Log average heart rate received
      Log_Write(LOG_LEVEL_DATA_PROCESSING, "Data Processing Task: Received Average Heart Rate -> %d BPM", data_oximeter_package->average_heart_rate_in_bpm);

      // Log the elapsed time details
      Log_Write(LOG_LEVEL_DATA_PROCESSING, "Elapsed Time: %02u:%02u:%02u.%03u", 
          data_oximeter_package->elapsed_time.hours, 
          data_oximeter_package->elapsed_time.minutes, 
          data_oximeter_package->elapsed_time.seconds, 
          data_oximeter_package->elapsed_time.milliseconds);
      
      return DEF_TRUE;
    }
  return DEF_FALSE;
}

CPU_BOOLEAN ProcessRawEnvironmentData(DATA_SET_PACKAGE_ENVIRONMENT *data_environment_package){
   
  // Initialize package set to store informations to transmit
  *data_environment_package = (DATA_SET_PACKAGE_ENVIRONMENT) {0, {0, 0, 0, 0}};
  return FALSE;
}

acceptanceRatesCategory isDataInAccceptableRange(CPU_INT08S average_heart_rate){
  if (average_heart_rate >= HEART_RATE_GOOD_MIN && average_heart_rate <= HEART_RATE_GOOD_MAX) {
    return HEART_RATE_GOOD;
  } else if (average_heart_rate >= HEART_RATE_NORMAL_MIN && average_heart_rate <= HEART_RATE_NORMAL_MAX) {
    return HEART_RATE_NORMAL;
  } else if (average_heart_rate >= HEART_RATE_CRITICAL_MIN && average_heart_rate <= HEART_RATE_CRITICAL_MAX) {
    return HEART_RATE_CRITICAL;
  } else {
    return HEART_RATE_OUT_OF_RANGE;  // Handle unexpected values
  }
}

/* [] END OF FILE */
