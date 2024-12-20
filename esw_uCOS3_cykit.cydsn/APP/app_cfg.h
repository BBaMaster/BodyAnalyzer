/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                              (c) Copyright 2012; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      APPLICATION CONFIGURATION
*
*                                           CYPRESS PSoC5
*                                              with the
*                                     CY8CKIT-050 Development Kit
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : DC
*********************************************************************************************************
*/

#ifndef  APP_CFG_MODULE_PRESENT
#define  APP_CFG_MODULE_PRESENT
    
#include <stdio.h>


/*
*********************************************************************************************************
*                                       ADDITIONAL uC/MODULE ENABLES
*********************************************************************************************************
*/

#define  APP_CFG_PROBE_COM_EN                           DEF_ENABLED


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_PRIO                        2u
#define  APP_CFG_TASK_LED_PRIO                          6u
#define  APP_CFG_TASK_BUTTON_PRIO                       4u
#define  APP_CFG_TASK_LOG_PRIO                          3u
#define  APP_CFG_TASK_I2C_PRIO                          5u   // Priority for I2C Task
#define  APP_CFG_TASK_BME688_PRIO                       6u   // Priority for I2C Task
#define  APP_CFG_TASK_DATA_PROCESSING_PRIO              7u   // Priority for Data Processing Task
/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_STK_SIZE                    128u
#define  APP_CFG_TASK_LED_STK_SIZE                      256u
#define  APP_CFG_TASK_BUTTON_STK_SIZE                   128u
#define  APP_CFG_TASK_LOG_STK_SIZE                      256u
#define  APP_CFG_TASK_MAX30102_STK_SIZE                 512u   // Stack size for MAX30102 Task
#define  APP_CFG_TASK_BME688_STK_SIZE                   512u   // Stack size for SPI Task
#define  APP_CFG_TASK_SPI_STK_SIZE                      512u   // Stack size for SPI Task
#define  APP_CFG_TASK_DATA_PROCESSING_STK_SIZE          512u   // Stack size for Data Processing Task
/*
*********************************************************************************************************
*                                          TASK STACK SIZES LIMIT
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_STK_SIZE_PCT_FULL           90u
#define  APP_CFG_TASK_START_STK_SIZE_LIMIT         (APP_CFG_TASK_START_STK_SIZE    * (100u - APP_CFG_TASK_START_STK_SIZE_PCT_FULL))    / 100u

#define  APP_CFG_TASK_LED_STK_SIZE_PCT_FULL             90u
#define  APP_CFG_TASK_LED_STK_SIZE_LIMIT           (APP_CFG_TASK_LED_STK_SIZE  * (100u - APP_CFG_TASK_LED_STK_SIZE_PCT_FULL))  / 100u
  
#define  APP_CFG_TASK_BUTTON_STK_SIZE_PCT_FULL          90u
#define  APP_CFG_TASK_BUTTON_STK_SIZE_LIMIT           (APP_CFG_TASK_BUTTON_STK_SIZE  * (100u - APP_CFG_TASK_BUTTON_STK_SIZE_PCT_FULL))  / 100u
    
#define  APP_CFG_TASK_LOG_STK_SIZE_PCT_FULL             90u
#define  APP_CFG_TASK_LOG_STK_SIZE_LIMIT           (APP_CFG_TASK_LOG_STK_SIZE  * (100u - APP_CFG_TASK_LOG_STK_SIZE_PCT_FULL))  / 100u

#define  APP_CFG_TASK_MAX30102_STK_SIZE_PCT_FULL             90u
#define  APP_CFG_TASK_MAX30102_STK_SIZE_LIMIT           (APP_CFG_TASK_MAX30102_STK_SIZE  * (100u - APP_CFG_TASK_MAX30102_STK_SIZE_PCT_FULL))  / 100u
    
#define  APP_CFG_TASK_BME688_STK_SIZE_PCT_FULL             90u
#define  APP_CFG_TASK_BME688_STK_SIZE_LIMIT           (APP_CFG_TASK_BME688_STK_SIZE  * (100u - APP_CFG_TASK_BME688_STK_SIZE_PCT_FULL))  / 100u
#define  APP_CFG_TASK_SPI_STK_SIZE_PCT_FULL             90u
#define  APP_CFG_TASK_SPI_STK_SIZE_LIMIT           (APP_CFG_TASK_SPI_STK_SIZE  * (100u - APP_CFG_TASK_SPI_STK_SIZE_PCT_FULL))  / 100u
  
#define  APP_CFG_TASK_DATA_PROCESSING_STK_SIZE_PCT_FULL             90u
#define  APP_CFG_TASK_DATA_PROCESSING_STK_SIZE_LIMIT           (APP_CFG_TASK_DATA_PROCESSING_STK_SIZE  * (100u - APP_CFG_TASK_DATA_PROCESSING_STK_SIZE_PCT_FULL))  / 100u
/*
*********************************************************************************************************
*                                          uC/LIB CONFIGURATION
*********************************************************************************************************
*/
#define  LIB_MEM_CFG_ARG_CHK_EXT_EN                     DEF_DISABLED
#define  LIB_MEM_CFG_OPTIMIZE_ASM_EN                    DEF_DISABLED
#define  LIB_MEM_CFG_ALLOC_EN                           DEF_DISABLED
#define  LIB_MEM_CFG_HEAP_SIZE                          1024u
#define  LIB_STR_CFG_FP_EN                              DEF_DISABLED


/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/

#define  TRACE_LEVEL_OFF                                0
#define  TRACE_LEVEL_INFO                               1
#define  TRACE_LEVEL_DEBUG                              2

#define  APP_TRACE_LEVEL                                TRACE_LEVEL_OFF
#define  APP_TRACE                                      printf

#define  APP_TRACE_INFO(x)            ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_TRACE x) : (void)0)
#define  APP_TRACE_DEBUG(x)           ((APP_TRACE_LEVEL >= TRACE_LEVEL_DEBUG) ? (void)(APP_TRACE x) : (void)0)


#endif
