add_executable(project_motor_turning project_motor_turning.c)

target_include_directories(
    project_motor_turning PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )

# pull in common dependencies
target_link_libraries(project_motor_turning pico_stdlib hardware_pwm)

target_include_directories(project_motor_turning PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/../../common
            )

# enable usb output, enable uart output
pico_enable_stdio_usb(project_motor_turning 1)
pico_enable_stdio_uart(project_motor_turning 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(project_motor_turning)

# add url via pico_set_program_url
example_auto_set_url(project_motor_turning)