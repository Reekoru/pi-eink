#include <stdint.h>
#include <stdio.h>
#include "drivers/gt911.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <bcm2835.h>
#include <string.h>
#include "project_settings.h"
#include "drivers/gpio.h"
#include "drivers/i2c.h"

#define GT911_ADDR 0x5D
#define I2C_BUS "/dev/i2c-1"

#define GT911_CONFIG_ADDR 0x8047
#define GT911_STATUS_ADDR 0x814E
#define GT911_TOUCH1_ADDR 0x8158

static uint8_t tx_buffer[256];
static uint8_t rx_buffer[256];

// https://www.orientdisplay.com/pdf/GT911.pdf
static uint8_t GT911_Config[] = {
    0x00,0xE0,0x01,0x20,0x03,0x0A,0x05,0x00,0x01,0x0F,
    0x28,0x0F,0x50,0x32,0x03,0x05,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x89,0x29,0x0A,
    0x52,0x50,0x0C,0x08,0x00,0x00,0x00,0x00,0x03,0x1D,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x32,0x00,0x00,
    0x00,0x48,0x70,0x94,0xC5,0x02,0x07,0x00,0x00,0x04,
    0x87,0x4B,0x00,0x7D,0x52,0x00,0x74,0x59,0x00,0x6B,
    0x62,0x00,0x64,0x6B,0x00,0x64,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x08,0x0A,0x0C,0x0E,0x10,0x12,0x14,0x16,
    0x18,0x1A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x2A,0x29,0x28,0x24,0x22,0x20,0x1F,0x1E,
    0x1D,0x0E,0x0C,0x0A,0x08,0x06,0x05,0x04,0x02,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0xAA,0x01
};

/* Private function prototypes */
static void _GT911_Init_GPIO(void);
static void _calculate_checksum(void);
static GT911_Status_t _GT911_SendConfig();
static GT911_Status_t GT911_ClearStatus(void);


/* Function definitions */
GT911_Status_t GT911_Reset(void)
{
    GPIO_SetDirOut(TP_INT_PIN);
    GPIO_SetLevel(TP_RST_PIN, LOW);
    delay(20);
    GPIO_SetLevel(TP_INT_PIN, LOW);
    delay(20);
    GPIO_SetLevel(TP_RST_PIN, HIGH);
    delay(50);
    GPIO_SetDirIn(TP_INT_PIN, true); // Leave floating input
    return GT911_OK;
}

GT911_Status_t GT911_Init(void)
{
    // Open I2C Bus
    puts("Opening I2C bus");
    I2C_Config_t config = {.slaveAddr = GT911_ADDR, .clockDivider = I2C_CLOCK_DIVIDER_2500};
    if(I2C_Init(config) != I2C_OK)
    {
        puts("Err");
        return GT911_ERR;
    }

    puts("Resetting GT911");
    GT911_Reset();
    delay(200); // Have to wait at least 50 ms before configuration
    GT911_ReadProductID();

    puts("Configuring GT911...");
    _calculate_checksum();
    if(_GT911_SendConfig() !=  I2C_OK)
    {
        puts("Something went wrong");
    }
    puts("Successfully configured");
    return GT911_OK;
}

/**
 * @brief Reads the register 0x814E which holds the status, large touch, Key, and # gestures
 * @param status Pointer to the output of the read command.
 * @return GT911_OK if successful, GT911_ERR if error
 */
GT911_Status_t GT911_ReadStatus(uint8_t *status)
{
    GPIO_SetDirIn(TP_INT_PIN, true);
    tx_buffer[0] = (GT911_STATUS_ADDR & 0xFF00) >> 8;
    tx_buffer[1] = (GT911_STATUS_ADDR & 0xFF);
    GT911_Status_t ret = I2C_Write(tx_buffer, 2); // Send to read from 0x814E
    
    if (ret != I2C_OK) return ret;

    ret = I2C_Read(rx_buffer, 1); // Read status register

    if (ret != I2C_OK) return ret;

    *status = rx_buffer[0];
    GT911_ClearStatus();
    return GT911_OK;
}

GT911_Status_t GT911_ReadTouch(GT911_Coordinates_t *coordinates, uint8_t *num_coordinates)
{
    uint8_t status;
    GT911_Status_t ret = GT911_ReadStatus(&status);
    if(ret != GT911_OK) return ret;
    if((status & 0x80) == 0) return GT911_STATUS_NREADY;

    puts("Ready");

    *num_coordinates = status & 0x0F;

    printf("Num coord: %d\n\r", *num_coordinates);

    for(uint8_t i = 0; i < (*num_coordinates); i++)
    {
        tx_buffer[0] = ((GT911_TOUCH1_ADDR + (8 * (i - 1))) & 0xFF00) >> 8; // Next gesture has 1 byte offset
        tx_buffer[1] = ((GT911_TOUCH1_ADDR + (8 * (i - 1))) & 0x00FF);
        I2C_Write(tx_buffer, 2);
        I2C_Read(rx_buffer, 6);

        coordinates->x = (rx_buffer[1] << 8) | rx_buffer[0];
        coordinates->y = (rx_buffer[3] << 8) | rx_buffer[2];
    }
    return GT911_OK;
}

void GT911_ReadProductID(void)
{
    tx_buffer[0] = 0x81;
    tx_buffer[1] = 0x40;
    I2C_Write(tx_buffer, 2);
    I2C_Read(rx_buffer, 4);
    printf("GT911 ID: %c%c%c%c\n",
           rx_buffer[0], rx_buffer[1],
           rx_buffer[2], rx_buffer[3]);
}




static GT911_Status_t _GT911_SendConfig()
{
    tx_buffer[0] = (GT911_CONFIG_ADDR & 0xFF00) >> 8;
    tx_buffer[1] = (GT911_CONFIG_ADDR & 0xFF);
    memcpy(&tx_buffer[2], GT911_Config, sizeof(GT911_Config));
    return I2C_Write(tx_buffer, sizeof(GT911_Config) + 2);
}

static GT911_Status_t GT911_ClearStatus(void)
{
    tx_buffer[0] = (GT911_STATUS_ADDR >> 8) & 0xFF;
    tx_buffer[1] = GT911_STATUS_ADDR & 0xFF;
    tx_buffer[2] = 0x00;
    return I2C_Write(tx_buffer, 3);
}

static void _calculate_checksum(void)
{
    uint8_t checksum = 0;
    for(uint8_t i = 0; i < 184; i++)
    {
        checksum += GT911_Config[i];
    }

    GT911_Config[184] = (~checksum) + 1;
    GT911_Config[185] = 0x01;
}