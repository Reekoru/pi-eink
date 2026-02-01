/**
 * Example usage of image loading and dithering functions
 */

#include "utils/image.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    // Step 1: Load image from file
    uint8_t* grayscale_image = NULL;
    size_t image_size = 0;
    
    err_t result = LoadImageFromFile("input.png", &grayscale_image, &image_size);
    if (result != PI_OK) {
        fprintf(stderr, "Failed to load image: error %d\n", result);
        return 1;
    }
    
    // Calculate dimensions (you need to know the image dimensions)
    // For an 800x480 image:
    const size_t width = 800;
    const size_t height = 480;
    
    // Verify the loaded image size matches expected dimensions
    if (image_size != width * height) {
        fprintf(stderr, "Image size mismatch: expected %zu, got %zu\n", 
                width * height, image_size);
        free(grayscale_image);
        return 1;
    }
    
    // Step 2: Allocate output buffer for dithered image
    // For 800x480, this will be 48000 bytes (800*480/8)
    size_t output_size = (width * height + 7) / 8;
    uint8_t* dithered_image = (uint8_t*)malloc(output_size);
    if (!dithered_image) {
        fprintf(stderr, "Failed to allocate output buffer\n");
        free(grayscale_image);
        return 1;
    }
    
    // Step 3: Apply Atkinson dithering
    result = dither_image(grayscale_image, dithered_image, width, height);
    if (result != PI_OK) {
        fprintf(stderr, "Failed to dither image: error %d\n", result);
        free(grayscale_image);
        free(dithered_image);
        return 1;
    }

    result = dither_image(grayscale_image, dithered_image, width, height);
    if (result != PI_OK) {
        fprintf(stderr, "Failed to dither image: error %d\n", result);
        free(grayscale_image);
        free(dithered_image);
        return 1;
    }
    
    printf("Successfully dithered image: %zux%zu -> %zu bytes\n", 
           width, height, output_size);
    
    // Step 4: Use the dithered image
    // For example, send to e-ink display:
    // send_to_display(dithered_image, output_size);
    
    // Or save to a file:
    FILE* fp = fopen("output_dithered.bin", "wb");
    if (fp) {
        fwrite(dithered_image, 1, output_size, fp);
        fclose(fp);
        printf("Saved dithered image to output_dithered.bin\n");
    }
    
    // Step 5: Clean up
    free(grayscale_image);
    free(dithered_image);
    
    return 0;
}

/**
 * COMPLETE WORKFLOW:
 * 
 * 1. LoadImageFromFile():
 *    - Input: Path to PNG/JPEG/BMP file
 *    - Output: Grayscale image buffer (8-bit per pixel)
 *    - Returns: Size of buffer (width * height)
 * 
 * 2. dither_image():
 *    - Input: Grayscale buffer, dimensions
 *    - Output: 1-bit packed image (MSB first)
 *    - For 800x480: outputs 48000 bytes exactly
 * 
 * MEMORY MANAGEMENT:
 * - LoadImageFromFile() allocates memory - YOU must free it
 * - dither_image() requires pre-allocated output buffer
 * - Always free both buffers when done
 * 
 * BIT PACKING FORMAT:
 * - Each byte contains 8 pixels
 * - MSB (bit 7) = first pixel, LSB (bit 0) = 8th pixel
 * - 1 = white/on, 0 = black/off
 * - Example: 0b10101010 = white-black-white-black pattern
 */
