
#include <gpio_tasks.h>

CPU_BOOLEAN toggle_state = 0;
/*
initialises everything gpioness
*/
void init_gpio()
{
    Log_Write(LOG_LEVEL_INFO,"We are in the init");
    OS_ERR os_err;
    PWM_SPO2_Start();
    PWM_Heartbeat_Start();
    Pin_1_SetDriveMode(Pin_1_DM_STRONG);
    Pin_2_SetDriveMode(Pin_2_DM_STRONG);
    push_button_SetDriveMode(push_button_DM_STRONG);
    push_button_Write(1);
    
    
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
                if (os_err == OS_ERR_NONE) {
                    UART_1_PutString("(init_gpio) But-Task created\n");
                } else {
                    char error_msg[64];
                    snprintf(error_msg, sizeof(error_msg), "Knopf: (init_gpio) But-Task not created. Error Code: %d\n", os_err);
                    UART_1_PutString(error_msg);
                }

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
                if (os_err == OS_ERR_NONE) {
                    UART_1_PutString("(init_gpio) RB-Task created\n");
                } else {
                    char error_msg[64];
                    snprintf(error_msg, sizeof(error_msg), "Knopf: (init_gpio) RB-Task not created. Error Code: %d\n", os_err);
                    UART_1_PutString(error_msg);
                }
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
    (void)p_arg;  // Unused parameter
    OS_ERR os_err;

    while (DEF_TRUE) {
        // Calculate PWM value for SpO2
        CPU_INT08U pwm_spo2 = (CPU_INT08U)((Global_spO2 * 255) / 100);

        // Update the PWM compare value based on SpO2, ensuring proper scaling
        if (Global_spO2 > 0) {
            PWM_SPO2_WriteCompare(pwm_spo2);  // Set PWM for SpO2
        } else {
            PWM_SPO2_WriteCompare(0);  // Set PWM to 0 if SpO2 is zero
        }

        // Heartbeat effect for red LED
        if (Global_heart_rate > 0) {
            // Calculate half-period time in ms based on heart rate (beats per minute)
            CPU_INT16U half_beat_period_ms = (60000 / Global_heart_rate) / 2;  // Half of one beat period

            // Turn LED on for half the beat period
            PWM_Heartbeat_WriteCompare(255);  // Full brightness
            OSTimeDlyHMSM(0, 0, 0, half_beat_period_ms, OS_OPT_TIME_HMSM_STRICT, &os_err);

            // Turn LED off for the other half
            PWM_Heartbeat_WriteCompare(0);  // Off
            OSTimeDlyHMSM(0, 0, 0, half_beat_period_ms, OS_OPT_TIME_HMSM_STRICT, &os_err);
        } else {
            PWM_Heartbeat_WriteCompare(0);  // Turn off if no heart rate
        }

        // Short delay to avoid excessive CPU usage in the main loop
        OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_HMSM_STRICT, &os_err);
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
static void LedG_Task(void *p_arg)
{
    (void)p_arg;
    OS_ERR os_err;

    while (DEF_TRUE) {
        if (toggle_state) {
            // Check if we are in blink mode
            if (GlobalGreen == GREEN_LED_BLINK) {
                // Toggle LED state
                if (gpio_read(PORT1, P1_2) == 1) {
                    gpio_low(PORT1, P1_2);  // Turn LED off
                } else {
                    gpio_high(PORT1, P1_2); // Turn LED on
                }
                
                // Delay to control blink rate
                OSTimeDlyHMSM(0, 0, 0, 250, OS_OPT_TIME_HMSM_NON_STRICT, &os_err);
            } else {
                // Ensure LED is on in non-blink mode
                gpio_high(PORT1, P1_2);
                
                // Delay to avoid unnecessary CPU usage
                OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_NON_STRICT, &os_err);
            }
        } else {
            // Turn off LED if toggle_state is inactive
            gpio_low(PORT1, P1_2);
            
            // Delay to avoid excessive CPU usage in main loop
            OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_NON_STRICT, &os_err);
        }
    }
}

