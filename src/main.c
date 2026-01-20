#include <stdio.h>
#include "utils/spi.h"
#include "drivers/epd.h"
#include "img.h"
#include "utils/time.h"

int main()
{
    printf("Starting Eink display app!\n\r");

    if(EPD_Init() != EPD_STATUS_OK)
    {
        printf("EPD failed to initialize\n\r");
        EPD_Close();
        return 1;
    }

    printf("EPD has been initialized!\n\r");

    EPD_DisplayImage(test_image_2);
    delay_ms(2000);
    EPD_DisplayImage(test_image_1);
    EPD_Sleep();
    
    EPD_Close();
    
    puts("Closing application...");
    return 0;
}