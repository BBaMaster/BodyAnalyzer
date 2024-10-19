#ifndef BSP_SPI_H
#define BSP_SPI_H

#include <project.h>
#include <includes.h>

/* Define buffer size for SPI transfers */
#define SPI_BUFFER_SIZE 16

/* Function Prototypes */
static int8_t bsp_spi_read(uint8_t *reg_addr, uint8_t *data, uint32_t len);
static int8_t bsp_spi_write(uint8_t *reg_addr, const uint8_t *data, uint32_t len);
void SPI_Task(void *p_arg);   // SPI Task function

#endif /* BSP_SPI_H */
