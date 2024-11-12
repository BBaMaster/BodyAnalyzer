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
    
    OS_ERR os_err;
    OS_MSG_SIZE msg_size;
    
    // Initialize package set to store information to transmit
    *data_environment_package = (DATA_SET_PACKAGE_ENVIRONMENT){0, 0, 0, 0};
    
    // Define a variable to hold the received data
    RAW_ENVIRONMENT_DATA *p_msg_SPI_data;

      // Convert and scale temperature
      data_environment_package->temperature_in_celsius = ((p_msg_SPI_data->temperature_raw * 12500) / 4095) - 4000;
      
      // Convert and scale humidity
      data_environment_package->humidity_in_percentage = (p_msg_SPI_data->humidity_raw * 10000) / 65535;
      
      // Convert and scale pressure
      data_environment_package->pressure_in_bar = ((p_msg_SPI_data->pressure_raw * 8000) / 65535) + 3000;
      
      // Convert gas concentration
      data_environment_package->gas_in_ohm = (p_msg_SPI_data->gas_raw * 1000) / 65535;
      
      return DEF_TRUE;
}

/* Helper function to check if environmental data is within acceptable indoor ranges */
CPU_BOOLEAN isEnvironmentDataInRange(DATA_SET_PACKAGE_ENVIRONMENT *data) {
    return (data->temperature_in_celsius >= TEMPERATURE_LEVEL_INDOOR_MIN && data->temperature_in_celsius <= TEMPERATURE_LEVEL_INDOOR_MAX) &&
           (data->humidity_in_percentage >= HUMIDITY_LEVEL_INDOOR_MIN && data->humidity_in_percentage <= HUMIDITY_LEVEL_INDOOR_MAX) &&
           (data->pressure_in_bar >= PRESSURE_LEVEL_INDOOR_MIN && data->pressure_in_bar <= PRESSURE_LEVEL_INDOOR_MAX) &&
           (data->gas_in_ohm >= GAS_LEVEL_INDOOR_MIN && data->gas_in_ohm <= GAS_LEVEL_INDOOR_MAX);
}
