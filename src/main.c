/*******************************************************************
 *
 * main.c - LVGL simulator for GNU/Linux
 *
 * Based on the original file from the repository
 *
 * @note eventually this file won't contain a main function and will
 * become a library supporting all major operating systems
 *
 * To see how each driver is initialized check the
 * 'src/lib/display_backends' directory
 *
 * - Clean up
 * - Support for multiple backends at once
 *   2025 EDGEMTech Ltd.
 *
 * Author: EDGEMTech Ltd, Erik Tagirov (erik.tagirov@edgemtech.ch)
 *
 ******************************************************************/
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"

#include "src/lib/driver_backends.h"
#include "src/lib/simulator_util.h"
#include "src/lib/simulator_settings.h"

#include "drivers/spi.h"
#include "drivers/epd.h"
#include "img.h"
#include "drivers/gt911.h"
#include "drivers/gpio.h"
#include "utils/time.h"
#include "utils/utilities.h"
#include "utils/image.h"
#include <bcm2835.h>

pthread_mutex_t lvgl_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Internal functions */
static void configure_simulator(int argc, char ** argv);
static void print_lvgl_version(void);
static void print_usage(void);
static void save_framebuffer_2bit_timer_cb(lv_timer_t * timer);

/* contains the name of the selected backend if user
 * has specified one on the command line */
static char * selected_backend;

/* Global simulator settings, defined in lv_linux_backend.c */
extern simulator_settings_t settings;

/* EPD related */
static gt911_tp_t touch;

void* touch_poll_thread(void* arg);
static void tp_released_cb(gt911_tp_t *touch);

static void eink_init(void);
static void eink_update_image(uint8_t* image_data);

#define EPD_WIDTH 480
#define EPD_HEIGHT 800

/* Others */
static void lvgl_tp_read(lv_indev_t * indev, lv_indev_data_t * data);

void* lvgl_handler_thread(void* arg) {
    while(1) {
        pthread_mutex_lock(&lvgl_mutex);
        lv_tick_inc(5);
        lv_timer_handler();    // Process touch
        pthread_mutex_unlock(&lvgl_mutex);
        usleep(5000);   
    }
    return NULL;
}

/**
 * @brief Print LVGL version
 */
static void print_lvgl_version(void)
{
    fprintf(stdout, "%d.%d.%d-%s\n",
            LVGL_VERSION_MAJOR,
            LVGL_VERSION_MINOR,
            LVGL_VERSION_PATCH,
            LVGL_VERSION_INFO);
}

/**
 * @brief Print usage information
 */
static void print_usage(void)
{
    fprintf(stdout, "\nlvglsim [-V] [-B] [-f] [-m] [-b backend_name] [-W window_width] [-H window_height]\n\n");
    fprintf(stdout, "-V print LVGL version\n");
    fprintf(stdout, "-B list supported backends\n");
    fprintf(stdout, "-f fullscreen\n");
    fprintf(stdout, "-m maximize\n");
}

/**
 * @brief Configure simulator
 * @description process arguments received by the program to select
 * appropriate options
 * @param argc the count of arguments in argv
 * @param argv The arguments
 */
static void configure_simulator(int argc, char ** argv)
{
    int opt = 0;

    selected_backend = NULL;
    driver_backends_register();

    const char * env_w = getenv("LV_SIM_WINDOW_WIDTH");
    const char * env_h = getenv("LV_SIM_WINDOW_HEIGHT");
    /* Default values */
    settings.window_width = atoi(env_w ? env_w : "800");
    settings.window_height = atoi(env_h ? env_h : "480");

    /* Parse the command-line options. */
    while((opt = getopt(argc, argv, "b:fmW:H:BVh")) != -1) {
        switch(opt) {
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
                break;
            case 'V':
                print_lvgl_version();
                exit(EXIT_SUCCESS);
                break;
            case 'B':
                driver_backends_print_supported();
                exit(EXIT_SUCCESS);
                break;
            case 'b':
                if(driver_backends_is_supported(optarg) == 0) {
                    die("error no such backend: %s\n", optarg);
                }
                selected_backend = strdup(optarg);
                break;
            case 'f':
                settings.fullscreen = true;
                break;
            case 'm':
                settings.maximize = true;
                break;
            case 'W':
                settings.window_width = atoi(optarg);
                break;
            case 'H':
                settings.window_height = atoi(optarg);
                break;
            case ':':
                print_usage();
                die("Option -%c requires an argument.\n", optopt);
                break;
            case '?':
                print_usage();
                die("Unknown option -%c.\n", optopt);
        }
    }
}

/**
 * @brief entry point
 * @description start a demo
 * @param argc the count of arguments in argv
 * @param argv The arguments
 */
