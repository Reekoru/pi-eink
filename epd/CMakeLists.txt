# NO project() or cmake_minimum_required needed here usually, 
# as it's a child of the root.

# Define your hardware libraries
add_library(drivers STATIC
    src/drivers/epd.c
    src/drivers/gpio.c
    src/drivers/gt911.c
    src/drivers/i2c.c
    src/drivers/spi.c
)

add_library(utils STATIC
    src/utils/time.c
    src/utils/image.c
    src/utils/timer.c
)

# Set the include path relative to THIS folder
target_include_directories(drivers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)