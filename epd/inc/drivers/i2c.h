#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stddef.h>
#include "utils/err.h"

typedef enum
{
    I2C_CLOCK_DIVIDER_2500   = 2500,      /*!< 2500 = 10us = 100 kHz */
    I2C_CLOCK_DIVIDER_626    = 626,       /*!< 622 = 2.504us = 399.3610 kHz */
    I2C_CLOCK_DIVIDER_150    = 150,       /*!< 150 = 60ns = 1.666 MHz (default at reset) */
    I2C_CLOCK_DIVIDER_148    = 148        /*!< 148 = 59ns = 1.689 MHz */
} i2c_clock_divider_t;

typedef struct
{
    uint8_t slaveAddr;
    i2c_clock_divider_t clockDivider;
} i2c_config_t;



pi_err_t i2c_init(i2c_config_t config);
pi_err_t i2c_read(uint8_t* rx, size_t len);
pi_err_t i2c_write(uint8_t* tx, size_t len);

#endif // I2C_H