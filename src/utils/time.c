#include "utils/time.h"
#include <bcm2835.h>

void delay_ms(uint32_t ms)
{
    bcm2835_delay(ms);
}

void delay_us(uint32_t us)
{
    bcm2835_delayMicroseconds(us);
}