#include <bme280_interface.h>
/*

void BME280_Init(void) {
    struct bme280_dev dev;

    // Configure the device structure for SPI
    dev.chip_id = BME280_I2C_ADDR_PRIM; // Set chip ID for identification
    dev.intf = BME280_SPI_INTF; // Set interface to SPI
    dev.read = bsp_spi_read; // Assign the SPI read function
    dev.write = bsp_spi_write; // Assign the SPI write function
    dev.delay_us = bsp_delay_ms; // Assign the delay function

    // Initialize the BME280 sensor
    int8_t result = bme280_init(&dev);
    if (result != BME280_OK) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to initialize BME280: %d\n", result);
    }
}

void BME280_Read_Data(void) {
    struct bme280_dev dev;
    struct bme280_data comp_data;

    // Read the compensated data from the sensor
    int8_t result = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
    if (result == BME280_OK) {
        // Process and log the data
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "Temp: %.2f C, Pressure: %.2f hPa, Humidity: %.2f %%\n",
                 comp_data.temperature, comp_data.pressure, comp_data.humidity);
        UART_1_PutString(buffer);
    } else {
        UART_1_PutString("Failed to read BME280 data\n");
    }
}
*/

