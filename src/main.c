#include <stdio.h>
#include "drivers/spi.h"
#include "drivers/epd.h"
#include "img.h"
#include "drivers/gt911.h"
#include "drivers/gpio.h"
#include "utils/time.h"

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
    GT911_Config_t gt911_config = {.x_resolution=480, .y_resolution=800, .num_touch_points=1, .reverse_y = true, .reverse_x = true, .switch_xy=false, .sw_noise_reduction=true};
    GPIO_Init();
    if(GT911_Init(gt911_config) != GT911_OK)
    {
        puts("Something has gone wrong");
        return 1;
    }

    delay_ms(1000);
    GT911_Coordinates_t coordinates;
    uint8_t num_coordinates;
    uint8_t status;
    puts("Starting main application...");
    while(1)
    {
        if(GT911_ReadTouch(&coordinates, &num_coordinates) != GT911_OK)
            continue;
        
            if(num_coordinates > 0)
            {
                for(uint8_t i = 0; i < num_coordinates; i++)
                {
                    printf("Touch %d - X: %d \t Y: %d", i, coordinates.x, coordinates.y);
                }
                printf("\n\r");
            }
    }
    
    puts("Closing application...");
    return 0;
}