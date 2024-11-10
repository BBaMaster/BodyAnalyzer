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

#include <cpu.h>
#include <PWM1.h>
#include <PWM2.h>

#define PWM_hb 1
#define PWM_ac 2
#define PWM_bo 3

//Setzt die Wechselzeit des gewählten PWMs
void BSP_PWM_set_Halfperiod(CPU_INT08U pwm, CPU_INT08U halfperiod);
void BSP_PWM_set_Period(CPU_INT08U pwm, CPU_INT08U period);

void BSP_PWM_Start(CPU_INT08U pwm); //Startet einen PWM
void BSP_PWM_Stop(CPU_INT08U pwm); //Startet einen PWM


/* [] END OF FILE */