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

#include <gpio_tasks.h>

CPU_BOOLEAN toggle_state = 0;
OS_MEM OximeterDataProcessed;
OS_MEM EnvSensorDataProcessed;
/*
initialises everything gpioness
*/
void init_gpio()
{
    Log_Write(LOG_LEVEL_INFO,"We are in the init");
    OS_ERR os_err;
    PWM_Environment_Start();
    PWM_Heartbeat_Start();
    Pin_1_SetDriveMode(Pin_1_DM_STRONG);
    Pin_2_SetDriveMode(Pin_2_DM_STRONG);
    push_button_SetDriveMode(push_button_DM_STRONG);
    push_button_Write(1);
    

    uint8_t EnvSensorDataPart[100][sizeof(LED_CONTROL_MESSAGE) * 8];
    uint8_t OximeterDataProcessedData[100][sizeof(DATA_SET_PACKAGE_OXIMETER) * 8];
    OSMemCreate(&EnvSensorDataProcessed, "evnironment sensor data block", &EnvSensorDataPart[0][0], 100, sizeof(LED_CONTROL_MESSAGE) * 8, &os_err);
    OSMemCreate(&OximeterDataProcessed, "evnironment sensor data block", &OximeterDataProcessedData[0][0], 100, sizeof(DATA_SET_PACKAGE_OXIMETER) * 8, &os_err);
    
    OSTaskCreate((OS_TCB *)&BUT_Task_TCB,
                 (CPU_CHAR *)"BUT Task",
                 (OS_TASK_PTR)BUT_Task,
                 (void *)0,
                 (OS_PRIO)APP_CFG_TASK_BUTTON_PRIO,
                 (CPU_STK *)&BUT_TaskStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_BUTTON_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE)APP_CFG_TASK_BUTTON_STK_SIZE,
                 (OS_MSG_QTY)0u,
                 (OS_TICK)0u,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&os_err);
    if (os_err == OS_ERR_NONE) Log_Write(LOG_LEVEL_BUT, "(init_gpio) But-Task created", os_err);
    else Log_Write(LOG_LEVEL_ERROR, "Knopf: (init_gpio) But-Task not created", os_err);
    
    OSTaskCreate((OS_TCB *)&LedG_Task_TCB,
                 (CPU_CHAR *)"LedG Task",
                 (OS_TASK_PTR)LedG_Task,
                 (void *)0,
                 (OS_PRIO)APP_CFG_TASK_LED_PRIO,
                 (CPU_STK *)&LedG_TaskStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_LED_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE)APP_CFG_TASK_LED_STK_SIZE,
                 (OS_MSG_QTY)0u,
                 (OS_TICK)0u,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&os_err);
    if (os_err == OS_ERR_NONE) Log_Write(LOG_LEVEL_LED, "(init_gpio) Led-Task created", os_err);
    else Log_Write(LOG_LEVEL_ERROR, "Led: (init_gpio) Led-Task not created", os_err);
/*
    OSTaskCreate((OS_TCB *)&LedRB_Task_TCB,
                 (CPU_CHAR *)"LedRB Task",
                 (OS_TASK_PTR)LedRB_Task,
                 (void *)0,
                 (OS_PRIO)APP_CFG_TASK_LED_PRIO,
                 (CPU_STK *)&LedRB_TaskStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_LED_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE)APP_CFG_TASK_LED_STK_SIZE,
                 (OS_MSG_QTY)0u,
                 (OS_TICK)0u,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&os_err);
    if (os_err == OS_ERR_NONE) Log_Write(LOG_LEVEL_LED, "(init_gpio) Led-Task created", os_err);
    else Log_Write(LOG_LEVEL_ERROR, "Led: (init_gpio) Led-Task not created", os_err);\
    */
}

/*
when the button is pressed button var is true
when not then not
*/
static void BUT_Task(void *p_arg)
{
    OS_ERR err;
    (void)p_arg;

    Log_Write(LOG_LEVEL_INFO, "Button Task initialized.\n");

    CPU_BOOLEAN button_status = 0;
    CPU_BOOLEAN last_button_status = 1; // Assume unpressed state initially

    while (DEF_TRUE) {
        button_status = BSP_PB_StatusGet(P2_2);

        if (button_status == 0 && last_button_status == 1) {
            toggle_state = !toggle_state;  // Toggle the state

            if (toggle_state) {
                Log_Write(LOG_LEVEL_INFO, "Button pressed: Activating tasks.\n");
            } else {
                Log_Write(LOG_LEVEL_INFO, "Button pressed: Deactivating tasks.\n");
            }
        }

        last_button_status = button_status;
        OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &err);
        if (err != OS_ERR_NONE) {
            Log_Write(LOG_LEVEL_ERROR,"Failed to delay in Button Task\n");
        }
    }
}
/*
calculates the heart rate from bpm to count of 50 Hz.
*/
CPU_INT16U heartratecalc(CPU_INT08U p)
{
  return 3000/p;
}

