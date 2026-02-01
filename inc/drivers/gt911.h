#include <stdint.h>
#include <stdbool.h>

typedef struct GT911_TouchPoint GT911_TouchPoint_t;

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
    uint16_t x_prev;
    uint16_t y_prev;
} GT911_Coordinates_t;

typedef enum
{
    NORMAL = 0x00,
    GESTURE = 0x08
} GT911_Command_t;

typedef struct
{
    uint16_t x_resolution;
    uint16_t y_resolution;
    uint8_t num_touch_points;
    bool reverse_x;
    bool reverse_y;
    bool switch_xy;
    bool sw_noise_reduction;
    GT911_Command_t mode;
} GT911_Config_t;

typedef enum
{
    GESTURE_NONE,
    GESTURE_TAP,
    GESTURE_LEFT,
    GESTURE_RIGHT,
    GESTURE_UP,
    GESTURE_DOWN,
    GESTURE_DOUBLE_TAP
}GT911_Gesture_t;

typedef void (*OnTouchReleasedHandler)(GT911_TouchPoint_t*);

struct GT911_TouchPoint
{
    GT911_Coordinates_t coordinate;
    GT911_Gesture_t gesture;
    bool is_touched;
    bool has_moved;
    bool was_touched;
    OnTouchReleasedHandler touch_released_fptr;
    uint8_t touch_id;
};

#define GT911_GestureToString(g)        \
    ((g) == GESTURE_NONE        ? "GESTURE_NONE" : \
     (g) == GESTURE_TAP         ? "GESTURE_TAP" : \
     (g) == GESTURE_LEFT        ? "GESTURE_LEFT" : \
     (g) == GESTURE_RIGHT       ? "GESTURE_RIGHT" : \
     (g) == GESTURE_UP          ? "GESTURE_UP" : \
     (g) == GESTURE_DOWN        ? "GESTURE_DOWN" : \
     (g) == GESTURE_DOUBLE_TAP  ? "GESTURE_DOUBLE_TAP" : \
                                  "UNKNOWN")
GT911_Status_t GT911_Reset(void);
GT911_Status_t GT911_Init(GT911_Config_t config);
GT911_Status_t GT911_ReadTouch(GT911_TouchPoint_t *touch);
GT911_Status_t GT911_ReadStatus(uint8_t *status);
GT911_Status_t GT911_ClearStatus();
GT911_Status_t GT911_SetCommand(uint8_t command);
GT911_Status_t GT911_ReadGesture(GT911_Gesture_t *gesture);


void TouchPoint_Init(GT911_TouchPoint_t *touch);
void TouchPoint_RegisterOnTouchReleasedHandler(GT911_TouchPoint_t *touch, OnTouchReleasedHandler handler);
void GT911_ReadProductID(void);