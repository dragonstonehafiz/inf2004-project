add_executable(
    main_controller
    main_controller.c
    )

target_compile_definitions(main_controller PRIVATE
    WIFI_SSID=\"bighowdy\"
    WIFI_PASSWORD=\"yeedyourlasthaw\"
    )
    
target_include_directories(main_controller PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../../common # for our common lwipopts
        )

target_link_libraries(main_controller 
    pico_stdlib
    hardware_i2c
    pico_cyw43_arch_lwip_threadsafe_background
    common_lib
    )

pico_add_extra_outputs(main_controller)
pico_enable_stdio_usb(main_controller 1)