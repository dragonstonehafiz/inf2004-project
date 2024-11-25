add_executable(
    main_dashboard
    udp_dashboard.c
    )

target_compile_definitions(main_dashboard PRIVATE)
    
target_include_directories(main_dashboard PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../../common # for our common lwipopts
        )

target_link_libraries(main_dashboard 
    pico_stdlib
    hardware_i2c
    pico_cyw43_arch_lwip_threadsafe_background
    )

pico_add_extra_outputs(main_dashboard)
pico_enable_stdio_usb(main_dashboard 1)