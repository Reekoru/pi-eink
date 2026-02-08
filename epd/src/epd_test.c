#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "drivers/spi.h"
#include "drivers/epd.h"
#include "img.h"
#include "drivers/gt911.h"
#include "drivers/gpio.h"
#include "utils/time.h"
#include "utils/utilities.h"
#include "utils/image.h"
#include "project_settings.h"
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>

// Global touch structure for thread access
static gt911_tp_t touch;
void *process_frames_thread(void *arg);

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

// Touch polling thread function
void* touch_poll_thread(void* arg) {
    (void)arg;
    while (1) {
        gt911_read_touch(&touch);
        usleep(5000); // 1 ms
    }
    return NULL;
}

static void tp_released_cb(gt911_tp_t *touch);

static uint8_t image_index = 0;

int main()
{
    puts("Starting GT911");
    puts("Starting Eink display app!");

    uint8_t* grayscale_image = NULL;
    size_t image_size = 0;
    pi_err_t result = img_file_ld("src/images/Library.jpg", &grayscale_image, &image_size);
    if (result != PI_OK) {
        fprintf(stderr, "Failed to load image: error %d\n", result);
        return 1;
    }

    const size_t width = 800;
    const size_t height = 480;
    
    // Verify the loaded image size matches expected dimensions
    if (image_size != width * height) {
        fprintf(stderr, "Image size mismatch: expected %zu, got %zu\n", 
                width * height, image_size);
        free(grayscale_image);
        return 1;
    }

    size_t output_size = (width * height + 7) / 8;
    uint8_t* dithered_image = (uint8_t*)malloc(output_size);
    if (!dithered_image) {
        fprintf(stderr, "Failed to allocate output buffer\n");
        free(grayscale_image);
        return 1;
    }

    result = dither_image(grayscale_image, dithered_image, width, height);
    if (result != PI_OK) {
        fprintf(stderr, "Failed to dither image: error %d\n", result);
        free(grayscale_image);
        free(dithered_image);
        return 1;
    }

    // This must be called before anything to do with bcm2835
    bcm2835_init();

    if(epd_init_gpio() != PI_OK)
    {
        printf("bcme2835 failed to initialize\n\r");
        epd_close();
        return 1;
    }
    puts("Initializing EPD...");
    epd_init();
    epd_display_image(dithered_image);
    // epd_sleep();
    gt911_config_t gt911_config = {
        .x_resolution=480, 
        .y_resolution=800, 
        .num_touch_points=1, 
        .reverse_y = true, 
        .reverse_x = true, 
        .switch_xy=false, 
        .sw_noise_reduction=true, 
        .mode = NORMAL
    };
    if(gt911_init(gt911_config) != PI_OK)
    {
        puts("Something has gone wrong");
        return 1;
    }

    delay_ms(1000);
    gt911_coordinates_t coordinates;
    // gt911_tp_t touch; // Now global
    uint8_t num_coordinates;
    uint8_t status;
    gt911_gesture_t gesture;
    puts("Starting main application...");

    tp_init(&touch);
    tp_set_touch_release_cb(&touch, tp_released_cb);

    pthread_t touch_thread;
    if (pthread_create(&touch_thread, NULL, touch_poll_thread, NULL) != 0) {
        fprintf(stderr, "Failed to create touch polling thread\n");
        epd_close();
        exit_program(1);
    }
    pthread_t frame_thread;
    if (pthread_create(&frame_thread, NULL, process_frames_thread, NULL) != 0) {
        fprintf(stderr, "Failed to create frame processing thread\n");
        epd_close();
        exit_program(1);
    }

    while(1)
    {
        usleep(100000);
    }
    puts("Closing application...");
    
    epd_sleep();
    epd_close();
    return 0;
}

static void tp_released_cb(gt911_tp_t *touch)
{
    if(touch->gesture == GESTURE_DOWN) exit_program(0);
    puts("Released");
    printf("Coordinate: x=%d, y%d \t gesture=%s\n\r", touch->coordinate.x, touch->coordinate.y, GT911_GestureToString(touch->gesture));
    puts("Changing image");

    epd_display_image(images[(image_index++ % 2)]);
}

void *process_frames_thread(void *arg) {
    int fd, wd;
    char buffer[BUF_LEN];
    const char* filename = "frame.raw";

    fd = inotify_init();
    if (fd < 0) { perror("inotify_init"); return NULL; }

    wd = inotify_add_watch(fd, FRAME_DIR, IN_CLOSE_WRITE);

    printf("Watching for updates to %s in %s...\n", filename, FRAME_DIR);

    while (1) {
        int length = read(fd, buffer, BUF_LEN);
        if (length < 0) break;

        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];
            if (event->len && (strcmp(event->name, filename) == 0)) {   // Check if the modified file is the raw frame
                char path[512];
                snprintf(path, sizeof(path), "%s/%s", FRAME_DIR, filename);
                
                FILE *f = fopen(path, "rb");
                if (f) {
                    size_t raw_size = EPD_WIDTH * EPD_HEIGHT;
                    uint8_t *raw = malloc(raw_size);
                    // uint8_t *decoded = malloc(EPD_WIDTH * EPD_HEIGHT);
                    uint8_t *dithered = malloc((EPD_WIDTH * EPD_HEIGHT + 7) / 8);

                    if (fread(raw, 1, raw_size, f) == raw_size) {
                        // decode_2bpp(raw, decoded);
                        dither_image(raw, dithered, EPD_WIDTH, EPD_HEIGHT);
                        epd_display_image(dithered);
                        printf("E-ink Updated from single frame file.\n");
                    }

                    fclose(f);
                    free(raw); free(dithered); //free(decoded);
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
    return NULL;
}