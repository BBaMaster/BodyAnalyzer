#include "data_processing_task.h"
#include <app_cfg.h>
#include "log_task.h"
#include "os.h"

/* Task Control Block and Stack */
OS_TCB Data_Processing_TCB;
CPU_STK Data_Processing_TaskStk[APP_CFG_TASK_DATA_PROCESSING_STK_SIZE];

/* Function prototype */
static void Data_Processing_Task(void *p_arg);

/**
 @brief This function initializes two message queues to send processed sensor values.

 @param none
**/
void initMessageQueues(){

  OS_ERR   os_err;
  OSQCreate(&CommQProcessedOximeterData, "Creating message queue to send processed oximeter values", DATA_PROCESSING_QUEUE_SIZE, &os_err);
  if (os_err == OS_ERR_NONE) {
    Log_Write(LOG_LEVEL_DATA_PROCESSING, "Queue created for oximeter data successfully", 0);
  } else {
    Log_Write(LOG_LEVEL_ERROR, "Error: Queue creation failed", os_err);
  }
  
  OSQCreate(&CommQProcessedEnvironmentData, "Creating message queue to send processed environment values", DATA_PROCESSING_QUEUE_SIZE, &os_err);
  if (os_err == OS_ERR_NONE) {
    Log_Write(LOG_LEVEL_DATA_PROCESSING, "Queue created for environment data successfully", 0);
  } else {
    Log_Write(LOG_LEVEL_ERROR, "Error: Queue creation failed", os_err);
  }
}

/**
 @brief This is the initialization function for the task itself which is called from main().

 @param void
**/
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

/**
 @brief Main task for the data processing part of the project. This task is responsible
        to gather values from both sensors -> oximeter and environment and control them against pre-defined
        acceptance ranges. After checking the processed values are sent to a message queue to further processing
        in led task, logging etc ...

 @param void *p_arg
**/
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
    if(processRawOximeterData(&processed_data_oximeter)){

      // Determine the acceptance rates category
      acceptanceRatesCategory category = isDataInAccceptableRange(processed_data_oximeter.average_heart_rate_in_bpm, processed_data_oximeter.blood_oxygen_level_in_percentage);

      switch (category) {
        case HEART_RATE_GOOD:
            Log_Write(LOG_LEVEL_DATA_PROCESSING, "Average Heart rate is in good range.", 0);
            break;
        case HEART_RATE_NORMAL:
            Log_Write(LOG_LEVEL_DATA_PROCESSING, "Average Heart rate is in normal range.", 0);
            break;
        case HEART_RATE_CRITICAL:
            Log_Write(LOG_LEVEL_DATA_PROCESSING, "Average Heart rate is in critical range!", 0);
            break;
        case BLOOD_OXYGEN_NORMAL:
            Log_Write(LOG_LEVEL_DATA_PROCESSING, "Blood oxygen level is in normal range!", 0);
            break;
        case BLOOD_OXYGEN_TOLERABLE:
            Log_Write(LOG_LEVEL_DATA_PROCESSING, "Blood oxygen is on a tolerable level!", 0);
            break;
        case BLOOD_OXYGEN_DECREASED:
            Log_Write(LOG_LEVEL_DATA_PROCESSING, "Blood oxygen level decreased..be careful!", 0);
            break;
        default:
            Log_Write(LOG_LEVEL_DATA_PROCESSING, "out of range...", 0);
            break;
      }

      // Post to queue for further processing
      OSQPost(&CommQProcessedOximeterData, &processed_data_oximeter, sizeof(processed_data_oximeter), OS_OPT_POST_FIFO, &os_err);
      if (os_err == OS_ERR_Q_MAX) {
          Log_Write(LOG_LEVEL_ERROR, "Data Processing Task: Queue is full, can't send the value to further task...", 0);
          entries = OSQFlush(&CommQProcessedOximeterData, &os_err);  // Flush queue if full
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
    } else if(processRawEnvironmentData(&processd_data_environment)){
      Log_Write(LOG_LEVEL_DATA_PROCESSING, "...environment sensor data should be sent from here ..");
    }
  }
  OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
}

CPU_BOOLEAN RetrieveSensorData(OS_Q *queue, CPU_INT08S *data_field, const char *sensor_name) {
    OS_ERR os_err;
    OS_MSG_SIZE msg_size;
    CPU_TS raw_timestamp = OS_TS_GET();
    CPU_INT32S *p_msg_I2C;

    // Wait for data from the specified queue
    p_msg_I2C = OSQPend(queue, 100, OS_OPT_PEND_BLOCKING, &msg_size, &raw_timestamp, &os_err);

    // Handle potential errors or timeouts
    if (os_err == OS_ERR_TIMEOUT) {
        // No errors, queue was just empty; no further action needed
        return DEF_FALSE;
    } else if (os_err != OS_ERR_NONE) {
        Log_Write(LOG_LEVEL_ERROR, "Data Processing Task: OSQPend() failed for %s with error code %d", sensor_name, os_err);
        return DEF_FALSE;
    }

    // Successfully received data, process and store the least significant 8 bits
    *data_field = (CPU_INT08S)(*p_msg_I2C & 0xFF);

    // Log the received data
    Log_Write(LOG_LEVEL_DATA_PROCESSING, "Data Processing Task: Received %s -> %d", sensor_name, *data_field);
    return DEF_TRUE;
}

