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

#ifndef ENVIRONMENT_SENSOR_H
#define ENVIRONMENT_SENSOR_H

#include "os.h"
#include "bsp_i2c.h"
#include "log_task.h"
#include <bme68x.h>
#include "includes.h"
#include "data_processing_task.h"
  
#define MEASUREMENT_INTERVAL 1
extern CPU_INT08U GlobalGreen;
  
typedef struct{
  int16_t temperature; //temperature in °C * 100
  uint32_t pressure; //pressure in pascal
  uint32_t humidity; //relative humidity in % * 100
  uint32_t gas_resistance; // gas_resistance in Ohm
}Environment_Data;

void BME688_Init_Task(void);
static void bme688_Task(void *p_arg);
void initializeMessageQueueEnvironment(void);
  
#endif /* ENVIRONMENT_SENSOR_H */
/* [] END OF FILE */
