add_executable(encoder encoder.c)

# Link to common dependencies such as the standard Pico SDK library, GPIO, and Timer
target_link_libraries(encoder pico_stdlib hardware_timer hardware_gpio)

target_include_directories(encoder PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/../../common
            )

# Enable USB-based stdio for this project (for printf over USB)
pico_enable_stdio_usb(encoder 1)
pico_enable_stdio_uart(encoder 0)

# Create output files (bin/hex/map) for the executable
pico_add_extra_outputs(encoder)