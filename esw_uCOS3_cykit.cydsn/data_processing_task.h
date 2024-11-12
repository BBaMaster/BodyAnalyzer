#ifndef DATA_PROCESSING_TASK_H
#define DATA_PROCESSING_TASK_H
  
// Include necessary headers
#include <includes.h>
#include <log_task.h>

   
/* Acceptance ranges for humidity */
#define HUMIDITY_LEVEL_INDOOR_MIN 20
#define HUMIDITY_LEVEL_INDOOR_MAX 50
  
#define TEMPERATURE_LEVEL_INDOOR_MIN 20
#define TEMPERATURE_LEVEL_INDOOR_MAX 25

#define PRESSURE_LEVEL_INDOOR_MIN 100000
#define PRESSURE_LEVEL_INDOOR_MAX 105000

#define GAS_LEVEL_INDOOR_MIN 110000
#define GAS_LEVEL_INDOOR_MAX 521000
  

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

CPU_BOOLEAN processRawEnvironmentData(DATA_SET_PACKAGE_ENVIRONMENT *data_environment_package);
CPU_BOOLEAN retrieveSensorData(OS_Q *queue, CPU_INT08U *data_field, const char *sensor_name);
CPU_BOOLEAN isEnvironmentDataInRange(DATA_SET_PACKAGE_ENVIRONMENT *data_environment_package);
void initializeMessageQueuesDataProcessing();

  
#endif /* DATA_PROCESSING_TASK_H */