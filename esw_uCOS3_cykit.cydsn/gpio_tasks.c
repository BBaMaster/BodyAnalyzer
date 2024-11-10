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
CPU_VOID init_gpio(CPU_VOID)
{
  Pin_1_SetDriveMode(Pin_1_DM_STRONG);
  Pin_2_SetDriveMode(Pin_2_DM_STRONG);
  push_button_SetDriveMode(push_button_DM_STRONG);
  push_button_Write(1);
  
  Pin_Blue_Led_1_SetDriveMode(Pin_Blue_Led_DM_STRONG);
  Pin_Green_Led_1_SetDriveMode(Pin_Green_Led_DM_STRONG);
  Pin_Red_Led_1_SetDriveMode(Pin_Red_Led_DM_STRONG);
  
  BSP_PWM_Start(PWM_ac);
  BSP_PWM_Start(PWM_bo);
  BSP_PWM_Start(PWM_hb);
  
  OS_ERR os_err = {0};
  Log_Write(LOG_LEVEL_BUT, "(init_gpio) Knopf-Task wird erstellt", os_err);

    /* Create the Button task */
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
    if (os_err == OS_ERR_NONE) Log_Write(LOG_LEVEL_BUT, "(init_gpio) Knopf-Task erstellt", os_err);
    else Log_Write(LOG_LEVEL_ERROR, "Knopf: (init_gpio) Knopf-Task nicht erstellt", os_err);
    
     /* Create the LeD task */
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
    if (os_err == OS_ERR_NONE) Log_Write(LOG_LEVEL_LED, "(init_gpio) Led-Task erstellt", os_err);
    else Log_Write(LOG_LEVEL_ERROR, "Led: (init_gpio) Led-Task nicht erstellt", os_err);

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
    if (os_err == OS_ERR_NONE) Log_Write(LOG_LEVEL_LED, "(init_gpio) Led-Task erstellt", os_err);
    else Log_Write(LOG_LEVEL_ERROR, "Led: (init_gpio) Led-Task nicht erstellt", os_err);
}

static void BUT_Task(void *p_arg)
{
  (void)p_arg;
  for (OS_ERR os_err; DEF_TRUE; OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err), button_variable = BSP_PB_StatusGet(P2_2));
}

static void LedRB_Task(void *p_arg)
{
  (void)p_arg;
  OS_ERR os_err;
  CPU_TS ts;
  DATA_SET_PACKAGE_OXIMETER *data;
  OS_MSG_SIZE *data_size;
  for (OS_ERR os_err; DEF_TRUE; OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err))
    if (button_variable == DEF_TRUE)
    {
      if (0 != (data = OSQPend(&CommQProcessedOximeterData, 0, OS_OPT_PEND_BLOCKING, data_size, &ts, &os_err)))
      {
        if (*data_size == sizeof(LED_CONTROL_MESSAGE))
        {
          BSP_PWM_set_Period(PWM_hb, data->average_heart_rate_in_bpm);
          BSP_PWM_set_Halfperiod(PWM_bo, data->blood_oxygen_level_in_percentage);
        }
        else Log_Write(LOG_LEVEL_ERROR, "LedB_Task: Nachricht hat falsche grxsze", os_err);
      }
      else Log_Write(LOG_LEVEL_ERROR, "LedB_Task: keine Q-Nachricht", os_err);
    }
}

CPU_INT08U allCorrCalc(CPU_INT08U p)
{
  if (p == GREEN_LED_BLINK) return 1; // blinky blinky
  else if (p == GREEN_LED_FULL_BRIGHTNESS) return 2; // Full brightness
  else return 0; //Error -> green shutdown
}

static void LedG_Task(void *p_arg)
{
  (void)p_arg;
  OS_ERR os_err;
  CPU_TS ts;
  LED_CONTROL_MESSAGE *lcm;
  OS_MSG_SIZE *lcm_size; 
  for (OS_ERR os_err; DEF_TRUE; OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err))
    if (button_variable == DEF_TRUE)
    {
      if (0 != (lcm = OSQPend(&CommQProcessedEnvironmentData, 0, OS_OPT_PEND_BLOCKING, lcm_size, &ts, &os_err)))
      {
        if (*lcm_size == sizeof(LED_CONTROL_MESSAGE))
          BSP_PWM_set_Halfperiod(PWM_ac, allCorrCalc(lcm->mode_data));
        else
          Log_Write(LOG_LEVEL_ERROR, "LedB_Task: Nachricht hat falsche grxsze", os_err);
      }
      else
        Log_Write(LOG_LEVEL_ERROR, "LedB_Task: keine Q-Nachricht", os_err);
    }
}
/* [] END OF FILE */
