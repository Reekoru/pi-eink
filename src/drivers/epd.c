#include "drivers/epd.h"
#include "drivers/gpio.h"
#include "utils/time.h"
#include "drivers/spi.h"
#include "project_settings.h"
#include <stddef.h>
#include <stdio.h>
#include <bcm2835.h>

#define EPD_BUSY_PIN 24
#define EPD_DC_PIN 25
#define EPD_CS_PIN 8
#define EPD_RST_PIN 17

static void EPD_WriteCmd(uint8_t cmd);
static void EPD_WriteData(uint8_t data);

static uint16_t edp_width;
static uint16_t edp_height;

static SPI_Config_t config =
    {
        .bit_order = SPI_BIT_ORDER_MSBFIRST,
        .mode = SPI_MODE_0,
        .clock_divider = SPI_CLOCK_DIVIDER_128,
        .chip_select = SPI_CS_0,
        .chip_select_polarity = 0 // LOW polarity
};
EPD_Status_t EPD_Init_bcme2835(void)
{
    // Initialize bcm2835 library
    if (!bcm2835_init())
    {
        printf("bcm2835_init failed. Are you running with sudo?\n");
        return EPD_STATUS_ERR;
    }
    // Initialize GPIOs
    GPIO_SetDirOut(EPD_RST_PIN);
    GPIO_SetDirOut(EPD_DC_PIN);
    GPIO_SetDirIn(EPD_BUSY_PIN, false);

    SPI_Init(config);

    return EPD_STATUS_OK;
}
EPD_Status_t EPD_Init(void)
{
    EPD_Reset();

    EPD_WriteCmd(PSR_CMD);
    EPD_WriteData(0x1F); //

    EPD_WriteCmd(CDI_CMD);
    EPD_WriteData(0x10);
    EPD_WriteData(0x07);

    EPD_WriteCmd(PON_CMD); // Turn on EDP
    delay_ms(100);
    EPD_WaitUntilIdle();

    EPD_WriteCmd(BTST_CMD);
    EPD_WriteData(0x27);
    EPD_WriteData(0x27);
    EPD_WriteData(0x18);
    EPD_WriteData(0x17);

    EPD_WriteCmd(CCSET_CMD);
    EPD_WriteData(0x02);

    EPD_WriteCmd(TSSET_CMD);
    EPD_WriteData(0x4E);

    return EPD_STATUS_OK;
}

EPD_Status_t EPD_Init_4g(void)
{
    EPD_Reset();

    EPD_WriteCmd(PSR_CMD);
    EPD_WriteData(0x1F); //

    EPD_WriteCmd(CDI_CMD);
    EPD_WriteData(0x10);
    EPD_WriteData(0x07);

    EPD_WriteCmd(PON_CMD); // Turn on EDP
    delay_ms(100);
    EPD_WaitUntilIdle();

    EPD_WriteCmd(BTST_CMD);
    EPD_WriteData(0x27);
    EPD_WriteData(0x27);
    EPD_WriteData(0x18);
    EPD_WriteData(0x17);

    EPD_WriteCmd(CCSET_CMD);
    EPD_WriteData(0x02);

    EPD_WriteCmd(TSSET_CMD);
    EPD_WriteData(0x5F);

    return EPD_STATUS_OK;
}

EPD_Status_t EPD_DisplayInit(void)
{
    EPD_Reset();

    EPD_WriteCmd(PSR_CMD);
    EPD_WriteData(0x1F);

    EPD_WriteCmd(PON_CMD);
    delay_ms(100);
    EPD_WaitUntilIdle();

    EPD_WriteCmd(CCSET_CMD);
    EPD_WriteData(0x02);

    EPD_WriteCmd(TSSET_CMD);
    EPD_WriteData(0x4E); // 0x4E looks like to work with for images that get more complex

    return EPD_STATUS_OK;
}

void EPD_Reset(void)
{
    GPIO_SetLevel(EPD_RST_PIN, LOW);
    delay_ms(200);
    GPIO_SetLevel(EPD_RST_PIN, HIGH);
    delay_ms(200);
}