CPU_BOOLEAN processRawOximeterData(DATA_SET_PACKAGE_OXIMETER *data_oximeter_package) {
    CPU_BOOLEAN heart_rate_received = DEF_FALSE;
    CPU_BOOLEAN spo2_received = DEF_FALSE;

    // Initialize data structure fields
    *data_oximeter_package = (DATA_SET_PACKAGE_OXIMETER) {0, 0};

    // Retrieve heart rate data
    heart_rate_received = RetrieveSensorData(&CommQI2CHeartRate, &data_oximeter_package->average_heart_rate_in_bpm, "Average Heart Rate");

    // Retrieve SpO2 data
    spo2_received = RetrieveSensorData(&CommQI2CSPO2, &data_oximeter_package->blood_oxygen_level_in_percentage, "SpO2");

    // Return TRUE only if both data values were successfully received
    return heart_rate_received && spo2_received;
}

CPU_BOOLEAN processRawEnvironmentData(DATA_SET_PACKAGE_ENVIRONMENT *data_environment_package) {
    
    OS_ERR os_err;
    OS_MSG_SIZE msg_size;
    CPU_TS raw_timestamp = OS_TS_GET();
    
    // Initialize package set to store information to transmit
    *data_environment_package = (DATA_SET_PACKAGE_ENVIRONMENT){0, 0, 0, 0};
    
    // Define a variable to hold the received data
    RAW_ENVIRONMENT_DATA *p_msg_SPI_data;

    // Wait for environmental data from the queue
    p_msg_SPI_data = (RAW_ENVIRONMENT_DATA *)OSQPend(&CommQSPIData, 100, OS_OPT_PEND_BLOCKING, &msg_size, &raw_timestamp, &os_err);

    if (os_err == OS_ERR_TIMEOUT) {
        // Queue was empty, no hints necessary
        return DEF_FALSE;
    } else if (os_err != OS_ERR_NONE) {
        Log_Write(LOG_LEVEL_ERROR, "Data Processing Task: OSQPend() operation failed for environmental data with error code", os_err);
        return DEF_FALSE;
    } else {
      // Convert and scale each environmental parameter
      
      // Convert and scale temperature
      data_environment_package->temperature_in_celsius = ((p_msg_SPI_data->temperature_raw * 12500) / 4095) - 4000;
      
      // Convert and scale humidity
      data_environment_package->humidity_in_percentage = (p_msg_SPI_data->humidity_raw * 10000) / 65535;
      
      // Convert and scale pressure
      data_environment_package->pressure_in_bar = ((p_msg_SPI_data->pressure_raw * 8000) / 65535) + 3000;
      
      // Convert gas concentration
      data_environment_package->gas_in_ohm = (p_msg_SPI_data->gas_raw * 1000) / 65535;
      
      // Log each converted environmental value
      Log_Write(LOG_LEVEL_DATA_PROCESSING, "Data Processing Task: Received Temperature -> %d.%02d deg C", 
                data_environment_package->temperature_in_celsius / 100, data_environment_package->temperature_in_celsius % 100);
      Log_Write(LOG_LEVEL_DATA_PROCESSING, "Data Processing Task: Received Humidity -> %d.%02d %%", 
                data_environment_package->humidity_in_percentage / 100, data_environment_package->humidity_in_percentage % 100);
      Log_Write(LOG_LEVEL_DATA_PROCESSING, "Data Processing Task: Received Pressure -> %d.%1d hPa", 
                data_environment_package->pressure_in_bar / 10, data_environment_package->pressure_in_bar % 10);
      Log_Write(LOG_LEVEL_DATA_PROCESSING, "Data Processing Task: Received Gas Concentration -> %d ppm", 
                data_environment_package->gas_in_ohm);

      return DEF_TRUE;
    }
}


acceptanceRatesCategory isDataInAccceptableRange(CPU_INT08S average_heart_rate, CPU_INT08S blood_oxygen_level) {
  
  // Classify Heart Rate
  if (average_heart_rate >= HEART_RATE_GOOD_MIN && average_heart_rate <= HEART_RATE_GOOD_MAX) {
      return HEART_RATE_GOOD;
  }
  if (average_heart_rate >= HEART_RATE_NORMAL_MIN && average_heart_rate <= HEART_RATE_NORMAL_MAX) {
      return HEART_RATE_NORMAL;
  }
  if (average_heart_rate >= HEART_RATE_CRITICAL_MIN && average_heart_rate <= HEART_RATE_CRITICAL_MAX) {
      return HEART_RATE_CRITICAL;
  }

  // Classify Blood Oxygen Level (only if heart rate is out of range)
  if (blood_oxygen_level >= BLOOD_OXYGEN_LEVEL_NORMAL_MIN && blood_oxygen_level <= BLOOD_OXYGEN_LEVEL_NORMAL_MAX) {
      return BLOOD_OXYGEN_NORMAL;
  }
  if (blood_oxygen_level >= BLOOD_OXYGEN_LEVEL_TOLERABLE_MIN && blood_oxygen_level <= BLOOD_OXYGEN_LEVEL_TOLERABLE_MAX) {
      return BLOOD_OXYGEN_TOLERABLE;
  }
  if (blood_oxygen_level >= BLOOD_OXYGEN_LEVEL_DECREASED_MIN && blood_oxygen_level <= BLOOD_OXYGEN_LEVEL_DECREASED_MAX) {
      return BLOOD_OXYGEN_DECREASED;
  }
  
  // If both heart rate and blood oxygen level are out of range
  return DATA_OUT_OF_RANGE;
}


/* [] END OF FILE */