int main(int argc, char ** argv)
{

    configure_simulator(argc, argv);

    /* Initialize LVGL. */
    lv_init();

    /* Initialize EINK display */
    eink_init();

    /* Initialize the configured backend */
    if(driver_backends_init_backend(selected_backend) == -1) {
        die("Failed to initialize display backend");
    }

    /* Enable for EVDEV support */
#if LV_USE_EVDEV
    if(driver_backends_init_backend("EVDEV") == -1) {
        die("Failed to initialize evdev");
    }
#endif

    pthread_mutex_lock(&lvgl_mutex);
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, lvgl_tp_read);

    /*Create a Demo*/
    lv_demo_widgets();
    pthread_mutex_unlock(&lvgl_mutex);
    // lv_demo_widgets_start_slideshow();

    lv_timer_create(save_framebuffer_2bit_timer_cb, 10000, NULL);

    /* Enter the run loop of the selected backend */
    driver_backends_run_loop();

    return 0;
}

static void save_framebuffer_2bit_timer_cb(lv_timer_t * timer)
{
    static uint8_t count = 0;
    lv_display_t * disp = lv_display_get_default();
    lv_draw_buf_t * buf = lv_display_get_buf_active(disp);
    if(buf && buf->data) {
        int width = lv_display_get_horizontal_resolution(disp);
        int height = lv_display_get_vertical_resolution(disp);
        int stride = buf->header.stride;
        uint8_t * src = buf->data;
        printf("Framebuffer info: width=%d height=%d stride=%d\n", width, height, stride);

        size_t out_size = width * height;
        uint8_t * out = malloc(out_size);
        if(out) {
            for(int y = 0; y < height; y++) {
                uint8_t * row = src + y * stride;
                for(int x = 0; x < width; x++) {
                    // Read RGB565 pixel (2 bytes per pixel)
                    uint16_t pixel = row[x * 2] | (row[x * 2 + 1] << 8);

                    uint8_t r = (pixel >> 11) & 0x1F; // 5 bits
                    uint8_t g = (pixel >> 5) & 0x3F;  // 6 bits
                    uint8_t b = pixel & 0x1F;         // 5 bits

                    // Scale to 8-bit
                    r = (r << 3) | (r >> 2);
                    g = (g << 2) | (g >> 4);
                    b = (b << 3) | (b >> 2);
                    uint8_t gray = (uint8_t)((r + g + b) / 3);
                    out[y * width + x] = gray;
                }
            }

            eink_update_image(out);
        }
    }
}

/* EPD RELATED */
void* touch_poll_thread(void* arg) {
    (void)arg;
    while (1) {
        gt911_read_touch(&touch);
        usleep(5000); // 1 ms
    }
    return NULL;
}

static void tp_released_cb(gt911_tp_t *touch)
{
    if(touch->gesture == GESTURE_DOWN) exit_program(0);
    puts("Released");
    printf("Coordinate: x=%d, y%d \t gesture=%s\n\r", touch->coordinate.x, touch->coordinate.y, GT911_GestureToString(touch->gesture));
    puts("Changing image");
}

static void eink_init(void)
{
    puts("Starting GT911");
    puts("Starting Eink display app!");

    // This must be called before anything to do with bcm2835
    bcm2835_init();

    if(epd_init_gpio() != PI_OK)
    {
        printf("bcme2835 failed to initialize\n\r");
        epd_close();
        exit_program(1);
    }
    puts("Initializing EPD...");
    epd_init();
    // epd_sleep();
    gt911_config_t gt911_config = {
        .x_resolution=800, 
        .y_resolution=480, 
        .num_touch_points=1, 
        .reverse_y = false, 
        .reverse_x = false, 
        .switch_xy=true, 
        .sw_noise_reduction=true, 
        .mode = NORMAL
    };
    if(gt911_init(gt911_config) != PI_OK)
    {
        puts("Something has gone wrong");
        exit_program(1);
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

    pthread_t lv_thread;
    if (pthread_create(&lv_thread, NULL, lvgl_handler_thread, NULL) != 0) {
        fprintf(stderr, "Failed to create LVGL handler thread\n");
        epd_close();
        exit_program(1);
    }
}

static void eink_update_image(uint8_t* image_data)
{
    uint8_t *dithered = malloc((EPD_WIDTH * EPD_HEIGHT + 7) / 8);
    dither_image(image_data, dithered, EPD_WIDTH, EPD_HEIGHT);
    epd_display_image(dithered);
    printf("E-ink Updated from single frame file.\n");
    free(dithered);
}

static void lvgl_tp_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    (void) indev;
    if(touch.is_touched) {
        printf("Touch at x=%d, y=%d\n", touch.coordinate.x, touch.coordinate.y);
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = touch.coordinate.x;
        data->point.y = touch.coordinate.y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}