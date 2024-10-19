#ifndef BSP_I2C_PRESENT
#define BSP_I2C_PRESENT

#include <includes.h>

#define I2C_BUFFER_SIZE   256u    /* Define the size of the I2C buffer */
extern OS_SEM I2CTaskSem;

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

/* I2C Initialization function (task creation) */
void I2C_Init(void);

/* I2C Task Function */
static void I2C_Task(void *p_arg);

/* I2C Read Function */
uint8_t bsp_i2c_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);

/* I2C Write Function */
uint8_t bsp_i2c_write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len);

#endif  /* BSP_I2C_PRESENT */
