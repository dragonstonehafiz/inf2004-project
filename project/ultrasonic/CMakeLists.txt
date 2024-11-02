add_executable(ultrasonic ultrasonic.c)

# pull in common dependencies
target_link_libraries(ultrasonic pico_stdlib hardware_timer hardware_pwm)

target_include_directories(ultrasonic PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/../../common
            )

pico_enable_stdio_usb(ultrasonic 1)
pico_enable_stdio_uart(ultrasonic 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(ultrasonic)

# add url via pico_set_program_url
example_auto_set_url(ultrasonic)
