add_executable(project_wheel_encoder_test wheel_encoder_test.c)

target_include_directories(
    project_wheel_encoder_test PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )

# pull in common dependencies
target_link_libraries(project_wheel_encoder_test pico_stdlib hardware_pwm)

target_include_directories(project_wheel_encoder_test PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/../../common
            )

# enable usb output, enable uart output
pico_enable_stdio_usb(project_wheel_encoder_test 1)
pico_enable_stdio_uart(project_wheel_encoder_test 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(project_wheel_encoder_test)

# add url via pico_set_program_url
example_auto_set_url(project_wheel_encoder_test)