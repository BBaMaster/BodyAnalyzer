/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include <environmentSensor.h>

OS_TCB BME688_Task_TCB;                              // Task Control Block for BME688 task
CPU_STK BME688_TaskStk[APP_CFG_TASK_BME688_STK_SIZE]; // Stack for BME688 task
CPU_INT08U GlobalGreen;

void BME688_Init_Task(void) {
    OS_ERR os_err;

    Log_Write(LOG_LEVEL_BME688, "BME688 Init: Creating BME688 task", 0);

    /* Create the MAX30102 task */
    OSTaskCreate((OS_TCB *)&BME688_Task_TCB,           // Task Control Block
                 (CPU_CHAR *)"BME688 Task",            // Task name for debugging
                 (OS_TASK_PTR)bme688_Task,             // Pointer to task function
                 (void *)0,                              // Task arguments (none)
                 (OS_PRIO)APP_CFG_TASK_BME688_PRIO,         // Task priority
                 (CPU_STK *)&BME688_TaskStk[0],        // Stack base
                 (CPU_STK_SIZE)APP_CFG_TASK_BME688_STK_SIZE_LIMIT, // Minimum stack size
                 (CPU_STK_SIZE)APP_CFG_TASK_BME688_STK_SIZE,        // Stack size
                 (OS_MSG_QTY)0u,                         // No message queue required
                 (OS_TICK)0u,                            // No time slice
                 (void *)0,                              // No extra storage
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), // Stack options
                 (OS_ERR *)&os_err);                     // Error code

    // Log task creation success or failure
    if (os_err == OS_ERR_NONE) {
        Log_Write(LOG_LEVEL_BME688, "BME688 Task created successfully", 0);
    } else {
        Log_Write(LOG_LEVEL_ERROR, "Error: BME688 Task creation failed", os_err);
    }
}

//Wrapper function for I2C read
bme68x_read_fptr_t bsp_i2c_read_bme(uint8_t reg_addr, uint8_t* reg_data, uint32_t len, void* intf_ptr) {
    (void)intf_ptr; // Suppress unused parameter warning
    if(bsp_i2c_read(0x76, reg_addr, reg_data, len) != 0){
      Log_Write(LOG_LEVEL_ERROR, "Error Reading I2C BME", 0);
      return BME68X_E_COM_FAIL;
    } else return BME68X_OK;
}

//Wrapper function for i2c write
bme68x_write_fptr_t bsp_i2c_write_bme(uint8_t reg_addr, const uint8_t* reg_data, uint32_t len, void* intf_ptr) {
    (void)intf_ptr; // Suppress unused parameter warning
    if(bsp_i2c_write(0x76, reg_addr, (CPU_INT08U*)reg_data, len) != 0){
      Log_Write(LOG_LEVEL_ERROR, "Error Writing I2C BME", 0);
    return BME68X_E_COM_FAIL;
    } else return BME68X_OK;
}

// Wrapper function to cause microsecond delay
void us_Delay(uint32_t period, void *intf_ptr){
  (void)intf_ptr; // Suppress unused parameter warning
  CyDelayUs(period);
}

void enqueueEnvironmentData(Environment_Data* env_data) {

    DATA_SET_PACKAGE_ENVIRONMENT forReal;
    
    memcpy(&forReal, env_data, sizeof(DATA_SET_PACKAGE_ENVIRONMENT));
    // Assign values directly to struct fields
    forReal.gas_in_ohm = env_data->gas_resistance;
    forReal.humidity_in_percentage = env_data->humidity;
    forReal.pressure_in_bar = env_data->pressure;
    forReal.temperature_in_celsius = env_data->temperature;

    processRawEnvironmentData(&forReal);
    
    if(!isEnvironmentDataInRange(&forReal) ){
      GlobalGreen = GREEN_LED_BLINK;
    }else{
      GlobalGreen = GREEN_LED_FULL_BRIGHTNESS;
    }

    // Format the log message as large integers
    char log_message[128];
    snprintf(log_message, sizeof(log_message),
             "Environment Data - Temperature: %d °C, Humidity: %d %%, Pressure: %d bar, Gas Resistance: %d ohm\n",
             forReal.temperature_in_celsius,
             forReal.humidity_in_percentage,
             forReal.pressure_in_bar,
             forReal.gas_in_ohm);
    
    

    UART_1_PutString(log_message);
}



/**
 @brief Init_BME688 checks if the communication with the sensor is functional and fills the sensors registers with the necessary values
        to start Measuring.

 @param struct bme68x_dev * bme688_handle - struct which includes the function pointers to the interface functions
 @param struct bme68x_conf * conf - struct that encapsules the necessary config values for temperature, humidity and pressure measurements
 @param struct bme68x_heatr_conf * heatr_conf - struct that encapsules the necessary config valus for gas measurements

@retval each bit of the uint8_t return value stand for one of the following erros:
  - 0x01 communication with sensor failed
  - 0x02 self test failed
  - 0x04 T-H-P config write was unsuccessful
  - 0x08 T-H-P values in the sensor registers do not match up with the desired values
  - 0x10 writing heater config was unsuccessful

**/

