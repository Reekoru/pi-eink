#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>
#include <bcm2835.h>

// Delay macros
#define delay_ms(ms) bcm2835_delay(ms)
#define delay_us(us) bcm2835_delayMicroseconds(us)

typedef enum{
    PI_OK,
    PI_ERR,
    PI_ERR_DATA,
    PI_ERR_CLKT,
    PI_ERR_NACK,
    PI_ERR_INVALID_PARAM,
    PI_ERR_OUT_OF_MEMORY
}err_t;

void exit_program(int code);

#endif // COMMON_H