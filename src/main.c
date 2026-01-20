#include <stdio.h>
#include "utils/spi.h"
#include <bcm2835.h>
#include "drivers/epd.h"
#include "img.h"
#include "utils/time.h"

int main()
{
    printf("Starting Eink display app!\n\r");
    // Initialize bcm2835 library
    if (!bcm2835_init())
    {
        printf("bcm2835_init failed. Are you running with sudo?\n");
        return 1;
    }

    if(EPD_Init() != EPD_STATUS_OK)
    {
        printf("EPD failed to initialize\n\r");
        bcm2835_close();
        return 1;
    }

    printf("EPD has been initialized!\n\r");

    EPD_DisplayImage(test_image_2);
    delay_ms(2000);
    EPD_DisplayImage(test_image_1);
    EPD_Sleep();
    
    // Clean up
    bcm2835_close();
    
    printf("Hello There!\n\r");
    return 0;
}