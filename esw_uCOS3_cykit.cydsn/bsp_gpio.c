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

#include "bsp_gpio.h"

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
  
  if(port == PORT1){
    if(pin == P1_6){
      Pin_1_Write(HIGH);
    }else if(pin == P1_2){
      Pin_Green_Led_1_Write(HIGH); 
    }
    else{
      err++;
    }
  }
  else if(port == PORT2){
    if(pin == P2_0){
      Pin_2_Write(P2_0); 
    }
    else if(pin == P2_1){
      Pin_2_Write(P2_1);
    }
    else{
      err++;
    }
  }
  else{
    err++;
  }
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
  
  if(port == PORT1){
    if(pin == P1_6){
      Pin_1_Write(LOW);
    }else if(pin == P2_1){
      Pin_Green_Led_1_Write(LOW);
    }else{
      err++;
    }
  }
  else if(port == PORT2){
    if(pin == P2_0){
      Pin_2_Write(!(P2_0)); 
    }
    else if(pin == P2_1){
      Pin_2_Write(!(P2_1));
    }
    else{
      err++;
    }
  }
  else{
    err++;
  }
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
  }else if(port == PORT1){
    if(pin == P1_2){
      err = Pin_Green_Led_1_Read();
    }else{
      err = -1;
    }
  }
  else{
    err = -1;
  }
  return err;
}


/* [] END OF FILE */
