#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    GT911_OK = 1,
    GT911_ERR = 0,
    GT911_STATUS_NREADY
} GT911_Status_t;

typedef struct
{
    uint16_t x;
    uint16_t y;
} GT911_Coordinates_t;

typedef struct
{
    uint16_t x_resolution;
    uint16_t y_resolution;
    uint8_t num_touch_points;
    bool reverse_x;
    bool reverse_y;
    bool switch_xy;
    bool sw_noise_reduction;
} GT911_Config_t;

typedef enum
{
    NORMAL = 0x00,
    GESTURE = 0x08
} GT911_Command_t;

GT911_Status_t GT911_Reset(void);
GT911_Status_t GT911_Init(GT911_Config_t config);
GT911_Status_t GT911_ReadTouch(GT911_Coordinates_t *coordinates, uint8_t *num_coordinates);
GT911_Status_t GT911_ReadStatus(uint8_t *status);
GT911_Status_t GT911_ClearStatus();
GT911_Status_t GT911_SetCommand(uint8_t command);
void GT911_ReadProductID(void);