add_executable(TEST_ULTRASONIC main_ultrasonic.c)

# pull in common dependencies
target_link_libraries(
    TEST_ULTRASONIC
    pico_stdlib 
    hardware_timer
    hardware_pwm
    common_lib
    )

target_include_directories(
    TEST_ULTRASONIC PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../common
    )

pico_enable_stdio_usb(TEST_ULTRASONIC 1)
pico_enable_stdio_uart(TEST_ULTRASONIC 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(TEST_ULTRASONIC)

# add url via pico_set_program_url
example_auto_set_url(TEST_ULTRASONIC)
