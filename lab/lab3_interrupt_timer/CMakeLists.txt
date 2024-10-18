add_executable(lab3_ex lab3_ex.c)

# pull in common dependencies
target_link_libraries(lab3_ex pico_stdlib)

# enable usb output, enable uart output
pico_enable_stdio_usb(lab3_ex 1)
pico_enable_stdio_uart(lab3_ex 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(lab3_ex)

# add url via pico_set_program_url
example_auto_set_url(lab3_ex)