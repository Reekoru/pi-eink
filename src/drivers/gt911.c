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

#define GT911_ADDR 0x14
#define I2C_BUS "/dev/i2c-1"

#define GT911_CONFIG_REG 0x8047
#define GT911_STATUS_REG 0x814E
#define GT911_TOUCH1_REG 0x8158
#define GT911_COMMAND_CHECK_REG 0x8046
#define GT911_COMMAND_REG 0x8040
#define GT911_GESTURE_REG 0x814B

#define GT911_FIRMWARE_REG 0x8144

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

static const uint8_t tap_threshold = 5;

/* Private function prototypes */
static void _calculate_checksum(void);
static GT911_Status_t _GT911_SendConfig();
static GT911_Status_t GT911_ReadFirmware(uint16_t *firmware);
static void TouchPoint_GetGesture(GT911_TouchPoint_t *touch);

void TouchPoint_Init(GT911_TouchPoint_t *touch)
{
    touch->coordinate.x = 0;
    touch->coordinate.y = 0;
    touch->coordinate.x_prev = 0;
    touch->coordinate.y_prev = 0;
    touch->gesture = GESTURE_NONE;
    touch->touch_id = 0;
    touch->is_touched = false;
    touch->has_moved = false;
    touch->touch_released_fptr = NULL;
}


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

GT911_Status_t GT911_Init(GT911_Config_t config)
{
    // Open I2C Bus
    puts("Opening I2C bus");
    I2C_Config_t i2c_config = {.slaveAddr = GT911_ADDR, .clockDivider = I2C_CLOCK_DIVIDER_2500};
    if(I2C_Init(i2c_config) != I2C_OK)
    {
        puts("Err");
        return GT911_ERR;
    }

    puts("Resetting GT911");
    GT911_Reset();
    delay(200); // Have to wait at least 50 ms before configuration
    uint16_t firmware;
    GT911_ReadProductID();
    GT911_ReadFirmware(&firmware);

    printf("Firmware version: %4x\r\n", firmware);

    puts("Configuring GT911...");

    // Configure resolution
    GT911_Config[1] = config.x_resolution & 0xFF;
    GT911_Config[2] = (config.x_resolution & 0xFF00) >> 8;
    GT911_Config[3] = config.y_resolution & 0xFF;
    GT911_Config[4] = (config.y_resolution & 0xFF00) >> 8;

    GT911_Config[5] = config.num_touch_points & 0x0F;

    GT911_Config[6] = 0;
	GT911_Config[6] |= config.reverse_y << 7;
	GT911_Config[6] |= config.reverse_x << 6;
	GT911_Config[6] |= config.switch_xy << 3;
	GT911_Config[6] |= config.sw_noise_reduction << 2;

    _calculate_checksum();

    GT911_Reset();
    delay(100);

    if(_GT911_SendConfig() !=  I2C_OK)
    {
        puts("Something went wrong");
    }
    
    GT911_SetCommand(config.mode);
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
    tx_buffer[0] = (GT911_STATUS_REG & 0xFF00) >> 8;
    tx_buffer[1] = (GT911_STATUS_REG & 0xFF);
    GT911_Status_t ret = I2C_Write(tx_buffer, 2); // Send to read from 0x814E
    if (ret != I2C_OK) return ret;
    ret = I2C_Read(rx_buffer, 1); // Read status register
    if (ret != I2C_OK) return ret;
    *status = rx_buffer[0];
    return GT911_OK;
}

/**
 * @brief Reads the touch and provides the coordinate and gesture. Executes the registered OnTouchReleasedHandler on release.
 */
GT911_Status_t GT911_ReadTouch(GT911_TouchPoint_t *touch)
{
    uint8_t status;
    GT911_Status_t ret = GT911_ReadStatus(&status);
    if(ret != GT911_OK) return ret;
    if((status & 0x80) == 0) return GT911_STATUS_NREADY;
    uint8_t num_coordinates = status & 0x0F;

    if(num_coordinates != 0)
    {
        touch->is_touched = true;
        for(uint8_t i = 0; i < (num_coordinates); i++)
        {
            tx_buffer[0] = (((GT911_TOUCH1_REG - 8) + (8 * i)) & 0xFF00) >> 8; // Next touch has 1 byte offset
            tx_buffer[1] = (((GT911_TOUCH1_REG - 8) + (8 * i)) & 0x00FF);
            I2C_Write(tx_buffer, 2);
            I2C_Read(rx_buffer, 6);
            
            if(touch->was_touched)
            {
                touch->coordinate.x_prev = touch->coordinate.x;
                touch->coordinate.y_prev = touch->coordinate.y;
                touch->has_moved = false;
                touch->gesture = GESTURE_NONE;
            }

            touch->coordinate.x = (rx_buffer[1] << 8) | rx_buffer[0];
            touch->coordinate.y = (rx_buffer[3] << 8) | rx_buffer[2];
        }

        touch->was_touched = true;
    }
    else
    {
        if(touch->was_touched)
        {
            TouchPoint_GetGesture(touch);

            if(touch->touch_released_fptr != NULL)
            {
                touch->touch_released_fptr(touch);
            }
        }
        touch->is_touched = false;
        touch->has_moved = false;
        touch->was_touched = false;
        touch->coordinate.x_prev = touch->coordinate.x;
        touch->coordinate.y_prev = touch->coordinate.y;
        touch->gesture = GESTURE_NONE;
    }

    GT911_ClearStatus();
    return GT911_OK;
}

