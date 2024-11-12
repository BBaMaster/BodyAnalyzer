/*******************************************************************************
* File Name: PWM_SPO2_PM.c
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

#include "PWM_SPO2.h"

static PWM_SPO2_backupStruct PWM_SPO2_backup;


/*******************************************************************************
* Function Name: PWM_SPO2_SaveConfig
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
*  PWM_SPO2_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void PWM_SPO2_SaveConfig(void) 
{

    #if(!PWM_SPO2_UsingFixedFunction)
        #if(!PWM_SPO2_PWMModeIsCenterAligned)
            PWM_SPO2_backup.PWMPeriod = PWM_SPO2_ReadPeriod();
        #endif /* (!PWM_SPO2_PWMModeIsCenterAligned) */
        PWM_SPO2_backup.PWMUdb = PWM_SPO2_ReadCounter();
        #if (PWM_SPO2_UseStatus)
            PWM_SPO2_backup.InterruptMaskValue = PWM_SPO2_STATUS_MASK;
        #endif /* (PWM_SPO2_UseStatus) */

        #if(PWM_SPO2_DeadBandMode == PWM_SPO2__B_PWM__DBM_256_CLOCKS || \
            PWM_SPO2_DeadBandMode == PWM_SPO2__B_PWM__DBM_2_4_CLOCKS)
            PWM_SPO2_backup.PWMdeadBandValue = PWM_SPO2_ReadDeadTime();
        #endif /*  deadband count is either 2-4 clocks or 256 clocks */

        #if(PWM_SPO2_KillModeMinTime)
             PWM_SPO2_backup.PWMKillCounterPeriod = PWM_SPO2_ReadKillTime();
        #endif /* (PWM_SPO2_KillModeMinTime) */

        #if(PWM_SPO2_UseControl)
            PWM_SPO2_backup.PWMControlRegister = PWM_SPO2_ReadControlRegister();
        #endif /* (PWM_SPO2_UseControl) */
    #endif  /* (!PWM_SPO2_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: PWM_SPO2_RestoreConfig
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
*  PWM_SPO2_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_SPO2_RestoreConfig(void) 
{
        #if(!PWM_SPO2_UsingFixedFunction)
            #if(!PWM_SPO2_PWMModeIsCenterAligned)
                PWM_SPO2_WritePeriod(PWM_SPO2_backup.PWMPeriod);
            #endif /* (!PWM_SPO2_PWMModeIsCenterAligned) */

            PWM_SPO2_WriteCounter(PWM_SPO2_backup.PWMUdb);

            #if (PWM_SPO2_UseStatus)
                PWM_SPO2_STATUS_MASK = PWM_SPO2_backup.InterruptMaskValue;
            #endif /* (PWM_SPO2_UseStatus) */

            #if(PWM_SPO2_DeadBandMode == PWM_SPO2__B_PWM__DBM_256_CLOCKS || \
                PWM_SPO2_DeadBandMode == PWM_SPO2__B_PWM__DBM_2_4_CLOCKS)
                PWM_SPO2_WriteDeadTime(PWM_SPO2_backup.PWMdeadBandValue);
            #endif /* deadband count is either 2-4 clocks or 256 clocks */

            #if(PWM_SPO2_KillModeMinTime)
                PWM_SPO2_WriteKillTime(PWM_SPO2_backup.PWMKillCounterPeriod);
            #endif /* (PWM_SPO2_KillModeMinTime) */

            #if(PWM_SPO2_UseControl)
                PWM_SPO2_WriteControlRegister(PWM_SPO2_backup.PWMControlRegister);
            #endif /* (PWM_SPO2_UseControl) */
        #endif  /* (!PWM_SPO2_UsingFixedFunction) */
    }


/*******************************************************************************
* Function Name: PWM_SPO2_Sleep
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
*  PWM_SPO2_backup.PWMEnableState:  Is modified depending on the enable
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void PWM_SPO2_Sleep(void) 
{
    #if(PWM_SPO2_UseControl)
        if(PWM_SPO2_CTRL_ENABLE == (PWM_SPO2_CONTROL & PWM_SPO2_CTRL_ENABLE))
        {
            /*Component is enabled */
            PWM_SPO2_backup.PWMEnableState = 1u;
        }
        else
        {
            /* Component is disabled */
            PWM_SPO2_backup.PWMEnableState = 0u;
        }
    #endif /* (PWM_SPO2_UseControl) */

    /* Stop component */
    PWM_SPO2_Stop();

    /* Save registers configuration */
    PWM_SPO2_SaveConfig();
}


/*******************************************************************************
* Function Name: PWM_SPO2_Wakeup
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
*  PWM_SPO2_backup.pwmEnable:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_SPO2_Wakeup(void) 
{
     /* Restore registers values */
    PWM_SPO2_RestoreConfig();

    if(PWM_SPO2_backup.PWMEnableState != 0u)
    {
        /* Enable component's operation */
        PWM_SPO2_Enable();
    } /* Do nothing if component's block was disabled before */

}


/* [] END OF FILE */
