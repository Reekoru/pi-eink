#include <stdio.h>
#include "drivers/spi.h"
#include "drivers/epd.h"
#include "img.h"
#include "drivers/gt911.h"
#include "drivers/gpio.h"
#include "utils/time.h"

static void OnTouchReleased(GT911_TouchPoint_t *touch);

int main()
{
    puts("Starting GT911");
    // puts("Starting Eink display app!");
    // if(EPD_Init_bcme2835() != EPD_STATUS_OK)
    // {
    //     printf("bcme2835 failed to initialize\n\r");
    //     EPD_Close();
    //     return 1;
    // }
    // puts("Initializing EPD...");
    // // EPD_Init();
    // // EPD_DisplayImage(gImage_library);
    // // EPD_Sleep();
    // // delay_ms(2000);
    
    // EPD_Init_4g();
    // EPD_DisplayImage_4g(gImage_4Glibrary);
    // EPD_Sleep();
    // delay_ms(2000);

    // // EPD_Init();
    // // EPD_DisplayImage(gImage_panel);
    // // EPD_Sleep();
    // // delay_ms(2000);
    
    // EPD_Init_4g();
    // EPD_DisplayImage_4g(gImage_4Gpanel);
    // EPD_Sleep();
    // delay_ms(2000);
    
    // EPD_Close();
    GT911_Config_t gt911_config = {
        .x_resolution=480, 
        .y_resolution=800, 
        .num_touch_points=1, 
        .reverse_y = true, 
        .reverse_x = true, 
        .switch_xy=false, 
        .sw_noise_reduction=true, 
        .mode = NORMAL
    };
    GPIO_Init();
    if(GT911_Init(gt911_config) != GT911_OK)
    {
        puts("Something has gone wrong");
        return 1;
    }

    delay_ms(1000);
    GT911_Coordinates_t coordinates;
    GT911_TouchPoint_t touch;
    uint8_t num_coordinates;
    uint8_t status;
    GT911_Gesture_t gesture;
    puts("Starting main application...");

    TouchPoint_Init(&touch);
    TouchPoint_RegisterOnTouchReleasedHandler(&touch, OnTouchReleased);
    while(1)
    {
        if(GT911_ReadTouch(&touch) != GT911_OK)
            continue;
        
            // if(touch.is_touched)
            // {
            //     printf("Coordinates- x: %d, y: %d \t Gesture: %s\n\r", touch.coordinate.x, touch.coordinate.y, GT911_GestureToString(touch.gesture));
            // }
    }
    puts("Closing application...");
    return 0;
}

static void OnTouchReleased(GT911_TouchPoint_t *touch)
{
    puts("Released");
    printf("Coordinate: x=%d, y%d \t gesture=%s\n\r", touch->coordinate.x, touch->coordinate.y, GT911_GestureToString(touch->gesture));
}