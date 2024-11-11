add_executable(TEST_MOTOR_FORWARD main.c)

target_include_directories(
    TEST_MOTOR_FORWARD PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )

# pull in common dependencies
target_link_libraries(
    TEST_MOTOR_FORWARD 
    pico_stdlib 
    hardware_pwm
    common_lib
    )

target_include_directories(TEST_MOTOR_FORWARD PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/../../common
            )

# enable usb output, enable uart output
pico_enable_stdio_usb(TEST_MOTOR_FORWARD 1)
pico_enable_stdio_uart(TEST_MOTOR_FORWARD 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(TEST_MOTOR_FORWARD)

# add url via pico_set_program_url
example_auto_set_url(TEST_MOTOR_FORWARD)