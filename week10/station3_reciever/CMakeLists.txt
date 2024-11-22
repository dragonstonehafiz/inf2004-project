add_executable(
    week10_station3_reciever
    week10_station3_reciever.c
    )
    
target_compile_definitions(week10_station3_reciever PRIVATE)

target_include_directories(week10_station3_reciever PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../../common # for our common lwipopts
        )


target_link_libraries(week10_station3_reciever 
    pico_stdlib 
    pico_cyw43_arch_lwip_threadsafe_background
    )

pico_add_extra_outputs(week10_station3_reciever)
pico_enable_stdio_usb(week10_station3_reciever 1)