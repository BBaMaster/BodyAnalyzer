/*******************************************************************************
* File Name: PWM_bo_PM.c
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

#include "PWM_bo.h"

static PWM_bo_backupStruct PWM_bo_backup;


/*******************************************************************************
* Function Name: PWM_bo_SaveConfig
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
*  PWM_bo_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void PWM_bo_SaveConfig(void) 
{

    #if(!PWM_bo_UsingFixedFunction)
        #if(!PWM_bo_PWMModeIsCenterAligned)
            PWM_bo_backup.PWMPeriod = PWM_bo_ReadPeriod();
        #endif /* (!PWM_bo_PWMModeIsCenterAligned) */
        PWM_bo_backup.PWMUdb = PWM_bo_ReadCounter();
        #if (PWM_bo_UseStatus)
            PWM_bo_backup.InterruptMaskValue = PWM_bo_STATUS_MASK;
        #endif /* (PWM_bo_UseStatus) */

        #if(PWM_bo_DeadBandMode == PWM_bo__B_PWM__DBM_256_CLOCKS || \
            PWM_bo_DeadBandMode == PWM_bo__B_PWM__DBM_2_4_CLOCKS)
            PWM_bo_backup.PWMdeadBandValue = PWM_bo_ReadDeadTime();
        #endif /*  deadband count is either 2-4 clocks or 256 clocks */

        #if(PWM_bo_KillModeMinTime)
             PWM_bo_backup.PWMKillCounterPeriod = PWM_bo_ReadKillTime();
        #endif /* (PWM_bo_KillModeMinTime) */

        #if(PWM_bo_UseControl)
            PWM_bo_backup.PWMControlRegister = PWM_bo_ReadControlRegister();
        #endif /* (PWM_bo_UseControl) */
    #endif  /* (!PWM_bo_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: PWM_bo_RestoreConfig
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
*  PWM_bo_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_bo_RestoreConfig(void) 
{
        #if(!PWM_bo_UsingFixedFunction)
            #if(!PWM_bo_PWMModeIsCenterAligned)
                PWM_bo_WritePeriod(PWM_bo_backup.PWMPeriod);
            #endif /* (!PWM_bo_PWMModeIsCenterAligned) */

            PWM_bo_WriteCounter(PWM_bo_backup.PWMUdb);

            #if (PWM_bo_UseStatus)
                PWM_bo_STATUS_MASK = PWM_bo_backup.InterruptMaskValue;
            #endif /* (PWM_bo_UseStatus) */

            #if(PWM_bo_DeadBandMode == PWM_bo__B_PWM__DBM_256_CLOCKS || \
                PWM_bo_DeadBandMode == PWM_bo__B_PWM__DBM_2_4_CLOCKS)
                PWM_bo_WriteDeadTime(PWM_bo_backup.PWMdeadBandValue);
            #endif /* deadband count is either 2-4 clocks or 256 clocks */

            #if(PWM_bo_KillModeMinTime)
                PWM_bo_WriteKillTime(PWM_bo_backup.PWMKillCounterPeriod);
            #endif /* (PWM_bo_KillModeMinTime) */

            #if(PWM_bo_UseControl)
                PWM_bo_WriteControlRegister(PWM_bo_backup.PWMControlRegister);
            #endif /* (PWM_bo_UseControl) */
        #endif  /* (!PWM_bo_UsingFixedFunction) */
    }


/*******************************************************************************
* Function Name: PWM_bo_Sleep
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
*  PWM_bo_backup.PWMEnableState:  Is modified depending on the enable
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void PWM_bo_Sleep(void) 
{
    #if(PWM_bo_UseControl)
        if(PWM_bo_CTRL_ENABLE == (PWM_bo_CONTROL & PWM_bo_CTRL_ENABLE))
        {
            /*Component is enabled */
            PWM_bo_backup.PWMEnableState = 1u;
        }
        else
        {
            /* Component is disabled */
            PWM_bo_backup.PWMEnableState = 0u;
        }
    #endif /* (PWM_bo_UseControl) */

    /* Stop component */
    PWM_bo_Stop();

    /* Save registers configuration */
    PWM_bo_SaveConfig();
}


/*******************************************************************************
* Function Name: PWM_bo_Wakeup
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
*  PWM_bo_backup.pwmEnable:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_bo_Wakeup(void) 
{
     /* Restore registers values */
    PWM_bo_RestoreConfig();

    if(PWM_bo_backup.PWMEnableState != 0u)
    {
        /* Enable component's operation */
        PWM_bo_Enable();
    } /* Do nothing if component's block was disabled before */

}


/* [] END OF FILE */
