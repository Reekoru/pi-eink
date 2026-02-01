#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include "utils/utilities.h"

err_t LoadImageFromFile(const char* filepath, uint8_t** image_data, size_t* image_size);
err_t dither_image(const uint8_t* input_image, uint8_t* output_image, size_t width, size_t height);

#endif // IMAGE_H