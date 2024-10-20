#include <stdio.h>
#include "bme280/bme280.h" // Include BME280 header
#include "bsp_spi.h"  // Include your SPI BSP for read/write functions
#include <log_task.h>


void BME280_Init(struct bme280_dev* dev) {
    // Configure the device structure for SPI
    dev->chip_id = BME280_CHIP_ID; // Set chip ID for identification
    dev->intf = BME280_SPI_INTF; // Set interface to SPI
    dev->read = bsp_spi_read; // Assign the SPI read function
    dev->write = bsp_spi_write; // Assign the SPI write function
    dev->delay_us = Delay_MS; // Assign the delay function

    // Log the device configuration
    Log_Write(LOG_LEVEL_BME280, "Configuring BME280: Chip ID = %d, Interface = SPI", dev->chip_id);

    // Initialize the BME280 sensor
    int8_t result = bme280_init(dev);  // Pass dev directly, not &dev
    if (result != BME280_OK) {
        Log_Write(LOG_LEVEL_ERROR, "Failed to initialize BME280: %d", result);
    } else {
        Log_Write(LOG_LEVEL_INFO, "BME280 initialized successfully", 0);
    }
}

