#set(APP_WS_CLIENT ws-client)
#add_executable(${APP_WS_CLIENT}
#    ws_client.cpp
#    )
#target_link_libraries(${APP_WS_CLIENT}
#    pthread
#    ssl
#    crypto
#    ${Boost_LIBRARIES}
#    )

if(ENABLE_WIRINGPI)
    set(APP_DOORS_SWITCH doors-switch)
    add_executable(${APP_DOORS_SWITCH}
        gpio_doors_switch.cpp
        )
    target_link_libraries(${APP_DOORS_SWITCH}
        driver_modules
        log
        pthread
        wiringPi
        ${Boost_LIBRARIES}
        )
endif(ENABLE_WIRINGPI)


set(APP_RFID_IO rfid-io)
add_executable(${APP_RFID_IO}
    rfid_io.cpp
    )
target_link_libraries(${APP_RFID_IO}
    driver_modules
    rfid_module
    sig_dispatcher
    log
    tty_io
    pthread
    ssl
    crypto
    boost_program_options
    boost_filesystem
    boost_system
    )


set(APP_RFID_READER rfid-reader)
add_executable(${APP_RFID_READER}
    rfid_reader.cpp
    )
target_link_libraries(${APP_RFID_READER}
    driver_modules
    rfid_module
    sig_dispatcher
    log
    tty_io
    pthread
    ssl
    crypto
    boost_program_options
    boost_filesystem
    boost_system
    )


set(APP_TTY_IO tty-io)
add_executable(${APP_TTY_IO}
    tty_io.cpp
    )
target_link_libraries(${APP_TTY_IO}
    log
    tty_io
    pthread
    boost_program_options
    boost_filesystem
    boost_system
    )
