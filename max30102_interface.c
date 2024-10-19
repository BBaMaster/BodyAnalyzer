#include "max30102_interface.h"
#include "bsp_i2c.h"
#include "log_task.h"
#include "project.h"
#include <stdarg.h>



/* Dummy I2C init and deinit */
static uint8_t bsp_i2c_init(void) { 
    Log_Write(LOG_LEVEL_DEBUG, "I2C init", 0);
    return 0;  /* Return success */
}

static uint8_t bsp_i2c_deinit(void) { 
    Log_Write(LOG_LEVEL_DEBUG, "I2C deinit", 0);
    return 0;  /* Return success */
}
void bsp_delay_ms(uint32_t ms) {
    OS_ERR err;
    
    /* Convert ms to hours, minutes, seconds, and milliseconds */
    uint32_t hours = ms / 3600000;
    ms %= 3600000;
    uint32_t minutes = ms / 60000;
    ms %= 60000;
    uint32_t seconds = ms / 1000;
    ms %= 1000;

    OSTimeDlyHMSM(hours, minutes, seconds, ms, OS_OPT_TIME_HMSM_STRICT, &err);
    
    if (err != OS_ERR_NONE) {
        Log_Write(LOG_LEVEL_ERROR, "Error in delay function", err);
    }
}
/* Debug print function compatible with driver */
void bsp_debug_print(const char *format, ...) {
    char buffer[LOG_MSG_SIZE];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    UART_1_PutString(buffer);  /* Output the formatted message */
}
void max30102_receive_callback(uint8_t type) {
    // This function is triggered when new data is received.

    // Assuming that 'type' indicates what data is being received:
    // For instance, let's say:
    // type == 1 -> temperature data
    // type == 2 -> heart rate data
    // type == 3 -> SpO2 data

    switch (type) {
        case 1:
            // Handle temperature data
            UART_1_PutString("Temperature data received.\r\n");
            break;
        case 2:
            // Handle heart rate data
            UART_1_PutString("Heart rate data received.\r\n");
            break;
        case 3:
            // Handle SpO2 data
            UART_1_PutString("SpO2 data received.\r\n");
            break;
        default:
            UART_1_PutString("Unknown data type received.\r\n");
            break;
    }

    // You can add further processing of the data based on the type.
}
/* Initialize the MAX30102 driver */
uint8_t max30102_sensor_init(max30102_handle_t* max30102_handle) {
    uint8_t res;
    
    UART_1_PutString("We are in sensor init\r\n");

    UART_1_PutString("Linked handler\r\n");
    // Link I2C functions
    DRIVER_MAX30102_LINK_IIC_INIT(max30102_handle, bsp_i2c_init);
    DRIVER_MAX30102_LINK_IIC_DEINIT(max30102_handle, bsp_i2c_deinit);
    DRIVER_MAX30102_LINK_IIC_READ(max30102_handle, bsp_i2c_read);
    DRIVER_MAX30102_LINK_IIC_WRITE(max30102_handle, bsp_i2c_write);
    DRIVER_MAX30102_LINK_RECEIVE_CALLBACK(max30102_handle, max30102_receive_callback);
    // Link delay and debug print functions
    DRIVER_MAX30102_LINK_DELAY_MS(max30102_handle, bsp_delay_ms);
    DRIVER_MAX30102_LINK_DEBUG_PRINT(max30102_handle, bsp_debug_print);

    Log_Write(LOG_LEVEL_INFO, "Initializing and configuring MAX30102 sensor", 0);
    // Initialize the sensor
    UART_1_PutString("Attempting sensor initialization...\r\n");
    res = max30102_init(max30102_handle);  // This is critical for proper sensor initialization
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to initialize MAX30102 sensor", res);
        return res;
    }
    UART_1_PutString("Sensor initialization successful.\r\n");

    // Configure the sensor's settings (Mode, SpO2 settings, etc.)
    Log_Write(LOG_LEVEL_DEBUG, "Setting sensor mode to SPO2...\r\n",0);
    res = max30102_set_mode(max30102_handle, MAX30102_MODE_SPO2);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to set sensor mode", res);
        return res;
    }

    Log_Write(LOG_LEVEL_DEBUG, "Configuring SPO2 ADC range...\r\n",0);
    res = max30102_set_spo2_adc_range(max30102_handle, MAX30102_SPO2_ADC_RANGE_16384);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to configure SPO2 ADC range", res);
        return res;
    }

    Log_Write(LOG_LEVEL_DEBUG, "Configuring SPO2 sample rate...\r\n",0);
    res = max30102_set_spo2_sample_rate(max30102_handle, MAX30102_SPO2_SAMPLE_RATE_100_HZ);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to configure SPO2 sample rate", res);
        return res;
    }

    Log_Write(LOG_LEVEL_DEBUG, "Configuring ADC resolution...\r\n",0);
    res = max30102_set_adc_resolution(max30102_handle, MAX30102_ADC_RESOLUTION_18_BIT);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to configure ADC resolution", res);
        return res;
    }

    Log_Write(LOG_LEVEL_DEBUG, "Setting LED pulse amplitudes...\r\n",0);
    res = max30102_set_led_red_pulse_amplitude(max30102_handle, 0x24);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to set Red LED pulse amplitude", res);
        return res;
    }

    res = max30102_set_led_ir_pulse_amplitude(max30102_handle, 0x24);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to set IR LED pulse amplitude", res);
        return res;
    }

    Log_Write(LOG_LEVEL_INFO, "MAX30102 sensor initialized and configured successfully", 0);
    return 0;
}
