add_executable(lab6_ex
        lab6_ex.c
        )

# pull in common dependencies and additional clocks hardware support
target_link_libraries(lab6_ex pico_stdlib)


# enable usb output, enable uart output
pico_enable_stdio_usb(lab6_ex 1)
pico_enable_stdio_uart(lab6_ex 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(lab6_ex)

# add url via pico_set_program_url
example_auto_set_url(lab6_ex)
