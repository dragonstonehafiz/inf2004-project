add_executable(lab4_motor lab4_motor.c)

# pull in common dependencies
target_link_libraries(lab4_motor pico_stdlib hardware_pwm)

# enable usb output, enable uart output
pico_enable_stdio_usb(lab4_motor 1)
pico_enable_stdio_uart(lab4_motor 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(lab4_motor)

# add url via pico_set_program_url
example_auto_set_url(lab4_motor)