/*
output of heartrate and blood oxygen per Q from data processing
*/
static void LedRB_Task(void *p_arg)
{
  (void)p_arg;
  OS_ERR os_err;
  CPU_TS ts;
  DATA_SET_PACKAGE_OXIMETER *data;
  DATA_SET_PACKAGE_OXIMETER oxi_data;
  OS_MSG_SIZE *data_size = NULL;
  for (OS_ERR os_err; DEF_TRUE; OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err))
    if (button_variable == DEF_TRUE)
    {
      if (0 != (data = OSQPend(&CommQProcessedOximeterData, 0, OS_OPT_PEND_BLOCKING, data_size, &ts, &os_err)))
      {
        if (os_err == OS_ERR_NONE)
        {
          if (*data_size == sizeof(DATA_SET_PACKAGE_OXIMETER))
          {
            memcpy(&oxi_data, data, sizeof(DATA_SET_PACKAGE_OXIMETER));
            OSMemPut(&OximeterDataProcessed, data, &os_err);
            if(os_err != OS_ERR_NONE){
              Log_Write(LOG_LEVEL_ERROR, "Error putting memory space in LED_RBTask", 0);
            }
            
            BSP_PWM_set_Period(PWM_hb, heartratecalc(oxi_data.average_heart_rate_in_bpm));
            BSP_PWM_set_Halfperiod(PWM_bo, oxi_data.blood_oxygen_level_in_percentage);
          }
          else Log_Write(LOG_LEVEL_ERROR, "LedRB_Task: msg wrong size", os_err);
        }
        else Log_Write(LOG_LEVEL_ERROR, "LedRB_Task: unknown", os_err);
      }
      else Log_Write(LOG_LEVEL_ERROR, "LedRB_Task: no Q-Nachricht", os_err);
    }
}

/*
calculates the green led (wellbeing of the variables)
*/
CPU_INT08U allCorrCalc(CPU_INT08U p)
{
  if (p == GREEN_LED_BLINK) return 25; // blinky blinky
  else if (p == GREEN_LED_FULL_BRIGHTNESS) return 50; // Full brightness
  else return 0; //Error -> green shutdown
}

/*
output on the green led steert by data processing
*/
static void LedG_Task(void *p_arg)
{
  (void)p_arg;
  OS_ERR os_err;
  CPU_TS ts;
  LED_CONTROL_MESSAGE * command_message = NULL;
  LED_CONTROL_MESSAGE env_data;
  env_data.led_cmd = CMD_GREEN_LED;
  env_data.mode_data = GREEN_LED_FULL_BRIGHTNESS;
  OS_MSG_SIZE message_size;
  
  while(DEF_TRUE){
    if(toggle_state){
      //wait if led state changes
      if((command_message = OSQPend(&CommQProcessedEnvironmentData, 10, OS_OPT_PEND_NON_BLOCKING, &message_size, &ts, &os_err)) != NULL && os_err == 0){
        memcpy(&env_data, command_message, sizeof(LED_CONTROL_MESSAGE));
        OSMemPut(&EnvSensorDataProcessed, command_message, &os_err);
        if(os_err != OS_ERR_NONE){
          Log_Write(LOG_LEVEL_ERROR, "Error putting memory space in processRawEnvironmentData", 0);
        }
      }
      //check LED mode to change to
      if(env_data.mode_data == GREEN_LED_BLINK){
        //if LED is on -> turn off and vise versa
          if(gpio_read(PORT1,P1_2) == 1){
            gpio_high(PORT1,P1_2);
          }else if(gpio_read(PORT1,P1_2) == 0){
            gpio_low(PORT1,P1_2);  
          }else{
            Log_Write(LOG_LEVEL_ERROR, "Blink Error", 0);
          }
          //OSDelay to make LED blink
          OSTimeDlyHMSM(0,0,0,250,OS_OPT_TIME_HMSM_NON_STRICT, &os_err);
          continue; //to skip rest of while loop
      }else{
        if(gpio_read(PORT1,P1_2) == 0){
          gpio_high(PORT1,P1_2); //keep LED on
        }
      }
    }else{
      gpio_low(PORT1,P1_2);
    }
    
    OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_NON_STRICT, &os_err);
  } 
}
/* [] END OF FILE */
