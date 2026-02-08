#ifndef GPIO_H
#define GPIO_H
#include <stdbool.h>
#include <stdint.h>
#include "utils/err.h"

typedef enum {
    GPIO_MODE_INPUT,
    GPIO_MODE_OUTPUT
} gpio_mode_t;

pi_err_t gpio_init();
void gpio_set_dir_in(uint8_t pin, bool pullup);
void gpio_set_dir_out(uint8_t pin);
void gpio_set_level(uint8_t pin, uint8_t highlow);
uint8_t gpio_get_level(uint8_t pin);
#endif // GPIO_H