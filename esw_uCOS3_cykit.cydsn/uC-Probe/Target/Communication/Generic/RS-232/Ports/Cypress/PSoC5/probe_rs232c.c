/*
*********************************************************************************************************
*                                       uC/Probe Communication
*
*                         (c) Copyright 2007-2008; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        COMMUNICATION: RS-232
*
*                                            PORT TEMPLATE
*
* Filename      : probe_rs232c.c
* Version       : V2.20
* Programmer(s) : FT
*                 DC
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*
* Note(s) : (1) A UART component labeled as 'ProbeUART' MUST be created in PSoC.
*
*           (2) If a different named is selected 'ProbeUART' must be replaced with the new name
*               for the UART component.
*********************************************************************************************************
*/

#include  <probe_com.h>
#include  <probe_rs232.h>
#include  <ProbeUART.h>
#include  <ProbeUART_RxISR.h>
#include  <ProbeUART_TxISR.h>


/*
*********************************************************************************************************
*                                               ENABLE
*
* Note(s) : (1) See 'probe_rs232.h  ENABLE'.
*********************************************************************************************************
*/

#if (PROBE_COM_CFG_RS232_EN == DEF_ENABLED)                     /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       ProbeRS232_InitTarget()
*
* Description : Initialize the UART for Probe communication.
*
* Argument(s) : baud_rate   Intended baud rate of the RS-232.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_Init().
*
* Note(s)     : (1) Though the baud rate used may vary from application to application or target to
*                   target, other communication settings are constamt.  The hardware must always be
*                   configured for the following :
*
*                   (a) No parity
*                   (b) One stop bit
*                   (c) Eight data bits.
*
*               (2) Neither receive nor transmit interrupts should be enabled by this function.
*********************************************************************************************************
*/

void  ProbeRS232_InitTarget (CPU_INT32U  baud_rate)
{
	ProbeUART_RxISR_Disable();                                      /* Disable Interrupts                   */
	ProbeUART_TxISR_Disable();
    ProbeUART_TxISR_ClearPending();                                 /* Clear Buffers on the ISRs            */
    ProbeUART_RxISR_ClearPending();
    
    ProbeUART_Start();                                              /* UART Initialization                  */
    ProbeUART_RxISR_Start();                                        /* RX Interrupt Handler Initialization  */
	ProbeUART_TxISR_Start();                                        /* TX Interrupt Handler Initialization  */
	ProbeUART_RxISR_Enable();                                       /* Enable RX Interrupt Service Routine  */
	ProbeUART_TxISR_Disable();                                      /* Disable TX Interrupt Service Routine */
}


/*
*********************************************************************************************************
*                                     ProbeRS232_RxTxISRHandler()
*
* Description : Handle receive and transmit interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : (1) This function may be empty if receive and transmit interrupts are handled in the
*                   separate interrupt handlers, ProbeRS232_RxISRHandler() and ProbeRS232_TxISRHandler().
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{

}


/*
*********************************************************************************************************
*                                      ProbeRS232_RxISRHandler()
*
* Description : Handle receive interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : (1) This function may be empty if receive interrupts are handled by the combined receive/
*                   transmit interrupt handler, 'ProbeRS232_RxTxISRHandler()'.
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
    CPU_INT08U  rx_data;
	
	
	rx_data = ProbeUART_GetByte();                                  /* Obtain RX Data                       */
	ProbeRS232_RxHandler(rx_data);
    
}


/*
*********************************************************************************************************
*                                        ProbeRS232_RxIntDis()
*
* Description : Disable receive interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) The application should call this function to stop communication.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntDis (void)
{
    ProbeUART_RxISR_Disable();
}


/*
*********************************************************************************************************
*                                        ProbeRS232_RxIntEn()
*
* Description : Enable receive interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) The application should call this function to start communication.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntEn (void)
{
    ProbeUART_RxISR_Enable();
}


/*
*********************************************************************************************************
*                                      ProbeRS232_TxISRHandler()
*
* Description : Handle transmit interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : (1) This function may be empty if transmit interrupts are handled by the combined receive/
*                   transmit interrupt handler, 'ProbeRS232_RxTxISRHandler()'.
*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{
    ProbeRS232_TxHandler();

}


/*
*********************************************************************************************************
*                                          ProbeRS232_Tx1()
*
* Description : Transmit one byte.
*
* Argument(s) : c           The byte to transmit.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_TxHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_Tx1 (CPU_INT08U c)
{
    ProbeUART_PutChar(c);                                           /* Place data in the TX Buffer          */
    ProbeRS232_TxIntEn();                                           /* Enable Interrupt for Transmission    */
}


/*
*********************************************************************************************************
*                                        ProbeRS232_TxIntDis()
*
* Description : Disable transmit interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_TxHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntDis (void)
{
    ProbeUART_TxISR_Disable();
}


/*
*********************************************************************************************************
*                                        ProbeRS232_TxIntEn()
*
* Description : Enable transmit interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_TxStart().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntEn (void)
{
    ProbeUART_TxISR_Enable();
}


/*
*********************************************************************************************************
*                                              ENABLE END
*
* Note(s) : See 'ENABLE  Note #1'.
*********************************************************************************************************
*/

#endif
