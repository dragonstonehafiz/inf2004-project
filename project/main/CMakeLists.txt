if (EXISTS ${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c)
    add_executable(
        main_car
        main.c
        setup.c
        reciever.c
        states.c
        )

    target_compile_definitions(main_car PRIVATE
        WIFI_SSID=\"bighowdy\"
        WIFI_PASSWORD=\"yeedyourlasthaw\"
        )
        
    target_include_directories(
        main_car PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../../common # for our common lwipopts
        )

    target_link_libraries(main_car
        common_lib
        pico_stdlib 
        hardware_adc
        pico_cyw43_arch_lwip_threadsafe_background
        )

    pico_add_extra_outputs(main_car)
    pico_enable_stdio_usb(main_car 1)
endif()