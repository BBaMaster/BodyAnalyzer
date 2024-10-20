#include "max30102_interface.h"
#include "bsp_i2c.h"
#include "log_task.h"
#include "project.h"
#include <stdarg.h>
#define MAX30102_REG_INTERRUPT_ENABLE_1          0x02        /**< interrupt enable 1 register */
#define MAX30102_REG_INTERRUPT_ENABLE_2          0x03        /**< interrupt enable 2 register */

/* Dummy I2C init and deinit */
static uint8_t bsp_i2c_init(void) { 
    Log_Write(LOG_LEVEL_I2C, "I2C init", 0);
    return 0;  /* Return success */
}

static uint8_t bsp_i2c_deinit(void) { 
    Log_Write(LOG_LEVEL_I2C, "I2C deinit", 0);
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
    switch (type) {
        case 1:
            UART_1_PutString("Temperature data received.\r\n");
            break;
        case 2:
            UART_1_PutString("Heart rate data received.\r\n");
            break;
        case 3:
            UART_1_PutString("SpO2 data received.\r\n");
            break;
        default:
            UART_1_PutString("Unknown data type received.\r\n");
            break;
    }
}


uint8_t max30102_sensor_init(max30102_handle_t* max30102_handle) {
    uint8_t res;
    
    // Link I2C functions
    DRIVER_MAX30102_LINK_IIC_INIT(max30102_handle, bsp_i2c_init);
    DRIVER_MAX30102_LINK_IIC_DEINIT(max30102_handle, bsp_i2c_deinit);
    DRIVER_MAX30102_LINK_IIC_READ(max30102_handle, bsp_i2c_read);
    DRIVER_MAX30102_LINK_IIC_WRITE(max30102_handle, bsp_i2c_write);
    DRIVER_MAX30102_LINK_DELAY_MS(max30102_handle, bsp_delay_ms);
    DRIVER_MAX30102_LINK_DEBUG_PRINT(max30102_handle, bsp_debug_print);
    DRIVER_MAX30102_LINK_RECEIVE_CALLBACK(max30102_handle, max30102_receive_callback);
    
    Log_Write(LOG_LEVEL_INFO, "Initializing MAX30102 sensor...", 0);
    res = max30102_init(max30102_handle);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to initialize MAX30102 sensor (Error Code: %d)", res);
        return res;
    }
    
    res = max30102_set_fifo_roll(max30102_handle,MAX30102_BOOL_TRUE);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to enable FIFO roll (Error Code: %d)", res);
        return res;
    }

    Log_Write(LOG_LEVEL_MAX30102, "Configuring FIFO sample averaging...\r\n", 0);
    res = max30102_set_fifo_sample_averaging(max30102_handle, MAX30102_SAMPLE_AVERAGING_16); // Change to desired averaging level
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to configure FIFO sample averaging", res);
        return res;
    }
    
    // Set sensor mode to Multi-LED mode
    res = max30102_set_mode(max30102_handle, MAX30102_MODE_MULTI_LED);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to set sensor mode (Error Code: %d)", res);
        return res;
    }

    // Configure SpO2 settings
    res = max30102_set_spo2_adc_range(max30102_handle, MAX30102_SPO2_ADC_RANGE_16384);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to configure SPO2 ADC range (Error Code: %d)", res);
        return res;
    }

    res = max30102_set_spo2_sample_rate(max30102_handle, MAX30102_SPO2_SAMPLE_RATE_3200_HZ);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to configure SPO2 sample rate (Error Code: %d)", res);
        return res;
    }

    res = max30102_set_adc_resolution(max30102_handle, MAX30102_ADC_RESOLUTION_18_BIT);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to configure ADC resolution (Error Code: %d)", res);
        return res;
    }

    // Set LED pulse amplitudes
    res = max30102_set_led_red_pulse_amplitude(max30102_handle, 0xFF);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to set Red LED pulse amplitude (Error Code: %d)", res);
        return res;
    }

    res = max30102_set_led_ir_pulse_amplitude(max30102_handle, 0xFF);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to set IR LED pulse amplitude (Error Code: %d)", res);
        return res;
    }

    // Configure Multi-LED slots
    res = max30102_set_slot(max30102_handle, MAX30102_SLOT_1, MAX30102_LED_RED);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to set Slot 1 for Red LED (Error Code: %d)", res);
        return res;
    }

    res = max30102_set_slot(max30102_handle, MAX30102_SLOT_2, MAX30102_LED_IR);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to set Slot 2 for IR LED (Error Code: %d)", res);
        return res;
    }

    // Enable interrupts
    res = max30102_set_interrupt(max30102_handle, MAX30102_INTERRUPT_PPG_RDY_EN, MAX30102_BOOL_TRUE);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to enable FIFO Data Ready interrupt (Error Code: %d)", res);
        return res;
    }

    res = max30102_set_interrupt(max30102_handle, MAX30102_INTERRUPT_FIFO_FULL_EN, MAX30102_BOOL_TRUE);
    if (res != 0) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to enable FIFO Almost Full interrupt (Error Code: %d)", res);
        return res;
    }
    Log_Write(LOG_LEVEL_INFO, "MAX30102 sensor initialized and configured successfully", 0);
    return 0;  // Successful initialization
}
