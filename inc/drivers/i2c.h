#ifndef I2C_H
#define I2C_H

#include <stdint.h>

typedef enum
{
    I2C_OK,
    I2C_ERR,
    I2C_ERR_NACK,
    I2C_ERR_CLKT,
    I2C_ERR_DATA
} I2C_Status_t;

typedef enum
{
    I2C_CLOCK_DIVIDER_2500   = 2500,      /*!< 2500 = 10us = 100 kHz */
    I2C_CLOCK_DIVIDER_626    = 626,       /*!< 622 = 2.504us = 399.3610 kHz */
    I2C_CLOCK_DIVIDER_150    = 150,       /*!< 150 = 60ns = 1.666 MHz (default at reset) */
    I2C_CLOCK_DIVIDER_148    = 148        /*!< 148 = 59ns = 1.689 MHz */
} I2C_ClockDivider_t;

typedef struct
{
    uint8_t slaveAddr;
    I2C_ClockDivider_t clockDivider;
} I2C_Config_t;



I2C_Status_t I2C_Init(I2C_Config_t config);
I2C_Status_t I2C_Read();
I2C_Status_t I2C_Write();

#endif // I2C_H