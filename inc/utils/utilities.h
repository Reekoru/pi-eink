#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>
#include <bcm2835.h>

// Delay macros
#define delay_ms(ms) bcm2835_delay(ms)
#define delay_us(us) bcm2835_delayMicroseconds(us)

typedef enum{
    STATUS_OK,
    STATUS_ERR,
    STATUS_DATA,
    STATUS_CLKT,
    STATUS_NACK
}err_t;

#endif // COMMON_H