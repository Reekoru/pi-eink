#include <stdio.h>
#include "utils/spi.h"
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
    GPIO_Init();
    if(GT911_Init() != GT911_OK)
    {
        puts("Something has gone wrong");
        return 1;
    }
    
    puts("Closing application...");
    return 0;
}