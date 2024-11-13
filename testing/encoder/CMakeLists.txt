add_executable(TEST_ENCODER main.c)

# Link to common dependencies such as the standard Pico SDK library, GPIO, and Timer
target_link_libraries(
    TEST_ENCODER 
    pico_stdlib 
    hardware_timer 
    hardware_gpio
    common_lib
    )

target_include_directories(
    TEST_ENCODER PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )

# Enable USB-based stdio for this project (for printf over USB)
pico_enable_stdio_usb(TEST_ENCODER 1)
pico_enable_stdio_uart(TEST_ENCODER 0)

# Create output files (bin/hex/map) for the executable
pico_add_extra_outputs(TEST_ENCODER)