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

static void epd_write_cmd(uint8_t cmd);
static void epd_write_data(uint8_t data);

static uint16_t edp_width;
static uint16_t edp_height;

static bool is_deep_sleep = false;

static spi_config_t config =
    {
        .bit_order = SPI_BIT_ORDER_MSBFIRST,
        .mode = SPI_MODE_0,
        .clock_divider = SPI_CLOCK_DIVIDER_128,
        .chip_select = SPI_CS_0,
        .chip_select_polarity = 0 // LOW polarity
};
pi_err_t epd_init_gpio(void)
{
    // Initialize GPIOs
    gpio_set_dir_out(EPD_RST_PIN);
    gpio_set_dir_out(EPD_DC_PIN);
    gpio_set_dir_in(EPD_BUSY_PIN, false);

    spi_init(config);

    return RPI_OK;
}
pi_err_t epd_init(void)
{
    epd_reset();

    epd_write_cmd(PSR_CMD);
    epd_write_data(0x1F); //

    epd_write_cmd(CDI_CMD);
    epd_write_data(0x10);
    epd_write_data(0x07);

    epd_write_cmd(PON_CMD); // Turn on EDP
    delay_ms(100);
    epd_wait_idle();

    epd_write_cmd(BTST_CMD);
    epd_write_data(0x27);
    epd_write_data(0x27);
    epd_write_data(0x18);
    epd_write_data(0x17);

    epd_write_cmd(CCSET_CMD);
    epd_write_data(0x02);

    epd_write_cmd(TSSET_CMD);
    epd_write_data(0x4E);

    return RPI_OK;
}

pi_err_t epd_init_4g(void)
{
    epd_reset();

    epd_write_cmd(PSR_CMD);
    epd_write_data(0x1F); //

    epd_write_cmd(CDI_CMD);
    epd_write_data(0x10);
    epd_write_data(0x07);

    epd_write_cmd(PON_CMD); // Turn on EDP
    delay_ms(100);
    epd_wait_idle();

    epd_write_cmd(BTST_CMD);
    epd_write_data(0x27);
    epd_write_data(0x27);
    epd_write_data(0x18);
    epd_write_data(0x17);

    epd_write_cmd(CCSET_CMD);
    epd_write_data(0x02);

    epd_write_cmd(TSSET_CMD);
    epd_write_data(0x5F);

    return RPI_OK;
}

pi_err_t epd_display_init(void)
{
    epd_reset();

    epd_write_cmd(PSR_CMD);
    epd_write_data(0x1F);

    epd_write_cmd(PON_CMD);
    delay_ms(100);
    epd_wait_idle();

    epd_write_cmd(CCSET_CMD);
    epd_write_data(0x02);

    epd_write_cmd(TSSET_CMD);
    epd_write_data(0x4E); // 0x4E looks like to work with for images that get more complex

    return RPI_OK;
}

void epd_reset(void)
{
    gpio_set_level(EPD_RST_PIN, LOW);
    delay_ms(200);
    gpio_set_level(EPD_RST_PIN, HIGH);
    delay_ms(200);

    is_deep_sleep = false;
}

void epd_update(void)
{
    epd_write_cmd(DRF_CMD);
    delay_ms(1);
    epd_wait_idle();
}

pi_err_t epd_clear(void)
{
    return RPI_OK;
}

pi_err_t epd_display_image(const uint8_t *image)
{
    if (image != NULL)
    {
        epd_write_cmd(DTM1_CMD);
        for (int i = 0; i < EPD_WIDTH * EPD_HEIGHT / 8; i++)
        {
            epd_write_data(0x00); // bit set: white, bit reset: black
        }
        delay_ms(2);
        epd_write_cmd(DTM2_CMD);
        for (int i = 0; i < EPD_WIDTH * EPD_HEIGHT / 8; i++)
        {
            epd_write_data(image[i]);
        }
        delay_ms(2);
    }
    epd_update();
    return RPI_OK;
}

pi_err_t epd_display_image_4g(const uint8_t *image)
{
    uint8_t temp1, temp2, temp3;

    epd_write_cmd(DTM1_CMD);

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
        epd_write_data(~temp3);
    }

    epd_write_cmd(DTM2_CMD);
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
        epd_write_data(~temp3);
    }

    epd_update();

    return RPI_OK;
}

void epd_wait_idle(void)
{
    epd_write_cmd(FLG_CMD);

    // Poll while busy
    while (gpio_get_level(EPD_BUSY_PIN) == 0)
    {
        delay_ms(100);
    }
}

void epd_sleep(void)
{
    puts("EPD to sleep...");
    epd_write_cmd(POF_CMD);
    epd_wait_idle();

    epd_write_cmd(DSLP_CMD);
    epd_write_data(0xA5);

    is_deep_sleep = true;
}

void epd_close(void)
{
    puts("EPD to close...");
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

bool epd_is_deep_sleep(void)
{
    return is_deep_sleep;
}

/* Private functions */
static void epd_write_cmd(uint8_t cmd)
{
    gpio_set_level(EPD_DC_PIN, LOW); // DC LOW = Command
    spi_write(cmd);
}

static void epd_write_data(uint8_t data)
{
    gpio_set_level(EPD_DC_PIN, HIGH); // DC HIGH = Data
    spi_write(data);
}