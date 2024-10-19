#ifndef BSP_SPI_H
#define BSP_SPI_H

#include <project.h>
#include <includes.h>

/* Define buffer size for SPI transfers */
#define SPI_BUFFER_SIZE 16

/* Function Prototypes */
void SPI_Read(void);          // Function to start SPI read with DMA
void SPI_Write(CPU_INT08U* data, CPU_INT16U size);  // Function to write data over SPI
void SPI_Task(void *p_arg);   // SPI Task function

#endif /* BSP_SPI_H */
