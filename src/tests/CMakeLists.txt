# Пример добавления теста:
# add_unit_test(<Имя теста> <библиотека 1>  ... <библиотека N>)

include(${CMAKE_DIR}/UTest.cmake)

add_unit_test(ut_json_extractor driver_modules log tty_io pthread ${Boost_LIBRARIES})
add_unit_test(ut_command_handler driver_modules rfid_module log tty_io pthread ${LIBSERIAL_LIBRARY} ${Boost_LIBRARIES})
add_unit_test(ut_product_send log pthread ${Boost_LIBRARIES})
