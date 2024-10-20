#ifndef BSP_SPI_H
#define BSP_SPI_H

#include <project.h>
#include <includes.h>
#include <bme280/bme280.h>
#include <bme280_interface.h>
/* Define buffer size for SPI transfers */
#define SPI_BUFFER_SIZE 16

/* Function Prototypes */
int8_t bsp_spi_read(uint8_t reg_addr, uint8_t *data, uint32_t len);
int8_t bsp_spi_write(uint8_t reg_addr, const uint8_t *data, uint32_t len);
void SPI_Init(void);   // SPI Initialization function
void SPI_Task(void *p_arg); // SPI Task function
void Delay_MS(uint32_t ms);
int8_t read_bme280_chip_id2();
int8_t read_bme280_calibration_data();
#endif /* BSP_SPI_H */


