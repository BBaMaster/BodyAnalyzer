#include "log_task.h"
#include <app_cfg.h>
#include <stdio.h>  /* For snprintf */

/* Global variables */
static CPU_INT08U allowed_log_types = LOG_ALLOWED_LEVELS;  /* Bitmask for allowed log types */

/* Task Control Block and Stack */
OS_TCB Log_Task_TCB;
CPU_STK Log_TaskStk[APP_CFG_TASK_LOG_STK_SIZE];

/* Message queue for logs */
OS_Q Log_Queue;
OS_SEM LogTaskSem;

/* Message memory pool for logs */
OS_MEM MsgMemPool;
CPU_CHAR MsgMemPoolStorage[MESSAGE_POOL_SIZE][MESSAGE_SIZE];

/* Log task that handles UART output - forward declaration */
static void Log_Task(void *p_arg);

/* Helper function to print error messages directly via UART */
static void Log_PrintErrorWithCode(const char *msg, OS_ERR err_code)
{
    char buffer[LOG_MSG_SIZE];
    snprintf(buffer, sizeof(buffer), "%s (Error Code: %u)", msg, err_code);  // Print error code as unsigned integer
    UART_1_PutString(buffer);
    UART_1_PutString("\r\n");
}

/* Helper function to convert log level to string */
static const char* Log_LevelToString(CPU_INT08U level)
{
    switch (level) {
        case LOG_LEVEL_NONE:              return "NONE";
        case LOG_LEVEL_INFO:              return "INFO";
        case LOG_LEVEL_WARNING:           return "WARNING";
        case LOG_LEVEL_ERROR:             return "ERROR";
        case LOG_LEVEL_DEBUG:             return "DEBUG";
        case LOG_LEVEL_SPI:               return "SPI LOGGING";
        case LOG_LEVEL_I2C:               return "I2C LOGGING";
        case LOG_LEVEL_MAX30102:          return "MAX30102 LOGGING";
        case LOG_LEVEL_BME280:             return "BME280 LOGGING";
        default:                          return "UNKNOWN";
    }
}

/* Function to create the Log Task */
void Log_Init(void) {
    OS_ERR os_err;

    /* Create the log task, but leave the initialization to the task itself */
    OSTaskCreate((OS_TCB     *)&Log_Task_TCB,
                 (CPU_CHAR   *)"Log Task",
                 (OS_TASK_PTR )Log_Task,  
                 (void       *)0,
                 (OS_PRIO     )APP_CFG_TASK_LOG_PRIO,
                 (CPU_STK    *)&Log_TaskStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_LOG_STK_SIZE_LIMIT,
                 (CPU_STK_SIZE)APP_CFG_TASK_LOG_STK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&os_err);


    if (os_err != OS_ERR_NONE) { Log_PrintErrorWithCode("Error: Failed to create log task", os_err);
    }
}

/* Log task that handles memory allocation and message processing */
static void Log_Task(void *p_arg)
{
    OS_ERR os_err;
    OS_MSG_SIZE msg_size;
    CPU_TS ts;
    LOG_MSG *p_msg;

    (void)p_arg;

    /* Memory pool creation in the task */
    OSMemCreate(&MsgMemPool, "Log Message Pool", (void *)MsgMemPoolStorage, MESSAGE_POOL_SIZE, MESSAGE_SIZE, &os_err);
    if (os_err != OS_ERR_NONE) {
        Log_PrintErrorWithCode("Error: Failed to create log message pool", os_err);
        return;
    }
    
    /* Queue creation in the task */
    OSQCreate(&Log_Queue, "Log Queue", LOG_QUEUE_SIZE, &os_err);
    if (os_err != OS_ERR_NONE) {
        Log_PrintErrorWithCode("Error: Failed to create log queue", os_err);
        return;
    }
    OSSemPost(&LogTaskSem, OS_OPT_POST_1, &os_err);
    
    /* Log task loop */
    while (DEF_TRUE) {

        /* Wait for messages in the queue with infinite timeout */
        p_msg = (LOG_MSG *)OSQPend(&Log_Queue, 0, OS_OPT_PEND_BLOCKING, &msg_size, &ts, &os_err);

        if (os_err == OS_ERR_NONE) {
            /* Output the message if its type is allowed */
            if (p_msg->level & LOG_ALLOWED_LEVELS) {
                /* Print the log level followed by the message */
                UART_1_PutString(Log_LevelToString(p_msg->level));
                UART_1_PutString(": ");
                UART_1_PutString(p_msg->msg);
                UART_1_PutString("\r\n");

                /* Release the memory for the message after printing */
                OSMemPut(&MsgMemPool, (void *)p_msg, &os_err);
                if (os_err != OS_ERR_NONE) {
                    Log_PrintErrorWithCode("Error: Failed to release memory for log message", os_err);
                }
            }
        } else {
            Log_PrintErrorWithCode("Error: Failed to retrieve message from log queue", os_err);
        }
    }
}

/* Write a message to the log queue with a given severity level and optional error code */
void Log_Write(CPU_INT08U level, const CPU_CHAR *msg, ...) {
    // Check if the log level is valid and allowed
    if (!(level & LOG_ALLOWED_LEVELS) || (level == LOG_LEVEL_NONE)) {
        return;  // Skip logging if the message type is not allowed or is NONE
    }

    OS_ERR os_err;
    LOG_MSG *log_msg;
    CPU_CHAR formatted_msg[LOG_MSG_SIZE];
       
    /* Allocate memory for the log message */
    log_msg = (LOG_MSG *)OSMemGet(&MsgMemPool, &os_err);
    if (os_err != OS_ERR_NONE) {
        Log_PrintErrorWithCode("Error: Failed to allocate memory for log message", os_err);
        return;
    }

    /* Set the log message level */
    log_msg->level = level;
    if (!(level & LOG_ALLOWED_LEVELS) || level == LOG_LEVEL_NONE) {
    return;  // Skip logging if the message type is not allowed
    }
    /* Format the message with variable arguments */
    va_list args;
    va_start(args, msg);
    vsnprintf(formatted_msg, LOG_MSG_SIZE, msg, args);  // Use vsnprintf for variable argument formatting
    va_end(args);

    strncpy(log_msg->msg, formatted_msg, LOG_MSG_SIZE - 1);
    log_msg->msg[LOG_MSG_SIZE - 1] = '\0';  /* Ensure null termination */
    log_msg->level = level;
    if (!(level & LOG_ALLOWED_LEVELS) || level == LOG_LEVEL_NONE) {
    return;  // Skip logging if the message type is not allowed
    }
    /* Post the message to the queue */
    OSQPost(&Log_Queue, (void *)log_msg, sizeof(LOG_MSG), OS_OPT_POST_FIFO, &os_err);

    /* Handle errors in queue posting */
    if (os_err == OS_ERR_Q_FULL) {
        Log_PrintErrorWithCode("Error: Log queue is full", os_err);
        OSMemPut(&MsgMemPool, (void *)log_msg, &os_err);  /* Release memory on failure */
    } else if (os_err != OS_ERR_NONE) {
        Log_PrintErrorWithCode("Error: Failed to post message to log queue", os_err);
        OSMemPut(&MsgMemPool, (void *)log_msg, &os_err);  /* Release memory on failure */
    }
}