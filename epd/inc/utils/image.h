#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include "utils/err.h"

pi_err_t img_file_ld(const char* filepath, uint8_t** image_data, size_t* image_size);
pi_err_t dither_image(const uint8_t* input_image, uint8_t* output_image, size_t width, size_t height);
void decode_2bpp(const uint8_t *raw, uint8_t *out);
int find_latest_frame(const char *dir);

#endif // IMAGE_H