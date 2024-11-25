# Define target
set(target common_lib)

add_library(common_lib
        accelerometer.h
        accelerometer.c
        encoder.h
        encoder.c
        motor.h
        motor.c
        ultrasonic.c
        ultrasonic.h
        wheels.h
        wheels.c
        pins.h
        network.h
        network.c
        ir_sensor.c 
        code39_decoder.c
        utils.c
        )

# pull in common dependencies
target_link_libraries(
    common_lib
    pico_stdlib 
    hardware_pwm
    hardware_i2c
    hardware_adc 
    hardware_gpio
    hardware_timer
    pico_cyw43_arch_lwip_threadsafe_background
    )

target_include_directories(common_lib PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# create map/bin/hex file etc.
pico_enable_stdio_usb(common_lib 1)
