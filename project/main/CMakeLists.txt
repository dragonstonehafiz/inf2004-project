if (EXISTS ${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c)
    add_executable(
        main
        final.c
        )

    target_compile_definitions(main PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
        LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)
        PING_USE_SOCKETS=1
        )
        
    target_include_directories(
        main PRIVATE
        )

    target_link_libraries(main 
        pico_stdlib 
        hardware_adc
        pico_cyw43_arch_lwip_sys_freertos
        pico_lwip_iperf
        FreeRTOS-Kernel-Heap4
        common_lib
        )

    pico_add_extra_outputs(main)
    pico_enable_stdio_usb(main 1)
endif()