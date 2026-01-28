#include <stdint.h>

typedef enum
{
    GT911_OK = 1,
    GT911_ERR = 0 
} GT911_Status_t;

typedef struct
{
    uint16_t x;
    uint16_t y;
} GT911_Coordinates_t;

GT911_Status_t GT911_Reset(void);
GT911_Status_t GT911_Init(void);
GT911_Status_t GT911_ReadTouch(GT911_Coordinates_t *coordinates);
GT911_Status_t GT911_ReadStatus(uint8_t *status);
void GT911_ReadProductID(void);
