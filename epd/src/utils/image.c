#include "utils/image.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdint.h>
#include "project_settings.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#include "stb_image.h"

/**
 * @brief Loads an image from file and converts it to grayscale
 * @param filepath Path to image file (PNG, JPEG, BMP supported)
 * @param image_data Pointer to allocated image buffer (caller must free)
 * @param image_size Total size of image in bytes (width * height)
 * @return PI_OK on success, error code otherwise
 */
pi_err_t img_file_ld(const char* filepath, uint8_t** image_data, size_t* image_size)
{
    if (!filepath || !image_data || !image_size) {
        return PI_ERR_INVALID_PARAM;
    }
    
    int width, height, channels;
    
    uint8_t* img = stbi_load(filepath, &width, &height, &channels, 0);
    
    if (!img) {
        fprintf(stderr, "Error loading image: %s\n", stbi_failure_reason());
        return PI_ERR_DATA;
    }
    
    // Calculate output size (grayscale)
    *image_size = width * height;
    
    // Allocate grayscale buffer
    *image_data = (uint8_t*)malloc(*image_size);
    if (!*image_data) {
        stbi_image_free(img);
        return PI_ERR_OUT_OF_MEMORY;
    }
    
    // Convert to grayscale
    for (int i = 0; i < width * height; i++) {
        if (channels == 1) {
            // Already grayscale
            (*image_data)[i] = img[i];
        } else if (channels == 3 || channels == 4) {
            // RGB or RGBA
            uint8_t r = img[i * channels + 0];
            uint8_t g = img[i * channels + 1];
            uint8_t b = img[i * channels + 2];
            (*image_data)[i] = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
        } else {
            // Unsupported channel count
            free(*image_data);
            stbi_image_free(img);
            return PI_ERR_DATA;
        }
    }
    
    stbi_image_free(img);
    
    printf("Loaded image: %dx%d, %d channels, converted to grayscale\n", 
           width, height, channels);
    
    return PI_OK;
}

/**
 * @brief Performs Atkison's dithering algorithm on an image
 * @param input_image Input frame buffer (grayscale)
 * @param output_image Output frame buffer (1-bit packed)
 * @param width Width of input frame buffer
 * @param height Height of input frame buffer
 */
pi_err_t dither_image(const uint8_t* input_image, uint8_t* output_image, size_t width, size_t height)
{
    if (!input_image || !output_image || width == 0 || height == 0) {
        return PI_ERR_INVALID_PARAM;
    }
    
    int* error_buffer = (int*)malloc(width * height * sizeof(int));
    if (!error_buffer) {
        return PI_ERR_OUT_OF_MEMORY;
    }
    
    for (size_t i = 0; i < width * height; i++) {
        error_buffer[i] = (int)input_image[i];
    }
    
    size_t output_size = (width * height + 7) / 8;
    memset(output_image, 0, output_size);
    
    // Atkinson dithering
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            size_t idx = y * width + x;
            int old_pixel = error_buffer[idx];
            
            // Clamp to valid range
            if (old_pixel < 0) old_pixel = 0;
            if (old_pixel > 255) old_pixel = 255;
            
            int new_pixel = (old_pixel >= 128) ? 255 : 0;
            
            // Set bit for black pixels (0 = black)
            if (new_pixel == 0) {
                size_t bit_idx = y * width + x;
                output_image[bit_idx / 8] |= (1 << (7 - (bit_idx % 8)));
            }
            int error = old_pixel - new_pixel;
            
            // Distribute error to neighboring pixels
            // Right pixel (x+1, y)
            if (x + 1 < width) {
                error_buffer[idx + 1] += error / 8;
            }
            
            // Right-right pixel (x+2, y)
            if (x + 2 < width) {
                error_buffer[idx + 2] += error / 8;
            }
            
            // Below-left pixel (x-1, y+1)
            if (x > 0 && y + 1 < height) {
                error_buffer[idx + width - 1] += error / 8;
            }
            
            // Below pixel (x, y+1)
            if (y + 1 < height) {
                error_buffer[idx + width] += error / 8;
            }
            
            // Below-right pixel (x+1, y+1)
            if (x + 1 < width && y + 1 < height) {
                error_buffer[idx + width + 1] += error / 8;
            }
            
            // Below-below pixel (x, y+2)
            if (y + 2 < height) {
                error_buffer[idx + 2 * width] += error / 8;
            }
        }
    }
    
    free(error_buffer);
    return PI_OK;
}


void decode_2bpp(const uint8_t *raw, uint8_t *out) {
    for (int i = 0; i < EPD_WIDTH * EPD_HEIGHT; i++) {
        // int byte_idx = i / 4;
        // int bit_offset = (3 - (i % 4)) * 2;
        // uint8_t pixel = (raw[byte_idx] >> bit_offset) & 0x03;
        // out[i] = pixel * 85; // 0, 85, 170, 255
        out[i] = raw[i];
    }
}


int find_latest_frame(const char *dir) {
    int max_x = -1;
    DIR *d = opendir(dir);
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        int x;
        if (sscanf(entry->d_name, "frame_800x480_2bit_%d.raw", &x) == 1) {
            if (x > max_x) max_x = x;
        }
    }
    closedir(d);
    return max_x;
}