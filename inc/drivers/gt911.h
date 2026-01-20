#include <stdint.h>

typedef enum
{
    GT911_OK = 1,
    GT911_ERR = 0 
} GT911_Status_t;

GT911_Status_t GT911_Reset(void);
GT911_Status_t GT911_Init(void);