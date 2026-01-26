#include "drivers/gpio.h"
#include <bcm2835.h>

GPIO_Status_t GPIO_Init()
{
    return bcm2835_init();
}

void GPIO_SetDirIn(uint8_t pin, bool pullup)
{
    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
    if (pullup) 
    {
        bcm2835_gpio_set_pud(pin, BCM2835_GPIO_PUD_UP);
    } 
    else 
    {
        bcm2835_gpio_set_pud(pin, BCM2835_GPIO_PUD_DOWN);
    }
}

void GPIO_SetDirOut(uint8_t pin)
{
    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
}

void GPIO_SetLevel(uint8_t pin, uint8_t highlow)
{
    bcm2835_gpio_write(pin, highlow);
}

uint8_t GPIO_GetLevel(uint8_t pin)
{
    return bcm2835_gpio_lev(pin);
}