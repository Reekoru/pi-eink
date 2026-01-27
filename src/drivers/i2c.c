#include "drivers/i2c.h"
#include <stdio.h>
#include <bcm2835.h>

I2C_Status_t I2C_Init(I2C_Config_t config)
{
    if(!bcm2835_i2c_begin())
    {
        puts("I2C failed to initialize");
        return I2C_ERR;
    }

    bcm2835_i2c_setSlaveAddress(config.slaveAddr);
    bcm2835_i2c_setClockDivider(config.clockDivider);

    puts("Successfully initialized");
    return I2C_OK;
}

I2C_Status_t I2C_Write(uint8_t* tx, size_t len)
{
    uint8_t ret = bcm2835_i2c_write(tx, len);

    if(ret == BCM2835_I2C_REASON_ERROR_CLKT) return I2C_ERR_CLKT;
    if(ret == BCM2835_I2C_REASON_ERROR_DATA) return I2C_ERR_DATA;
    if(ret == BCM2835_I2C_REASON_ERROR_NACK) return I2C_ERR_NACK;
    return I2C_OK;
}
I2C_Status_t I2C_Read(uint8_t* rx, size_t len)
{
    uint8_t ret = bcm2835_i2c_read(rx, len);

    if(ret == BCM2835_I2C_REASON_ERROR_CLKT) return I2C_ERR_CLKT;
    if(ret == BCM2835_I2C_REASON_ERROR_DATA) return I2C_ERR_DATA;
    if(ret == BCM2835_I2C_REASON_ERROR_NACK) return I2C_ERR_NACK;
    return I2C_OK;
}