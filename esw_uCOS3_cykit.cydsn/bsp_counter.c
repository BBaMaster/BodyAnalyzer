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

#include <bsp_counter.h>

void init_counter(){
  Counter_1_Init();
  Counter_1_Enable(); 
}

void start_counter(uint16_t period){
  Counter_1_WriteCounter(0);
  Counter_1_WritePeriod(period);
  Counter_1_Start();
}

void stop_counter(){
  Counter_1_Stop(); 
}

uint16_t read_counter_value(){
  return Counter_1_ReadCounter();
}
/* [] END OF FILE */
