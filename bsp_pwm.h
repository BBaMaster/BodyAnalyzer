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

#define PWM1 1
#define PWM2 2

//Setzt die Wechselzeit des gewählten PWMs
void BSP_PWM_set_Halbperiode(CPU_INT08U pwm,
                             CPU_INT08U Ausgang,
                             CPU_INT08U halbperiode);

void BSP_PWM_Start(CPU_INT08U pwm); //Startet einen PWM
void BSP_PWM_Stop(CPU_INT08U pwm); //Startet einen PWM


/* [] END OF FILE */
