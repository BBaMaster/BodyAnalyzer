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

#include <bsp_pwm.h>

/*
Beschreibung: Setzt die Wechselzeit des gewählten PWMs auf die gesendete
              Halbperiode am gewählten Ausgang
Argumente: PWM Ist der PWM
           Ausgang ist der Ausgang des PWMs
           halbperiode ist die Halbperiode die auf den Ausgang des PWMs
                       geschrieben werden soll
Rückgabe: Nichts
Rufer: App_TaskPWM
Notizen: Keine
*/
void BSP_PWM_set_Halbperiode(CPU_INT08U pwm, CPU_INT08U Ausgang, CPU_INT08U halbperiode)
{
  if (pwm == PWM2)
  {
    if (Ausgang == 1)
      PWM2_WriteCompare1(halbperiode);
    else if (Ausgang == 2)
      PWM2_WriteCompare2(halbperiode);
  }
  else if (pwm == PWM1)
    PWM1_WriteCompare(halbperiode);
}

/*
Beschreibung: Startet einen PWM
Argumente: Gewählter PWM
Rückgabe: Nichts
Rufer: App_ObjCreate()
Notizen: Keine
*/
void BSP_PWM_Start(CPU_INT08U pwm)
{
  if (pwm == PWM1)
    PWM1_Start();
  else if (pwm == PWM2)
    PWM2_Start();
}

void BSP_PWM_Stop(CPU_INT08U pwm)
{
  if (pwm == PWM1) PWM1_Stop();
  else if (pwm == PWM2) PWM2_Stop();
}

/* [] END OF FILE */
