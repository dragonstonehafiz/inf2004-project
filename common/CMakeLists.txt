# Define target
set(target common_lib)

add_library(common_lib
        encoder.h
        encoder.c
        motor.h
        motor.c
        wheels.h
        wheels.c
        )

# pull in common dependencies
target_link_libraries(
    common_lib
    pico_stdlib 
    hardware_pwm
    )

target_include_directories(common_lib PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# create map/bin/hex file etc.
pico_enable_stdio_usb(common_lib 1)
