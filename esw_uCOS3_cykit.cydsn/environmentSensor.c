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

void BME688_Init(void) {
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

bme68x_read_fptr_t bsp_i2c_read_bme(uint8_t reg_addr, uint8_t* reg_data, uint32_t len, void* intf_ptr) {
    CPU_INT08U status;
    if(bsp_i2c_read(0x76, reg_addr, reg_data, len) != 0){
      Log_Write(LOG_LEVEL_ERROR, "Error Reading I2C BME", 0);
    }
}

bme68x_write_fptr_t bsp_i2c_write_bme(uint8_t reg_addr, const uint8_t* reg_data, uint32_t len, void* intf_ptr) {
    CPU_INT08U status;
    if(bsp_i2c_write(0x76, reg_addr, reg_data, len) != 0){
      Log_Write(LOG_LEVEL_ERROR, "Error Writing I2C BME", 0);
    }
}

void us_Delay(uint32_t period, void *intf_ptr){
  CyDelayUs(period);
}

static void bme688_Task(void *p_arg){
  OS_ERR os_err;
  (void)p_arg; // Suppress unused parameter warning
  
  Log_Write(LOG_LEVEL_BME688, "Initializing Bme688", 0);
  Clock_1_Start();
  struct bme68x_dev bme688_handle;
  int8_t rslt;
  struct bme68x_conf conf, check_conf;
  struct bme68x_heatr_conf heatr_conf, check_heatr_conf;
  struct bme68x_data data;
  uint8_t n_fields;
  bme688_handle.delay_us = us_Delay;
  bme688_handle.read = bsp_i2c_read_bme;
  bme688_handle.write = bsp_i2c_write_bme;
  bme688_handle.intf = BME68X_I2C_INTF;
  bme688_handle.amb_temp = 22;
  
  uint8_t chip_Id = 0;
  
  if (bme68x_init(&bme688_handle) != 0){
    Log_Write(LOG_LEVEL_ERROR, "Failed to initialize BME688 sensor", 0);
    return;  // Exit if sensor initialization fails
  }
  
  rslt = bme68x_selftest_check(&bme688_handle);
  if(rslt != 0){
    Log_Write(LOG_LEVEL_BME688,"Error setting BME688 selftest: %d", rslt);
  }
  
  Log_Write(LOG_LEVEL_BME688, "Bme688 found", 0);
  
  conf.filter = BME68X_FILTER_OFF;
  conf.odr = BME68X_ODR_NONE;
  conf.os_hum = BME68X_OS_16X;
  conf.os_pres = BME68X_OS_1X;
  conf.os_temp = BME68X_OS_2X;
  rslt = bme68x_set_conf(&conf, &bme688_handle);
  if(rslt != 0){
    Log_Write(LOG_LEVEL_ERROR,"Error setting BME688 meas config", 0);
  }
  
  rslt = bme68x_get_conf(&check_conf, &bme688_handle);
  if(rslt == 0 && !memcmp(&check_conf, &conf, (size_t)sizeof(conf))){
    Log_Write(LOG_LEVEL_BME688,"config compare failed bme688", 0);
  }
  
  heatr_conf.enable = BME68X_ENABLE   ;
  heatr_conf.heatr_temp = 300;
  heatr_conf.heatr_dur = 100;
  rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme688_handle);
  if(rslt != 0){
    Log_Write(LOG_LEVEL_ERROR,"Error setting BME688 meas config", 0);
  }else{
    Log_Write(LOG_LEVEL_BME688,"Setting BME688 meas config successful", 0);
  }
  
  memcpy(&check_heatr_conf, &heatr_conf, sizeof(heatr_conf));
  
  rslt = bme68x_get_heatr_conf(&check_heatr_conf, &bme688_handle);
  if(rslt == 0 && !memcmp(&check_heatr_conf, &heatr_conf, (size_t)sizeof(heatr_conf))){
    Log_Write(LOG_LEVEL_BME688,"Setting BME688 meas config successful", 0);
  }
  
  Log_Write(LOG_LEVEL_BME688,"Initializing BME688 sensor successful", 0);
  
  for(;;){
    rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme688_handle);
    if(rslt != 0){
      Log_Write(LOG_LEVEL_ERROR,"changing BME opmode error: %d", rslt);
    }
    uint16_t del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme688_handle) + (heatr_conf.heatr_dur * 1000);
    bme688_handle.delay_us(del_period, bme688_handle.intf_ptr);
    
    rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme688_handle);
    if(rslt != 0){
      Log_Write(LOG_LEVEL_ERROR,"Reading BME values error: %d", rslt);
    }
    
    Log_Write(LOG_LEVEL_BME688,"temp: %d", data.temperature);
    Log_Write(LOG_LEVEL_BME688,"humidity: %d", data.humidity);
    Log_Write(LOG_LEVEL_BME688,"pressure: %d", data.pressure);
    Log_Write(LOG_LEVEL_BME688,"gas: %d", data.gas_resistance);
    
    OSTimeDlyHMSM(0,0,4,0,OS_OPT_TIME_HMSM_NON_STRICT, &os_err); 
    
  }
  
}

/* [] END OF FILE */
