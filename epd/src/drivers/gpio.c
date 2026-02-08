#include "drivers/gpio.h"
#include <bcm2835.h>

pi_err_t gpio_init()
{
    return bcm2835_init();
}

void gpio_set_dir_in(uint8_t pin, bool pullup)
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

void gpio_set_dir_out(uint8_t pin)
{
    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
}

void gpio_set_level(uint8_t pin, uint8_t highlow)
{
    bcm2835_gpio_write(pin, highlow);
}

uint8_t gpio_get_level(uint8_t pin)
{
    return bcm2835_gpio_lev(pin);
}