GT911_Status_t GT911_ReadGesture(GT911_Gesture_t *gesture)
{
    uint8_t status;
    GT911_Status_t ret = GT911_ReadStatus(&status);
    if(ret != GT911_OK) return ret;

    uint8_t num_coordinates = status & 0x0F;
    tx_buffer[0] = (GT911_GESTURE_REG & 0xFF00) >> 8;
    tx_buffer[1] = GT911_GESTURE_REG & 0x00FF;
    I2C_Write(tx_buffer, 2);

    I2C_Read(rx_buffer, 1);

    *gesture = rx_buffer[0];

    GT911_ClearStatus();

    return GT911_OK;
}

GT911_Status_t GT911_SetCommand(uint8_t command){
    tx_buffer[0] = (GT911_COMMAND_CHECK_REG & 0xFF00) >> 8;
	tx_buffer[1] = GT911_COMMAND_CHECK_REG & 0xFF;
	tx_buffer[2] = command;
    I2C_Write(tx_buffer, 3);

	tx_buffer[0] = (GT911_COMMAND_REG & 0xFF00) >> 8;
	tx_buffer[1] = GT911_COMMAND_REG & 0xFF;
	tx_buffer[2] = command;
    I2C_Write(tx_buffer, 3);
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

GT911_Status_t GT911_ClearStatus(void)
{
    tx_buffer[0] = (GT911_STATUS_REG >> 8) & 0xFF;
    tx_buffer[1] = GT911_STATUS_REG & 0xFF;
    tx_buffer[2] = 0x00;
    return I2C_Write(tx_buffer, 3);
}

void TouchPoint_RegisterOnTouchReleasedHandler(GT911_TouchPoint_t *touch, OnTouchReleasedHandler handler)
{
    if(handler != NULL)
    {
        touch->touch_released_fptr = handler;
    }
}

/* Private functions*/
static GT911_Status_t _GT911_SendConfig()
{
    tx_buffer[0] = (GT911_CONFIG_REG & 0xFF00) >> 8;
    tx_buffer[1] = (GT911_CONFIG_REG & 0xFF);
    memcpy(&tx_buffer[2], GT911_Config, sizeof(GT911_Config));
    return I2C_Write(tx_buffer, sizeof(GT911_Config) + 2);
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

static GT911_Status_t GT911_ReadFirmware(uint16_t *firmware)
{
    // Low byte
    tx_buffer[0] = (GT911_FIRMWARE_REG >> 8) & 0xFF;
    tx_buffer[1] = GT911_FIRMWARE_REG & 0xFF;
    I2C_Status_t ret;
    ret = I2C_Write(tx_buffer, 2);
    if(ret != I2C_OK) return GT911_ERR;
    ret = I2C_Read(rx_buffer, 1);
    if(ret != I2C_OK) return GT911_ERR;
    *firmware = rx_buffer[0];

    // High byte
    tx_buffer[0] = ((GT911_FIRMWARE_REG + 1) >> 8) & 0xFF;
    tx_buffer[1] = (GT911_FIRMWARE_REG + 1) & 0xFF;
    ret = I2C_Write(tx_buffer, 2);
    if(ret != I2C_OK) return GT911_ERR;
    ret = I2C_Read(rx_buffer, 1);
    if(ret != I2C_OK) return GT911_ERR;
    *firmware |= (rx_buffer[0] << 8);

    return I2C_OK;
}

static void TouchPoint_GetGesture(GT911_TouchPoint_t *touch)
{
    
    int x_diff = touch->coordinate.x - touch->coordinate.x_prev;
    int y_diff = touch->coordinate.y - touch->coordinate.y_prev;
    if(abs(x_diff) < tap_threshold && abs(y_diff) < tap_threshold && !touch->has_moved) 
    {
        touch->gesture = GESTURE_TAP;
        touch->has_moved = false;
        return;
    }
    if(x_diff > 0 && abs(x_diff) > abs(y_diff)) touch->gesture = GESTURE_RIGHT;
    if(x_diff < 0 && abs(x_diff) > abs(y_diff)) touch->gesture = GESTURE_LEFT;
    if(y_diff > 0 && abs(x_diff) < abs(y_diff)) touch->gesture = GESTURE_DOWN;
    if(y_diff < 0 && abs(x_diff) < abs(y_diff)) touch->gesture = GESTURE_UP;
}