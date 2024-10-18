add_executable(project-motor-example project-motor-example.c)

target_include_directories(
    project-motor-example PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )

# pull in common dependencies
target_link_libraries(project-motor-example pico_stdlib hardware_pwm)

# enable usb output, enable uart output
pico_enable_stdio_usb(project-motor-example 1)
pico_enable_stdio_uart(project-motor-example 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(project-motor-example)

# add url via pico_set_program_url
example_auto_set_url(project-motor-example)