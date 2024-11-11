/*******************************************************************************
* File Name: PWM_Environment_PM.c
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

#include "PWM_Environment.h"

static PWM_Environment_backupStruct PWM_Environment_backup;


/*******************************************************************************
* Function Name: PWM_Environment_SaveConfig
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
*  PWM_Environment_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void PWM_Environment_SaveConfig(void) 
{

    #if(!PWM_Environment_UsingFixedFunction)
        #if(!PWM_Environment_PWMModeIsCenterAligned)
            PWM_Environment_backup.PWMPeriod = PWM_Environment_ReadPeriod();
        #endif /* (!PWM_Environment_PWMModeIsCenterAligned) */
        PWM_Environment_backup.PWMUdb = PWM_Environment_ReadCounter();
        #if (PWM_Environment_UseStatus)
            PWM_Environment_backup.InterruptMaskValue = PWM_Environment_STATUS_MASK;
        #endif /* (PWM_Environment_UseStatus) */

        #if(PWM_Environment_DeadBandMode == PWM_Environment__B_PWM__DBM_256_CLOCKS || \
            PWM_Environment_DeadBandMode == PWM_Environment__B_PWM__DBM_2_4_CLOCKS)
            PWM_Environment_backup.PWMdeadBandValue = PWM_Environment_ReadDeadTime();
        #endif /*  deadband count is either 2-4 clocks or 256 clocks */

        #if(PWM_Environment_KillModeMinTime)
             PWM_Environment_backup.PWMKillCounterPeriod = PWM_Environment_ReadKillTime();
        #endif /* (PWM_Environment_KillModeMinTime) */

        #if(PWM_Environment_UseControl)
            PWM_Environment_backup.PWMControlRegister = PWM_Environment_ReadControlRegister();
        #endif /* (PWM_Environment_UseControl) */
    #endif  /* (!PWM_Environment_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: PWM_Environment_RestoreConfig
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
*  PWM_Environment_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_Environment_RestoreConfig(void) 
{
        #if(!PWM_Environment_UsingFixedFunction)
            #if(!PWM_Environment_PWMModeIsCenterAligned)
                PWM_Environment_WritePeriod(PWM_Environment_backup.PWMPeriod);
            #endif /* (!PWM_Environment_PWMModeIsCenterAligned) */

            PWM_Environment_WriteCounter(PWM_Environment_backup.PWMUdb);

            #if (PWM_Environment_UseStatus)
                PWM_Environment_STATUS_MASK = PWM_Environment_backup.InterruptMaskValue;
            #endif /* (PWM_Environment_UseStatus) */

            #if(PWM_Environment_DeadBandMode == PWM_Environment__B_PWM__DBM_256_CLOCKS || \
                PWM_Environment_DeadBandMode == PWM_Environment__B_PWM__DBM_2_4_CLOCKS)
                PWM_Environment_WriteDeadTime(PWM_Environment_backup.PWMdeadBandValue);
            #endif /* deadband count is either 2-4 clocks or 256 clocks */

            #if(PWM_Environment_KillModeMinTime)
                PWM_Environment_WriteKillTime(PWM_Environment_backup.PWMKillCounterPeriod);
            #endif /* (PWM_Environment_KillModeMinTime) */

            #if(PWM_Environment_UseControl)
                PWM_Environment_WriteControlRegister(PWM_Environment_backup.PWMControlRegister);
            #endif /* (PWM_Environment_UseControl) */
        #endif  /* (!PWM_Environment_UsingFixedFunction) */
    }


/*******************************************************************************
* Function Name: PWM_Environment_Sleep
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
*  PWM_Environment_backup.PWMEnableState:  Is modified depending on the enable
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void PWM_Environment_Sleep(void) 
{
    #if(PWM_Environment_UseControl)
        if(PWM_Environment_CTRL_ENABLE == (PWM_Environment_CONTROL & PWM_Environment_CTRL_ENABLE))
        {
            /*Component is enabled */
            PWM_Environment_backup.PWMEnableState = 1u;
        }
        else
        {
            /* Component is disabled */
            PWM_Environment_backup.PWMEnableState = 0u;
        }
    #endif /* (PWM_Environment_UseControl) */

    /* Stop component */
    PWM_Environment_Stop();

    /* Save registers configuration */
    PWM_Environment_SaveConfig();
}


/*******************************************************************************
* Function Name: PWM_Environment_Wakeup
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
*  PWM_Environment_backup.pwmEnable:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_Environment_Wakeup(void) 
{
     /* Restore registers values */
    PWM_Environment_RestoreConfig();

    if(PWM_Environment_backup.PWMEnableState != 0u)
    {
        /* Enable component's operation */
        PWM_Environment_Enable();
    } /* Do nothing if component's block was disabled before */

}


/* [] END OF FILE */
