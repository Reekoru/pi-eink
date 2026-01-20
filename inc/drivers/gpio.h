#ifndef GPIO_H
#define GPIO_H
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    GPIO_MODE_INPUT,
    GPIO_MODE_OUTPUT
} GPIO_Mode_t;

typedef enum {
    GPIO_HIGH = 1,
    GPIO_LOW = 0
} GPIO_Level_t;

typedef enum {
    GPIO_OK = 0,
    GPIO_ERR = 1
} GPIO_Status_t;

GPIO_Status_t GPIO_Init();
void GPIO_SetDirIn(uint8_t pin, bool pullup);
void GPIO_SetDirOut(uint8_t pin);
void GPIO_SetLevel(uint8_t pin, GPIO_Level_t highlow);
uint8_t GPIO_GetLevel(uint8_t pin);
#endif // GPIO_H