uint8_t Init_BME688_Sensor(struct bme68x_dev * bme688_handle, struct bme68x_conf * conf, struct bme68x_heatr_conf * heatr_conf){
  // Set Initialization values and function pointers for BME688 library
  uint8_t returnValue = 0;
  bme688_handle->delay_us = us_Delay;
  bme688_handle->read = bsp_i2c_read_bme;
  bme688_handle->write = bsp_i2c_write_bme;
  bme688_handle->intf = BME68X_I2C_INTF;
  bme688_handle->amb_temp = 22;
  int8_t rslt;
  struct bme68x_conf check_conf;
  struct bme68x_heatr_conf check_heatr_conf;
  
  Log_Write(LOG_LEVEL_BME688, "Initializing Bme688", 0);
  
  //Check if BME688 is found
  if (bme68x_init(bme688_handle) != 0){
    Log_Write(LOG_LEVEL_ERROR, "Failed to initialize BME688 sensor", 0);
    returnValue |=  0x01;
    return returnValue; //exit if sensor isnt found
  }
  
  //Selfstest of sensor - not successful?
  rslt = bme68x_selftest_check(bme688_handle);
  if(rslt != 0){
    Log_Write(LOG_LEVEL_ERROR,"Error setting BME688 selftest: %d", rslt);
    returnValue |= 0x02;
  }
  
  //set values for temperature, humidity and pressure measurement
  conf->filter = BME68X_FILTER_OFF;
  conf->odr = BME68X_ODR_NONE;
  conf->os_hum = BME68X_OS_16X;
  conf->os_pres = BME68X_OS_1X;
  conf->os_temp = BME68X_OS_2X;
  
  //write config values to sensor registers
  rslt = bme68x_set_conf(conf, bme688_handle);
  if(rslt != 0){
    Log_Write(LOG_LEVEL_ERROR,"Error setting BME688 meas config", 0);
    returnValue |= 0x04;
  }
  
  //compare if the desired values are in the sensors registers
  rslt = bme68x_get_conf(&check_conf, bme688_handle);
  if(rslt == 0 && !memcmp(&check_conf, &conf, (size_t)sizeof(check_conf))){
    Log_Write(LOG_LEVEL_ERROR,"config compare failed bme688", 0);
    returnValue |= 0x08;
  }

  //set values for heater configuration
  heatr_conf->enable = BME68X_ENABLE   ;
  heatr_conf->heatr_temp = 300;
  heatr_conf->heatr_dur = 100;
  
  //write heater configuration to BME688
  rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, heatr_conf, bme688_handle);
  if(rslt != 0){
    Log_Write(LOG_LEVEL_ERROR,"Error setting BME688 heater config", 0);
    returnValue |= 0x10;
  }
  
  return 0;
}

/**
 @brief The bme688_Task initializes the sensor and performs temperature, humidity, pressure and gas measurements every 4 seconds. 
        The results of the measurement are sent to data processor task via a message queue

 @param void *p_arg - not used
**/
static void bme688_Task(void *p_arg){
  OS_ERR os_err;
  (void)p_arg; // Suppress unused parameter warning
 
  struct bme68x_dev bme688_handle;
  int8_t rslt;
  uint8_t error;
  struct bme68x_conf conf;
  struct bme68x_heatr_conf heatr_conf;
  struct bme68x_data data;
  uint8_t n_fields;
  Environment_Data* env_data = {0};
  
  if((error = Init_BME688_Sensor(&bme688_handle, &conf, &heatr_conf)) != 0){
    Log_Write(LOG_LEVEL_ERROR,"Error initializing BME688: %d", error);
  }else{
    Log_Write(LOG_LEVEL_BME688,"Initializing BME688 sensor successful", 0);
  }  
  
    
  while(DEF_TRUE){
    if (toggle_state) {
      //setting opmode to force mode to start measurement
      rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme688_handle);
      if(rslt != 0){
        Log_Write(LOG_LEVEL_ERROR,"changing BME opmode error: %d", rslt);
      }
      //wait for measurment to be done
      uint16_t del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme688_handle) + (heatr_conf.heatr_dur * 1000);
      bme688_handle.delay_us(del_period, bme688_handle.intf_ptr);
      //read data fom sensor
      rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme688_handle);
      if(rslt != 0){
        Log_Write(LOG_LEVEL_ERROR,"Reading BME values error: %d", rslt);
      }
      //filldata struct 
      env_data->temperature = data.temperature;
      env_data->humidity = data.humidity;
      env_data->pressure = data.pressure;
      env_data->gas_resistance = data.gas_resistance;
      //send Data to queue
      enqueueEnvironmentData(env_data);
      //uncomment if data should be recorded for a csv 
      //sendDataViaUartForCSV(env_data);
      
      
      //wait for defined interval
      OSTimeDlyHMSM(0,0,MEASUREMENT_INTERVAL,0,OS_OPT_TIME_HMSM_NON_STRICT, &os_err);  
    }    
 }
}

/* [] END OF FILE */
