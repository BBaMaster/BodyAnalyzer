/*******************************************************************************
* File Name: PWM_Heartbeat_PM.c
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

#include "PWM_Heartbeat.h"

static PWM_Heartbeat_backupStruct PWM_Heartbeat_backup;


/*******************************************************************************
* Function Name: PWM_Heartbeat_SaveConfig
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
*  PWM_Heartbeat_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void PWM_Heartbeat_SaveConfig(void) 
{

    #if(!PWM_Heartbeat_UsingFixedFunction)
        #if(!PWM_Heartbeat_PWMModeIsCenterAligned)
            PWM_Heartbeat_backup.PWMPeriod = PWM_Heartbeat_ReadPeriod();
        #endif /* (!PWM_Heartbeat_PWMModeIsCenterAligned) */
        PWM_Heartbeat_backup.PWMUdb = PWM_Heartbeat_ReadCounter();
        #if (PWM_Heartbeat_UseStatus)
            PWM_Heartbeat_backup.InterruptMaskValue = PWM_Heartbeat_STATUS_MASK;
        #endif /* (PWM_Heartbeat_UseStatus) */

        #if(PWM_Heartbeat_DeadBandMode == PWM_Heartbeat__B_PWM__DBM_256_CLOCKS || \
            PWM_Heartbeat_DeadBandMode == PWM_Heartbeat__B_PWM__DBM_2_4_CLOCKS)
            PWM_Heartbeat_backup.PWMdeadBandValue = PWM_Heartbeat_ReadDeadTime();
        #endif /*  deadband count is either 2-4 clocks or 256 clocks */

        #if(PWM_Heartbeat_KillModeMinTime)
             PWM_Heartbeat_backup.PWMKillCounterPeriod = PWM_Heartbeat_ReadKillTime();
        #endif /* (PWM_Heartbeat_KillModeMinTime) */

        #if(PWM_Heartbeat_UseControl)
            PWM_Heartbeat_backup.PWMControlRegister = PWM_Heartbeat_ReadControlRegister();
        #endif /* (PWM_Heartbeat_UseControl) */
    #endif  /* (!PWM_Heartbeat_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: PWM_Heartbeat_RestoreConfig
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
*  PWM_Heartbeat_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_Heartbeat_RestoreConfig(void) 
{
        #if(!PWM_Heartbeat_UsingFixedFunction)
            #if(!PWM_Heartbeat_PWMModeIsCenterAligned)
                PWM_Heartbeat_WritePeriod(PWM_Heartbeat_backup.PWMPeriod);
            #endif /* (!PWM_Heartbeat_PWMModeIsCenterAligned) */

            PWM_Heartbeat_WriteCounter(PWM_Heartbeat_backup.PWMUdb);

            #if (PWM_Heartbeat_UseStatus)
                PWM_Heartbeat_STATUS_MASK = PWM_Heartbeat_backup.InterruptMaskValue;
            #endif /* (PWM_Heartbeat_UseStatus) */

            #if(PWM_Heartbeat_DeadBandMode == PWM_Heartbeat__B_PWM__DBM_256_CLOCKS || \
                PWM_Heartbeat_DeadBandMode == PWM_Heartbeat__B_PWM__DBM_2_4_CLOCKS)
                PWM_Heartbeat_WriteDeadTime(PWM_Heartbeat_backup.PWMdeadBandValue);
            #endif /* deadband count is either 2-4 clocks or 256 clocks */

            #if(PWM_Heartbeat_KillModeMinTime)
                PWM_Heartbeat_WriteKillTime(PWM_Heartbeat_backup.PWMKillCounterPeriod);
            #endif /* (PWM_Heartbeat_KillModeMinTime) */

            #if(PWM_Heartbeat_UseControl)
                PWM_Heartbeat_WriteControlRegister(PWM_Heartbeat_backup.PWMControlRegister);
            #endif /* (PWM_Heartbeat_UseControl) */
        #endif  /* (!PWM_Heartbeat_UsingFixedFunction) */
    }


/*******************************************************************************
* Function Name: PWM_Heartbeat_Sleep
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
*  PWM_Heartbeat_backup.PWMEnableState:  Is modified depending on the enable
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void PWM_Heartbeat_Sleep(void) 
{
    #if(PWM_Heartbeat_UseControl)
        if(PWM_Heartbeat_CTRL_ENABLE == (PWM_Heartbeat_CONTROL & PWM_Heartbeat_CTRL_ENABLE))
        {
            /*Component is enabled */
            PWM_Heartbeat_backup.PWMEnableState = 1u;
        }
        else
        {
            /* Component is disabled */
            PWM_Heartbeat_backup.PWMEnableState = 0u;
        }
    #endif /* (PWM_Heartbeat_UseControl) */

    /* Stop component */
    PWM_Heartbeat_Stop();

    /* Save registers configuration */
    PWM_Heartbeat_SaveConfig();
}


/*******************************************************************************
* Function Name: PWM_Heartbeat_Wakeup
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
*  PWM_Heartbeat_backup.pwmEnable:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_Heartbeat_Wakeup(void) 
{
     /* Restore registers values */
    PWM_Heartbeat_RestoreConfig();

    if(PWM_Heartbeat_backup.PWMEnableState != 0u)
    {
        /* Enable component's operation */
        PWM_Heartbeat_Enable();
    } /* Do nothing if component's block was disabled before */

}


/* [] END OF FILE */
