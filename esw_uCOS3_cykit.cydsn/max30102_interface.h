#ifndef MAX30102_INTERFACE_H
#define MAX30102_INTERFACE_H

#include <stdint.h>
#include <max30102/driver_max30102.h>
/* Function Prototypes */

/**
 * @brief Initialize and configure the MAX30102 sensor.
 * 
 * @return uint8_t Returns 0 if successful, otherwise error code.
 */
uint8_t max30102_sensor_init(max30102_handle_t* max30102_handle);
#endif /* MAX30102_INTERFACE_H */
