add_executable(lab4_ex lab4_ex.c)

# pull in common dependencies
target_link_libraries(lab4_ex pico_stdlib hardware_adc hardware_pwm)

# enable usb output, enable uart output
pico_enable_stdio_usb(lab4_ex 1)
pico_enable_stdio_uart(lab4_ex 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(lab4_ex)

# add url via pico_set_program_url
example_auto_set_url(lab4_ex)