void EPD_Update(void)
{
    EPD_WriteCmd(DRF_CMD);
    delay_ms(1);
    EPD_WaitUntilIdle();
}

EPD_Status_t EPD_Clear(void)
{
    return EPD_STATUS_OK;
}

EPD_Status_t EPD_DisplayImage(const uint8_t *image)
{
    if (image != NULL)
    {
        EPD_WriteCmd(DTM1_CMD);
        for (int i = 0; i < EPD_WIDTH * EPD_HEIGHT / 8; i++)
        {
            EPD_WriteData(0x00); // bit set: white, bit reset: black
        }
        delay_ms(2);
        EPD_WriteCmd(DTM2_CMD);
        for (int i = 0; i < EPD_WIDTH * EPD_HEIGHT / 8; i++)
        {
            EPD_WriteData(image[i]);
        }
        delay_ms(2);
    }
    EPD_Update();
    return EPD_STATUS_OK;
}

EPD_Status_t EPD_DisplayImage_4g(const uint8_t *image)
{
    uint8_t temp1, temp2, temp3;

    EPD_WriteCmd(DTM1_CMD);

    for (uint32_t i = 0; i < EPD_WIDTH * EPD_HEIGHT / 8; i++)
    {
        temp3 = 0;
        for (uint8_t j = 0; j < 2; j++)
        {
            temp1 = image[i * 2 + j];
            for (uint8_t k = 0; k < 4; k++)
            {
                temp2 = temp1 & 0xC0;
                if (temp2 == 0xC0)
                    temp3 |= 0x01; // white
                else if (temp2 == 0x00)
                    temp3 |= 0x00; // black
                else if ((temp2 >= 0x80) && (temp2 < 0xC0))
                    temp3 |= 0x00; // gray1
                else if (temp2 == 0x40)
                    temp3 |= 0x01; // gray2

                if ((j == 0 && k <= 3) || (j == 1 && k <= 2))
                {
                    temp3 <<= 1;
                    temp1 <<= 2;
                }
            }
        }
        EPD_WriteData(~temp3);
    }

    EPD_WriteCmd(DTM2_CMD);
    for (uint32_t i = 0; i < EPD_WIDTH * EPD_HEIGHT / 8; i++) 
    {
        temp3 = 0;
        for (uint8_t j = 0; j < 2; j++)
        {
            temp1 = image[i * 2 + j];
            for (uint8_t k = 0; k < 4; k++)
            {
                temp2 = temp1 & 0xC0;
                if (temp2 == 0xC0)
                    temp3 |= 0x01; // white
                else if (temp2 == 0x00)
                    temp3 |= 0x00; // black
                else if ((temp2 >= 0x80) && (temp2 < 0xC0))
                    temp3 |= 0x01; // gray1
                else if (temp2 == 0x40)
                    temp3 |= 0x00; // gray2

                if ((j == 0 && k <= 3) || (j == 1 && k <= 2))
                {
                    temp3 <<= 1;
                    temp1 <<= 2;
                }
            }
        }
        EPD_WriteData(~temp3);
    }

    EPD_Update();

    return EPD_STATUS_OK;
}

void EPD_WaitUntilIdle(void)
{
    EPD_WriteCmd(FLG_CMD);

    // Poll while busy
    while (GPIO_GetLevel(EPD_BUSY_PIN) == 0)
    {
        delay_ms(100);
    }
}

void EPD_Sleep(void)
{
    EPD_WriteCmd(POF_CMD);
    EPD_WaitUntilIdle();

    EPD_WriteCmd(DSLP_CMD);
    EPD_WriteData(0xA5);
}

void EPD_Close(void)
{
    bcm2835_close();
}

void EPD_EnableTemperatureSensor()
{
    // To implement
}

void EPD_ReadTemperatureSensor()
{
    // To implement
}

/* Private functions */
static void EPD_WriteCmd(uint8_t cmd)
{
    GPIO_SetLevel(EPD_DC_PIN, LOW); // DC LOW = Command
    SPI_Write(cmd);
}

static void EPD_WriteData(uint8_t data)
{
    GPIO_SetLevel(EPD_DC_PIN, HIGH); // DC HIGH = Data
    SPI_Write(data);
}