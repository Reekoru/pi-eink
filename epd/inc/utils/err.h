#ifndef ERR_H
#define ERR_H

#include <stdlib.h>

// Forward declarations for exit_program macro
void epd_sleep(void);
void epd_close(void);

typedef enum{
    PI_OK,
    PI_ERR,
    PI_ERR_NREADY,
    PI_ERR_DATA,
    PI_ERR_CLKT,
    PI_ERR_NACK,
    PI_ERR_INVALID_PARAM,
    PI_ERR_OUT_OF_MEMORY
}pi_err_t;

#define PI_ERR_TO_STRING(err) \
    ((err) == PI_OK                 ? "PI_OK" : \
     (err) == PI_ERR                ? "PI_ERR" : \
     (err) == PI_ERR_NREADY         ? "PI_ERR_NREADY" : \
     (err) == PI_ERR_DATA           ? "PI_ERR_DATA" : \
     (err) == PI_ERR_CLKT           ? "PI_ERR_CLKT" : \
     (err) == PI_ERR_NACK           ? "PI_ERR_NACK" : \
     (err) == PI_ERR_INVALID_PARAM  ? "PI_ERR_INVALID_PARAM" : \
     (err) == PI_ERR_OUT_OF_MEMORY  ? "PI_ERR_OUT_OF_MEMORY" : \
                                      "UNKNOWN_ERROR")

#define exit_program(code) do { \
    puts("Exiting program..."); \
    epd_sleep(); \
    epd_close(); \
    exit(code); \
} while(0)

#endif // ERR_H