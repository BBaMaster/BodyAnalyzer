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
void BSP_PWM_set_Halfperiod(CPU_INT08U pwm, CPU_INT08U halfperiod)
{
  if (pwm == PWM_ac) PWM_ac_WriteCompare(halfperiod);
  else if (pwm == PWM_bo) PWM_bo_WriteCompare(halfperiod);
  else if (pwm == PWM_hb) PWM_hb_WriteCompare(halfperiod);
}

void BSP_PWM_set_Period(CPU_INT08U pwm, CPU_INT08U period)
{
  if (pwm == PWM_ac) PWM_ac_WritePeriod(period);
  else if (pwm == PWM_bo) PWM_bo_WritePeriod(period);
  else if (pwm == PWM_hb) PWM_hb_WritePeriod(period);
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
  if (pwm == PWM_ac) PWM_ac_Start();
  else if (pwm == PWM_bo) PWM_bo_Start();
  else if (pwm == PWM_hb) PWM_hb_Start();
}

void BSP_PWM_Stop(CPU_INT08U pwm)
{
  if (pwm == PWM_ac) PWM_ac_Stop();
  else if (pwm == PWM_bo) PWM_bo_Stop();
  else if (pwm == PWM_hb) PWM_hb_Stop();
}

/* [] END OF FILE */