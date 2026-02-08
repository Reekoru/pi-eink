#include <stdint.h>
#include <stdbool.h>
#include "utils/err.h"

typedef struct gt911_tp gt911_tp_t;

typedef void (*touch_release_cb_t)(gt911_tp_t*);

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint16_t x_prev;
    uint16_t y_prev;
} gt911_coordinates_t;

typedef enum
{
    NORMAL = 0x00,
    GESTURE = 0x08
} gt911_command_t;

typedef struct
{
    uint16_t x_resolution;
    uint16_t y_resolution;
    uint8_t num_touch_points;
    bool reverse_x;
    bool reverse_y;
    bool switch_xy;
    bool sw_noise_reduction;
    gt911_command_t mode;
} gt911_config_t;

typedef enum
{
    GESTURE_NONE,
    GESTURE_TAP,
    GESTURE_LEFT,
    GESTURE_RIGHT,
    GESTURE_UP,
    GESTURE_DOWN,
    GESTURE_DOUBLE_TAP
}gt911_gesture_t;

struct gt911_tp
{
    gt911_coordinates_t coordinate;
    gt911_gesture_t gesture;
    bool is_touched;
    bool has_moved;
    bool was_touched;
    touch_release_cb_t touch_released_fptr;
    uint8_t touch_id;
    uint8_t release_debounce_count;
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
pi_err_t gt911_reset(void);
pi_err_t gt911_init(gt911_config_t config);
pi_err_t gt911_read_touch(gt911_tp_t *touch);
pi_err_t gt911_read_status(uint8_t *status);
pi_err_t gt911_clear_status();
pi_err_t gt911_set_command(uint8_t command);
pi_err_t gt911_read_gesture(gt911_gesture_t *gesture);


void tp_init(gt911_tp_t *touch);
void tp_set_touch_release_cb(gt911_tp_t *touch, touch_release_cb_t handler);
void GT911_ReadProductID(void);