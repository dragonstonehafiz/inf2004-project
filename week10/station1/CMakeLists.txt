add_executable(week10_station1 station1.c)

target_include_directories(
    week10_station1 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )

# pull in common dependencies
target_link_libraries(week10_station1 pico_stdlib hardware_pwm)

target_include_directories(week10_station1 PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/../../common
            )

# enable usb output, enable uart output
pico_enable_stdio_usb(week10_station1 1)
pico_enable_stdio_uart(week10_station1 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(week10_station1)

# add url via pico_set_program_url
example_auto_set_url(week10_station1)