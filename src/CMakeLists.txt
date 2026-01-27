add_library(drivers
    drivers/epd.c
    drivers/gpio.c
    drivers/gt911.c
    drivers/i2c.c
)

add_library(utils
    utils/time.c
    utils/spi.c
)

add_executable(app
    main.c
)

target_include_directories(drivers PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
)

target_include_directories(utils PUBLIC
    ${PROJECT_SOURCE_DIR}/inc
)

target_include_directories(app PRIVATE
    ${PROJECT_SOURCE_DIR}/inc
)

target_link_libraries(app
    drivers
    utils
    bcm2835
)
