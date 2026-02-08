#ifndef TIME_H
#define TIME_H
#include <stdint.h>

#define delay_ms(ms) bcm2835_delay(ms)
#define delay_us(us) bcm2835_delayMicroseconds(us)

#endif // TIME_H