add_executable(
    week10_station3_controller
    week10_station3_controller.c
    )

target_compile_definitions(week10_station3_controller PRIVATE
    WIFI_SSID=\"SINGTEL-WWY2\"
    WIFI_PASSWORD=\"fefkxa7v34\"
    )
    
target_include_directories(week10_station3_controller PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../../common # for our common lwipopts
        )

target_link_libraries(week10_station3_controller 
    pico_stdlib 
    hardware_i2c
    pico_cyw43_arch_lwip_threadsafe_background
    )

pico_add_extra_outputs(week10_station3_controller)
pico_enable_stdio_usb(week10_station3_controller 1)