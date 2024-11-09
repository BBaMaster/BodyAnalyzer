#ifndef DATA_PROCESSING_TASK_H
#define DATA_PROCESSING_TASK_H
  
// Include necessary headers
#include <includes.h>
#include <log_task.h>
  
/* Maximum number of messages in the log queue */
#define MESSAGE_QUEUE_SIZE  100u
#define CMD_RED 0x01
#define CMD_BLUE 0x02
#define CMD_GREEN 0x03
  
  
/* Acceptance ranges for heartbeat in bpm */
  
#define HEART_RATE_GOOD_MIN     60
#define HEART_RATE_GOOD_MAX     80
#define HEART_RATE_NORMAL_MIN   81
#define HEART_RATE_NORMAL_MAX   100
#define HEART_RATE_CRITICAL_MIN 101
#define HEART_RATE_CRITICAL_MAX 200  // Example upper limit for critical
  
  
  /* Maximum number of messages in the log queue */
#define DATA_PROCESSING_QUEUE_SIZE  10u
  
/* pointer to queues  */
OS_Q   CommQI2C;
OS_Q   CommQSPI;
OS_Q   CommQProcessDataOximeter;
OS_Q   CommQProcessDataEnvironment;

/* Message structure for processed data */
// 
typedef struct {
    CPU_INT32U hours;
    CPU_INT32U minutes;
    CPU_INT32U seconds;
    CPU_INT32U milliseconds;
} ELAPSED_TIME_SET;

// Define a struct for oximeter data
typedef struct {
    CPU_INT08S command;
    CPU_INT08S average_heart_rate_in_bpm;   // Stores BPM value
    ELAPSED_TIME_SET elapsed_time;

} DATA_SET_PACKAGE_OXIMETER;

// Define a struct for environment data
typedef struct {
    CPU_INT08S command;
    ELAPSED_TIME_SET elapsed_time;

} DATA_SET_PACKAGE_ENVIRONMENT;

typedef enum {
    HEART_RATE_GOOD,
    HEART_RATE_NORMAL,
    HEART_RATE_CRITICAL,
    HEART_RATE_OUT_OF_RANGE
} acceptanceRatesCategory;

/* Function prototypes */
void Data_Processing_Init(void);
CPU_BOOLEAN ProcessRawOximeterData(DATA_SET_PACKAGE_OXIMETER *data_oximeter_package);
CPU_BOOLEAN ProcessRawEnvironmentData(DATA_SET_PACKAGE_ENVIRONMENT *data_environment_package);
acceptanceRatesCategory isDataInAccceptableRange(CPU_INT08S average_heart_rate);
void initializeMessageQueues(void);
ELAPSED_TIME_SET conversionOfTimestamp();


  
#endif /* DATA_PROCESSING_TASK_H */