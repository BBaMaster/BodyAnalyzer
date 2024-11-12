#include "data_processing_task.h"
#include <app_cfg.h>
#include "log_task.h"
#include "os.h"

/**
 @brief Processes the environment sensor values out of the queue straight into the processed struct
        
        This helper function expects a structs address as pointer to fullfill it with the processed clean values.
        Fruthermore this tasks receives the raw sensor values from the environment sensor and coverts them to the expected value
        format. After the process it stores the new values into the struct as a complete dataset.
        
 @param DATA_SET_PACKAGE_ENVIRONMENT *data_environment_package
**/
CPU_BOOLEAN processRawEnvironmentData(DATA_SET_PACKAGE_ENVIRONMENT *data_environment_package) {
   
      // Convert and scale temperature
      data_environment_package->temperature_in_celsius = data_environment_package->temperature_in_celsius / 100;
      
      // Convert and scale humidity
      data_environment_package->humidity_in_percentage = data_environment_package->humidity_in_percentage/1000;
      
      // Convert and scale pressure
      data_environment_package->pressure_in_bar = data_environment_package->pressure_in_bar;
      
      // Convert gas concentration
      data_environment_package->gas_in_ohm = data_environment_package->gas_in_ohm;
      
      return DEF_TRUE;
}

/* Helper function to check if environmental data is within acceptable indoor ranges */
CPU_BOOLEAN isEnvironmentDataInRange(DATA_SET_PACKAGE_ENVIRONMENT *data) {
    char log_message[128];
    snprintf(log_message, sizeof(log_message),
             "Environment Data - Temperature: %d °C, Humidity: %d %%, Pressure: %d bar, Gas Resistance: %d ohm\n",
             data->temperature_in_celsius,
             data->humidity_in_percentage,
             data->pressure_in_bar,
             data->gas_in_ohm)
    ;
    UART_1_PutString(log_message);
        if (data->temperature_in_celsius >= TEMPERATURE_LEVEL_INDOOR_MIN && data->temperature_in_celsius <= TEMPERATURE_LEVEL_INDOOR_MAX){
            if (data->humidity_in_percentage >= HUMIDITY_LEVEL_INDOOR_MIN && data->humidity_in_percentage <= HUMIDITY_LEVEL_INDOOR_MAX){
                if(data->pressure_in_bar >= PRESSURE_LEVEL_INDOOR_MIN && data->pressure_in_bar <= PRESSURE_LEVEL_INDOOR_MAX){
                    if(data->gas_in_ohm >= GAS_LEVEL_INDOOR_MIN && data->gas_in_ohm <= GAS_LEVEL_INDOOR_MAX){
                        return 0;
                    }
                }
            }
        } 
        return 1;
}
