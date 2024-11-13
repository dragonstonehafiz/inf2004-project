add_executable(
        TEST_ACCELEROMETER
        mpu6050_i2c.c
        )

# pull in common dependencies and additional i2c hardware support
target_link_libraries(TEST_ACCELEROMETER pico_stdlib hardware_i2c)

target_include_directories(TEST_ACCELEROMETER PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/../../common
            )

# create map/bin/hex file etc.
pico_add_extra_outputs(TEST_ACCELEROMETER)

# add url via pico_set_program_url
example_auto_set_url(TEST_ACCELEROMETER)

pico_enable_stdio_usb(TEST_ACCELEROMETER 1)
pico_enable_stdio_uart(TEST_ACCELEROMETER 0)