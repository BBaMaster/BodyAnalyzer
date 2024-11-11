/*******************************************************************************
* File Name: PWM_hb_PM.c
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

#include "PWM_hb.h"

static PWM_hb_backupStruct PWM_hb_backup;


/*******************************************************************************
* Function Name: PWM_hb_SaveConfig
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
*  PWM_hb_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void PWM_hb_SaveConfig(void) 
{

    #if(!PWM_hb_UsingFixedFunction)
        #if(!PWM_hb_PWMModeIsCenterAligned)
            PWM_hb_backup.PWMPeriod = PWM_hb_ReadPeriod();
        #endif /* (!PWM_hb_PWMModeIsCenterAligned) */
        PWM_hb_backup.PWMUdb = PWM_hb_ReadCounter();
        #if (PWM_hb_UseStatus)
            PWM_hb_backup.InterruptMaskValue = PWM_hb_STATUS_MASK;
        #endif /* (PWM_hb_UseStatus) */

        #if(PWM_hb_DeadBandMode == PWM_hb__B_PWM__DBM_256_CLOCKS || \
            PWM_hb_DeadBandMode == PWM_hb__B_PWM__DBM_2_4_CLOCKS)
            PWM_hb_backup.PWMdeadBandValue = PWM_hb_ReadDeadTime();
        #endif /*  deadband count is either 2-4 clocks or 256 clocks */

        #if(PWM_hb_KillModeMinTime)
             PWM_hb_backup.PWMKillCounterPeriod = PWM_hb_ReadKillTime();
        #endif /* (PWM_hb_KillModeMinTime) */

        #if(PWM_hb_UseControl)
            PWM_hb_backup.PWMControlRegister = PWM_hb_ReadControlRegister();
        #endif /* (PWM_hb_UseControl) */
    #endif  /* (!PWM_hb_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: PWM_hb_RestoreConfig
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
*  PWM_hb_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_hb_RestoreConfig(void) 
{
        #if(!PWM_hb_UsingFixedFunction)
            #if(!PWM_hb_PWMModeIsCenterAligned)
                PWM_hb_WritePeriod(PWM_hb_backup.PWMPeriod);
            #endif /* (!PWM_hb_PWMModeIsCenterAligned) */

            PWM_hb_WriteCounter(PWM_hb_backup.PWMUdb);

            #if (PWM_hb_UseStatus)
                PWM_hb_STATUS_MASK = PWM_hb_backup.InterruptMaskValue;
            #endif /* (PWM_hb_UseStatus) */

            #if(PWM_hb_DeadBandMode == PWM_hb__B_PWM__DBM_256_CLOCKS || \
                PWM_hb_DeadBandMode == PWM_hb__B_PWM__DBM_2_4_CLOCKS)
                PWM_hb_WriteDeadTime(PWM_hb_backup.PWMdeadBandValue);
            #endif /* deadband count is either 2-4 clocks or 256 clocks */

            #if(PWM_hb_KillModeMinTime)
                PWM_hb_WriteKillTime(PWM_hb_backup.PWMKillCounterPeriod);
            #endif /* (PWM_hb_KillModeMinTime) */

            #if(PWM_hb_UseControl)
                PWM_hb_WriteControlRegister(PWM_hb_backup.PWMControlRegister);
            #endif /* (PWM_hb_UseControl) */
        #endif  /* (!PWM_hb_UsingFixedFunction) */
    }


/*******************************************************************************
* Function Name: PWM_hb_Sleep
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
*  PWM_hb_backup.PWMEnableState:  Is modified depending on the enable
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void PWM_hb_Sleep(void) 
{
    #if(PWM_hb_UseControl)
        if(PWM_hb_CTRL_ENABLE == (PWM_hb_CONTROL & PWM_hb_CTRL_ENABLE))
        {
            /*Component is enabled */
            PWM_hb_backup.PWMEnableState = 1u;
        }
        else
        {
            /* Component is disabled */
            PWM_hb_backup.PWMEnableState = 0u;
        }
    #endif /* (PWM_hb_UseControl) */

    /* Stop component */
    PWM_hb_Stop();

    /* Save registers configuration */
    PWM_hb_SaveConfig();
}


/*******************************************************************************
* Function Name: PWM_hb_Wakeup
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
*  PWM_hb_backup.pwmEnable:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_hb_Wakeup(void) 
{
     /* Restore registers values */
    PWM_hb_RestoreConfig();

    if(PWM_hb_backup.PWMEnableState != 0u)
    {
        /* Enable component's operation */
        PWM_hb_Enable();
    } /* Do nothing if component's block was disabled before */

}


/* [] END OF FILE */
