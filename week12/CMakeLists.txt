# Add executable and source files
add_executable(
    week12_labtest
    main.c
    functions.c
)

# Link with the Pico standard library and hardware PWM
target_link_libraries(
    week12_labtest 
    pico_stdlib 
    hardware_pwm)

# Enable USB serial output
pico_enable_stdio_usb(week12_labtest 1)
# Enable UART for serial monitor output (optional)
pico_enable_stdio_uart(week12_labtest 1)

# Create the output files (e.g., .uf2 file for flashing)
pico_add_extra_outputs(week12_labtest)
