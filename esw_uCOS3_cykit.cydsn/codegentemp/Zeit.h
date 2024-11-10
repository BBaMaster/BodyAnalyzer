/*******************************************************************************
* File Name: Zeit.h
* Version 2.20
*
*  Description:
*   Provides the function and constant definitions for the clock component.
*
*  Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_CLOCK_Zeit_H)
#define CY_CLOCK_Zeit_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
* Conditional Compilation Parameters
***************************************/

/* Check to see if required defines such as CY_PSOC5LP are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5LP)
    #error Component cy_clock_v2_20 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5LP) */


/***************************************
*        Function Prototypes
***************************************/

void Zeit_Start(void) ;
void Zeit_Stop(void) ;

#if(CY_PSOC3 || CY_PSOC5LP)
void Zeit_StopBlock(void) ;
#endif /* (CY_PSOC3 || CY_PSOC5LP) */

void Zeit_StandbyPower(uint8 state) ;
void Zeit_SetDividerRegister(uint16 clkDivider, uint8 restart) 
                                ;
uint16 Zeit_GetDividerRegister(void) ;
void Zeit_SetModeRegister(uint8 modeBitMask) ;
void Zeit_ClearModeRegister(uint8 modeBitMask) ;
uint8 Zeit_GetModeRegister(void) ;
void Zeit_SetSourceRegister(uint8 clkSource) ;
uint8 Zeit_GetSourceRegister(void) ;
#if defined(Zeit__CFG3)
void Zeit_SetPhaseRegister(uint8 clkPhase) ;
uint8 Zeit_GetPhaseRegister(void) ;
#endif /* defined(Zeit__CFG3) */

#define Zeit_Enable()                       Zeit_Start()
#define Zeit_Disable()                      Zeit_Stop()
#define Zeit_SetDivider(clkDivider)         Zeit_SetDividerRegister(clkDivider, 1u)
#define Zeit_SetDividerValue(clkDivider)    Zeit_SetDividerRegister((clkDivider) - 1u, 1u)
#define Zeit_SetMode(clkMode)               Zeit_SetModeRegister(clkMode)
#define Zeit_SetSource(clkSource)           Zeit_SetSourceRegister(clkSource)
#if defined(Zeit__CFG3)
#define Zeit_SetPhase(clkPhase)             Zeit_SetPhaseRegister(clkPhase)
#define Zeit_SetPhaseValue(clkPhase)        Zeit_SetPhaseRegister((clkPhase) + 1u)
#endif /* defined(Zeit__CFG3) */


/***************************************
*             Registers
***************************************/

/* Register to enable or disable the clock */
#define Zeit_CLKEN              (* (reg8 *) Zeit__PM_ACT_CFG)
#define Zeit_CLKEN_PTR          ((reg8 *) Zeit__PM_ACT_CFG)

/* Register to enable or disable the clock */
#define Zeit_CLKSTBY            (* (reg8 *) Zeit__PM_STBY_CFG)
#define Zeit_CLKSTBY_PTR        ((reg8 *) Zeit__PM_STBY_CFG)

/* Clock LSB divider configuration register. */
#define Zeit_DIV_LSB            (* (reg8 *) Zeit__CFG0)
#define Zeit_DIV_LSB_PTR        ((reg8 *) Zeit__CFG0)
#define Zeit_DIV_PTR            ((reg16 *) Zeit__CFG0)

/* Clock MSB divider configuration register. */
#define Zeit_DIV_MSB            (* (reg8 *) Zeit__CFG1)
#define Zeit_DIV_MSB_PTR        ((reg8 *) Zeit__CFG1)

/* Mode and source configuration register */
#define Zeit_MOD_SRC            (* (reg8 *) Zeit__CFG2)
#define Zeit_MOD_SRC_PTR        ((reg8 *) Zeit__CFG2)

#if defined(Zeit__CFG3)
/* Analog clock phase configuration register */
#define Zeit_PHASE              (* (reg8 *) Zeit__CFG3)
#define Zeit_PHASE_PTR          ((reg8 *) Zeit__CFG3)
#endif /* defined(Zeit__CFG3) */


/**************************************
*       Register Constants
**************************************/

/* Power manager register masks */
#define Zeit_CLKEN_MASK         Zeit__PM_ACT_MSK
#define Zeit_CLKSTBY_MASK       Zeit__PM_STBY_MSK

/* CFG2 field masks */
#define Zeit_SRC_SEL_MSK        Zeit__CFG2_SRC_SEL_MASK
#define Zeit_MODE_MASK          (~(Zeit_SRC_SEL_MSK))

#if defined(Zeit__CFG3)
/* CFG3 phase mask */
#define Zeit_PHASE_MASK         Zeit__CFG3_PHASE_DLY_MASK
#endif /* defined(Zeit__CFG3) */

#endif /* CY_CLOCK_Zeit_H */


/* [] END OF FILE */
