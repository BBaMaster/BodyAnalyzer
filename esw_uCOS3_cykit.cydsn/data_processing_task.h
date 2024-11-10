#ifndef DATA_PROCESSING_TASK_H
#define DATA_PROCESSING_TASK_H
  
// Include necessary headers
#include <includes.h>
#include <log_task.h>
  
/* Maximum number of messages in the log queue */
#define MESSAGE_QUEUE_SIZE  250u

#define CMD_RED_LED                   0x01
#define CMD_BLUE_LED                  0x02
#define CMD_GREEN_LED                 0x03

#define GREEN_LED_FULL_BRIGHTNESS     1
#define GREEN_LED_BLINK               2
   
/* Acceptance ranges for heart beat in bpm */
#define HEART_RATE_GOOD_MIN     60
#define HEART_RATE_GOOD_MAX     80
#define HEART_RATE_NORMAL_MIN   81
#define HEART_RATE_NORMAL_MAX   100
#define HEART_RATE_CRITICAL_MIN 101
#define HEART_RATE_CRITICAL_MAX 200  // Example upper limit for critical

/* Acceptance ranges for blood oxygen level in percentage */
#define BLOOD_OXYGEN_LEVEL_NORMAL_MIN 98
#define BLOOD_OXYGEN_LEVEL_NORMAL_MAX 100
#define BLOOD_OXYGEN_LEVEL_TOLERABLE_MIN 95
#define BLOOD_OXYGEN_LEVEL_TOLERABLE_MAX 97
#define BLOOD_OXYGEN_LEVEL_DECREASED_MIN 90
#define BLOOD_OXYGEN_LEVEL_DECREASED_MAX 94
 
/* Acceptance ranges for humidity */
#define HUMIDITY_LEVEL_INDOOR_MIN 30
#define HUMIDITY_LEVEL_INDOOR_MAX 50
  
#define TEMPERATURE_LEVEL_INDOOR_MIN 20
#define TEMPERATURE_LEVEL_INDOOR_MAX 25

#define PRESSURE_LEVEL_INDOOR_MIN 1000
#define PRESSURE_LEVEL_INDOOR_MAX 1013

#define GAS_LEVEL_INDOOR_MIN 150000
#define GAS_LEVEL_INDOOR_MAX 521000
  
  
/* Maximum number of messages in the log queue */
#define DATA_PROCESSING_QUEUE_SIZE  100u
  
/* pointer to queues  */
OS_Q   CommQI2CSPO2;
OS_Q   CommQI2CHeartRate;
OS_Q   CommQSPIData;
OS_Q   CommQProcessedOximeterData;
OS_Q   CommQProcessedEnvironmentData;

// Ready to use data set package
typedef struct {
    CPU_INT08U average_heart_rate_in_bpm;          // Stores heart rate/BPM
    CPU_INT08U blood_oxygen_level_in_percentage;   // Stores SPO2 value in %
} DATA_SET_PACKAGE_OXIMETER;

// To caputre raw environment values
typedef struct {
    CPU_INT16S temperature_raw;                   //RAW values
    CPU_INT32S humidity_raw;                      //RAW values
    CPU_INT32S pressure_raw;                      //RAW values
    CPU_INT32S gas_raw;                           //RAW values
} RAW_ENVIRONMENT_DATA;

// Ready to use data set package
typedef struct {
    CPU_INT16S temperature_in_celsius;             // Stores temperature in celsius degree
    CPU_INT32S humidity_in_percentage;             // Stores humidity level
    CPU_INT32S pressure_in_bar;                    // Pressure level in air in BAR
    CPU_INT32S gas_in_ohm;                         // gas level in ohm
} DATA_SET_PACKAGE_ENVIRONMENT;

typedef struct {
    CPU_INT08S led_cmd;
    CPU_INT08S mode_data;
} LED_CONTROL_MESSAGE;

/* Function prototypes */
void Data_Processing_Init(void);
CPU_BOOLEAN processRawOximeterData(DATA_SET_PACKAGE_OXIMETER *data_oximeter_package);
CPU_BOOLEAN processRawEnvironmentData(DATA_SET_PACKAGE_ENVIRONMENT *data_environment_package);
CPU_BOOLEAN retrieveSensorData(OS_Q *queue, CPU_INT08U *data_field, const char *sensor_name);
CPU_BOOLEAN isEnvironmentDataInRange(DATA_SET_PACKAGE_ENVIRONMENT *data_environment_package);
void initMessageQueues();

  
#endif /* DATA_PROCESSING_TASK_H */