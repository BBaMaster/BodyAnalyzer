#ifndef LOG_TASK_H
#define LOG_TASK_H

#include <includes.h>

/* Logging levels as bitmask */
#define LOG_LEVEL_NONE        0x00  /* No logging */
#define LOG_LEVEL_INFO        0x01  /* Informational messages */
#define LOG_LEVEL_WARNING     0x02  /* Warning messages */
#define LOG_LEVEL_ERROR       0x04  /* Error messages */
#define LOG_LEVEL_DEBUG       0x08  /* Debug messages */

/* Default allowed logging levels (set to all levels by default) */
#ifndef LOG_ALLOWED_LEVELS
#define LOG_ALLOWED_LEVELS    (LOG_LEVEL_INFO | LOG_LEVEL_WARNING | LOG_LEVEL_ERROR | LOG_LEVEL_DEBUG)
#endif
extern OS_SEM LogTaskSem;
/* Maximum size of a single log message (in characters) */
#define LOG_MSG_SIZE  80u

/* Macro to align size to the nearest multiple of 4 bytes */
#define ALIGN_TO_WORD_SIZE(x)  (((x) + 3u) & (~3u))

/* Aligned size of the message */
#define MESSAGE_SIZE ALIGN_TO_WORD_SIZE(sizeof(LOG_MSG))

/* Maximum number of messages in the log queue */
#define LOG_QUEUE_SIZE  100u

/* Maximum number of log messages in the memory pool */
#define MESSAGE_POOL_SIZE 84u

/* Message structure */
typedef struct {
    CPU_INT08U level;  // Log level (info, warning, error, etc.)
    CPU_CHAR   msg[LOG_MSG_SIZE];  // Message content
} LOG_MSG;

/* Task Control Block and Stack Declaration */
extern OS_TCB Log_Task_TCB;
extern CPU_STK Log_TaskStk[APP_CFG_TASK_LOG_STK_SIZE];

/* Function Prototypes */
void Log_Init(void);  // Initializes the log task (creates the task, not memory pool/queue)
void Log_Write(CPU_INT08U level, const CPU_CHAR *msg, ...);

#endif /* LOG_TASK_H */
