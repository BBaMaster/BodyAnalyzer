/*******************************************************************************
* File Name: PWM_ac_PM.c
* Version 3.30
*
* Description:
*  This file provides the power management source code to API for the
*  PWM.
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "PWM_ac.h"

static PWM_ac_backupStruct PWM_ac_backup;


/*******************************************************************************
* Function Name: PWM_ac_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_ac_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void PWM_ac_SaveConfig(void) 
{

    #if(!PWM_ac_UsingFixedFunction)
        #if(!PWM_ac_PWMModeIsCenterAligned)
            PWM_ac_backup.PWMPeriod = PWM_ac_ReadPeriod();
        #endif /* (!PWM_ac_PWMModeIsCenterAligned) */
        PWM_ac_backup.PWMUdb = PWM_ac_ReadCounter();
        #if (PWM_ac_UseStatus)
            PWM_ac_backup.InterruptMaskValue = PWM_ac_STATUS_MASK;
        #endif /* (PWM_ac_UseStatus) */

        #if(PWM_ac_DeadBandMode == PWM_ac__B_PWM__DBM_256_CLOCKS || \
            PWM_ac_DeadBandMode == PWM_ac__B_PWM__DBM_2_4_CLOCKS)
            PWM_ac_backup.PWMdeadBandValue = PWM_ac_ReadDeadTime();
        #endif /*  deadband count is either 2-4 clocks or 256 clocks */

        #if(PWM_ac_KillModeMinTime)
             PWM_ac_backup.PWMKillCounterPeriod = PWM_ac_ReadKillTime();
        #endif /* (PWM_ac_KillModeMinTime) */

        #if(PWM_ac_UseControl)
            PWM_ac_backup.PWMControlRegister = PWM_ac_ReadControlRegister();
        #endif /* (PWM_ac_UseControl) */
    #endif  /* (!PWM_ac_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: PWM_ac_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_ac_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_ac_RestoreConfig(void) 
{
        #if(!PWM_ac_UsingFixedFunction)
            #if(!PWM_ac_PWMModeIsCenterAligned)
                PWM_ac_WritePeriod(PWM_ac_backup.PWMPeriod);
            #endif /* (!PWM_ac_PWMModeIsCenterAligned) */

            PWM_ac_WriteCounter(PWM_ac_backup.PWMUdb);

            #if (PWM_ac_UseStatus)
                PWM_ac_STATUS_MASK = PWM_ac_backup.InterruptMaskValue;
            #endif /* (PWM_ac_UseStatus) */

            #if(PWM_ac_DeadBandMode == PWM_ac__B_PWM__DBM_256_CLOCKS || \
                PWM_ac_DeadBandMode == PWM_ac__B_PWM__DBM_2_4_CLOCKS)
                PWM_ac_WriteDeadTime(PWM_ac_backup.PWMdeadBandValue);
            #endif /* deadband count is either 2-4 clocks or 256 clocks */

            #if(PWM_ac_KillModeMinTime)
                PWM_ac_WriteKillTime(PWM_ac_backup.PWMKillCounterPeriod);
            #endif /* (PWM_ac_KillModeMinTime) */

            #if(PWM_ac_UseControl)
                PWM_ac_WriteControlRegister(PWM_ac_backup.PWMControlRegister);
            #endif /* (PWM_ac_UseControl) */
        #endif  /* (!PWM_ac_UsingFixedFunction) */
    }


/*******************************************************************************
* Function Name: PWM_ac_Sleep
********************************************************************************
*
* Summary:
*  Disables block's operation and saves the user configuration. Should be called
*  just prior to entering sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_ac_backup.PWMEnableState:  Is modified depending on the enable
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void PWM_ac_Sleep(void) 
{
    #if(PWM_ac_UseControl)
        if(PWM_ac_CTRL_ENABLE == (PWM_ac_CONTROL & PWM_ac_CTRL_ENABLE))
        {
            /*Component is enabled */
            PWM_ac_backup.PWMEnableState = 1u;
        }
        else
        {
            /* Component is disabled */
            PWM_ac_backup.PWMEnableState = 0u;
        }
    #endif /* (PWM_ac_UseControl) */

    /* Stop component */
    PWM_ac_Stop();

    /* Save registers configuration */
    PWM_ac_SaveConfig();
}


/*******************************************************************************
* Function Name: PWM_ac_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration. Should be called just after
*  awaking from sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_ac_backup.pwmEnable:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_ac_Wakeup(void) 
{
     /* Restore registers values */
    PWM_ac_RestoreConfig();

    if(PWM_ac_backup.PWMEnableState != 0u)
    {
        /* Enable component's operation */
        PWM_ac_Enable();
    } /* Do nothing if component's block was disabled before */

}


/* [] END OF FILE */
