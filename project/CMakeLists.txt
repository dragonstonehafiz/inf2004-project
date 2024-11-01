cmake_minimum_required(VERSION 3.12)

add_subdirectory(motor)
add_subdirectory(motor_turning)
add_subdirectory(accelerometer)
add_subdirectory(wheel_encoder_test)
add_subdirectory(ultrasonic)

if (NOT PICO_NO_HARDWARE)

    if (NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
        message("Skipping Pico W FreeRTOS examples as FREERTOS_KERNEL_PATH not defined")
    else()
        include(FreeRTOS_Kernel_import.cmake)
        add_subdirectory(main)
    endif()
endif()