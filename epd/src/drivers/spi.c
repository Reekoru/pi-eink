#include "drivers/spi.h"
#include <bcm2835.h>

void spi_init(spi_config_t config)
{
    // Initialize 
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(config.bit_order);     //High first transmission
    bcm2835_spi_setDataMode(config.mode);                  //spi mode 0
    bcm2835_spi_setClockDivider(config.clock_divider);  //Frequency
    bcm2835_spi_chipSelect(config.chip_select);                     //set CE0
    bcm2835_spi_setChipSelectPolarity(config.chip_select, config.chip_select_polarity);     //enable cs0
}
void spi_write(uint8_t data)
{
    bcm2835_spi_transfer(data);
}