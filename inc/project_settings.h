#define TP_INT_PIN 19
#define TP_RST_PIN 26

#define EPD_WIDTH   800
#define EPD_HEIGHT  480

typedef enum{
    RPI_OK,
    RPI_ERR,
    RPI_DATA,
    RPI_CLKT,
    RPI_NACK
}RPI_ERR_t;