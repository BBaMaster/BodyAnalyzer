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
#include "max30102_interface.h"
#include "bsp_i2c.h"
#include "log_task.h"
#include <bme68x.h>
#include <bsp_counter.h>

void BME688_Init(void);
static void bme688_Task(void *p_arg);
  
#endif /* ENVIRONMENT_SENSOR_H */
/* [] END OF FILE */
