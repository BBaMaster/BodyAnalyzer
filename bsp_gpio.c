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

#include <bsp_gpio.h>


/*
*********************************************************************************************************
*                                       init_gpio()
*
* Description : This function initializes the Pin_1 GPIO module (P1_6, J1 Pin 23), the
*               Pin_2 GPIO module (P2_0, J1 Pin 1 and P2_1, J1 Pin 2) and the push button
*               GPIO module (P2_2, J1 Pin 3). On P2_1 a blue on-board LED is connected. On
*               P2_2 on side of a GND-connected button is connected.
*               
*               Es wird zusätzlich auch noch der Task für den Knopf und die LeDs
*               initialisiert.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : Not re-entrant.
*********************************************************************************************************
*/

CPU_VOID init_gpio(CPU_VOID)
{
  Pin_1_SetDriveMode(Pin_1_DM_STRONG);
  Pin_2_SetDriveMode(Pin_2_DM_STRONG);
  push_button_SetDriveMode(push_button_DM_STRONG);
  push_button_Write(1);
  
  Pin_Blue_Led_SetDriveMode(Pin_Blue_Led_DM_STRONG);
  Pin_Green_Led_SetDriveMode(Pin_Green_Led_DM_STRONG);
  Pin_Red_Led_SetDriveMode(Pin_Red_Led_DM_STRONG);
  
  OS_ERR os_err = {0};
  Log_Write(LOG_LEVEL_BUT, "(init_gpio) Knopf-Task wird erstellt", os_err);

    /* Create the I2C task */
    OSTaskCreate((OS_TCB *)&BUT_Task_TCB,
                 (CPU_CHAR *)"BUT Task",
                 (OS_TASK_PTR)BUT_Task,
                 (void *)0,
                 (OS_PRIO)APP_CFG_TASK_I2C_PRIO,
                 (CPU_STK *)&BUT_TaskStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_I2C_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE)APP_CFG_TASK_I2C_STK_SIZE,
                 (OS_MSG_QTY)0u,
                 (OS_TICK)0u,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&os_err);

    if (os_err == OS_ERR_NONE) Log_Write(LOG_LEVEL_BUT, "(init_gpio) Knopf-Task erstellt", os_err);
    else Log_Write(LOG_LEVEL_ERROR, "Knopf: (init_gpio) Knopf-Task nicht erstellt", os_err);
}

static void BUT_Task(void *p_arg)
{
  (void)p_arg;
  OS_ERR os_err;
  while (DEF_TRUE) if (BSP_PB_StatusGet(P2_2))
    {
      OSSemPost(&Knopf_Sem, OS_OPT_POST_ALL, &os_err);
      if (os_err != OS_ERR_NONE)
        Log_Write(LOG_LEVEL_ERROR, "Knopf: (BUT_Task) Knopf-Semaphore kaputt", os_err);
    }
}

/*
*********************************************************************************************************
*                                       gpio_high()
*
* Description : This function set based on a chosen port (PORT1, PORT2) the digital level
*               of a chosen pin (P1_6, P2_0, P2_1) on digital high (~ 5V).
*
* Argument(s) : port ... indicates which port should be addressed (PORT1, PORT2)
*               pin  ... indicates which pin of the chosen port is addressed (P1_6, P2_0, P2_1)
*
* Return(s)   : 0 ... if no error occurred
*               >1... if an error occurred
*
* Caller(s)   : Application.
*
* Note(s)     : Not re-entrant.
*********************************************************************************************************
*/

CPU_INT08U gpio_high(CPU_INT08U port, CPU_INT08U pin)
{
  CPU_INT08U err = 0;
  
  if(port == PORT1)
  {
    if(pin == P1_6) Pin_1_Write(HIGH);
    else if (pin == PIN_GREEN_LED) Pin_Green_Led_Write(HIGH);
    else err++;
  }
  else if(port == PORT2)
  {
    if(pin == P2_0) Pin_2_Write(P2_0);
    else if (pin == P2_1) Pin_2_Write(P2_1);
    else if (pin == PIN_BLUE_LED) Pin_Blue_Led_Write(HIGH);
    else if (pin == PIN_RED_LED) Pin_Red_Led_Write(HIGH);
    else err++;
  }
  else err++;
  return err;    
}

/*
*********************************************************************************************************
*                                       gpio_low()
*
* Description : This function resets based on a chosen port (PORT1, PORT2) the digital level
*               of a chosen pin (P1_6, P2_0, P2_1) on digital low (0V).
*
* Argument(s) : port ... indicates which port should be addressed (PORT1, PORT2)
*               pin  ... indicates which pin of the chosen port is addressed (P1_6, P2_0, P2_1)
*
* Return(s)   : 0 ... if no error occurred
*               >1... if an error occurred
*
* Caller(s)   : Application.
*
* Note(s)     : Not re-entrant.
*********************************************************************************************************
*/

CPU_INT08U gpio_low(CPU_INT08U port, CPU_INT08U pin)
{
  CPU_INT08U err = 0;
  
  if(port == PORT1)
  {
    if(pin == P1_6) Pin_1_Write(LOW);
    else if (pin == PIN_GREEN_LED) Pin_Green_Led_Write(LOW);
    else err++;
  }
  else if(port == PORT2)
  {
    if(pin == P2_0) Pin_2_Write(!P2_0);
    else if (pin == P2_1) Pin_2_Write(!P2_1);
    else if (pin == PIN_BLUE_LED) Pin_Blue_Led_Write(LOW);
    else if (pin == PIN_RED_LED) Pin_Red_Led_Write(LOW);
    else err++;
  }
  else err++;
  return err;     
}

/*
*********************************************************************************************************
*                                       gpio_read()
*
* Description : This function reads the digital level of a chosen port (PORT2) and pin (P2_2).
*
* Argument(s) : port ... indicates which port should be addressed (PORT2)
*               pin  ... indicates which pin of the chosen port is addressed (P2_2)
*
* Return(s)   : 0 ... if no error occurred
*               -1... if an error occurred
*
* Caller(s)   : Application.
*
* Note(s)     : Not re-entrant.
*********************************************************************************************************
*/

CPU_INT08S gpio_read(CPU_INT08U port, CPU_INT08U pin)
{
  CPU_INT08S err = 0;
  
  if(port == PORT2){
    if(pin == P2_2){
      err = push_button_Read();    
    }
    else{
      err = -1;
    }
  }
  else{
    err = -1;
  }
  return err;
}


/* [] END OF FILE */
