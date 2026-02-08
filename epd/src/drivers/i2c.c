#include "drivers/i2c.h"
#include <stdio.h>
#include <bcm2835.h>

pi_err_t i2c_init(i2c_config_t config)
{
    if(!bcm2835_i2c_begin())
    {
        puts("I2C failed to initialize");
        return PI_ERR;
    }

    bcm2835_i2c_setSlaveAddress(config.slaveAddr);
    bcm2835_i2c_setClockDivider(config.clockDivider);

    puts("Successfully initialized");
    return PI_OK;
}

pi_err_t i2c_write(uint8_t* tx, size_t len)
{
    uint8_t ret = bcm2835_i2c_write(tx, len);

    if(ret == BCM2835_I2C_REASON_ERROR_CLKT) return PI_ERR_CLKT;
    if(ret == BCM2835_I2C_REASON_ERROR_DATA) return PI_ERR_DATA;
    if(ret == BCM2835_I2C_REASON_ERROR_NACK) return PI_ERR_NACK;
    return PI_OK;
}
pi_err_t i2c_read(uint8_t* rx, size_t len)
{
    uint8_t ret = bcm2835_i2c_read(rx, len);

    if(ret == BCM2835_I2C_REASON_ERROR_CLKT) return PI_ERR_CLKT;
    if(ret == BCM2835_I2C_REASON_ERROR_DATA) return PI_ERR_DATA;
    if(ret == BCM2835_I2C_REASON_ERROR_NACK) return PI_ERR_NACK;
    return PI_OK;
}