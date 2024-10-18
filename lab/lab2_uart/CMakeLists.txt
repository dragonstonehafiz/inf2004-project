add_executable(lab2_ex lab2_ex.c)

# pull in common dependencies
target_link_libraries(lab2_ex pico_stdlib)

# enable usb output, enable uart output
pico_enable_stdio_usb(lab2_ex 1)
pico_enable_stdio_uart(lab2_ex 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(lab2_ex)

# add url via pico_set_program_url
example_auto_set_url(lab2_ex)