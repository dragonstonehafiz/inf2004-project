if (EXISTS ${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c)
    add_executable(
        week10_station1
        main.c
        ${PICO_LWIP_CONTRIB_PATH}/apps/ping/ping.c
        )
    target_compile_definitions(week10_station1 PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
        LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)
        PING_USE_SOCKETS=1
        )
    target_include_directories(week10_station1 PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/../common # for our common lwipopts
            ${PICO_LWIP_CONTRIB_PATH}/apps/ping
            )
    target_link_libraries(week10_station1 
        pico_stdlib hardware_adc
        pico_cyw43_arch_lwip_sys_freertos
        pico_lwip_iperf
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        )
    pico_add_extra_outputs(week10_station1)
    pico_enable_stdio_usb(week10_station1 1)
endif()