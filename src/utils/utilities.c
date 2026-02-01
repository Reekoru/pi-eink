#include "utils/utilities.h"
#include "drivers/epd.h"
#include "drivers/gt911.h"
#include <stdlib.h>
#include <stdio.h>

void exit_program(int code)
{
    puts("Exiting program...");
    EPD_Sleep();
    EPD_Close();
    exit(code);
}