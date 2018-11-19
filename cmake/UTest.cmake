# Процедура сканирует исходный код файлов и добавляет тесты в список ctest.
# Параметры:
#  add_boost_tests(EXECUTABLE EXTRA_ARGS SOURCES)
#   EXECUTABLE -- имя библиотеки для unit тестирования.
#   EXTRA_ARGS -- дополнительные аргументы для передачи в Boost unit test binary.
#   SOURCES    -- список файлов с исходным кодом.
function(add_boost_tests EXECUTABLE EXTRA_ARGS)
    # Проверка переданных аргументов
    if (NOT ARGN)
        message(FATAL_ERROR "No source files given to `add_boost_tests'")
    endif()

    # Идем в исходных кодах макро BOOST_AUTO_TEST_CASE и добавляем тест для найденного имени.
    foreach(source ${ARGN})
        set(REGEX_${EXECUTABLE} "(")
        file(STRINGS "${source}" found_tests REGEX "BOOST_(AUTO|FIXTURE)_TEST_CASE")

        foreach(hit ${found_tests})
            string(REGEX MATCH ".*\\(([a-zA-Z0-9]+)(,|\\)).*" found_name ${hit})

            if (CMAKE_MATCH_1)
                add_test("${CMAKE_MATCH_1}" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXECUTABLE} --run_test=${CMAKE_MATCH_1} ${EXTRA_ARGS})
                set(REGEX_${EXECUTABLE} "${REGEX_${EXECUTABLE}}${CMAKE_MATCH_1}|")
            else()
                string(REGEX MATCH ".*\\(([a-zA-Z0-9]+)(,|\\)).*" incorrect_proc ${hit})
                message(WARNING "test procedure name '${CMAKE_MATCH_1}' at '${source}' does not match naming policy")
            endif()
        endforeach()
        set(REGEX_${EXECUTABLE} "${REGEX_${EXECUTABLE}}|")
    endforeach()

    string(REGEX REPLACE "\\|\\|" ")" REGEX_${EXECUTABLE} ${REGEX_${EXECUTABLE}})
    string(REGEX REPLACE "ut_" "test_" TEST_TARGET ${EXECUTABLE})
    message(STATUS "Add test target:    ${TEST_TARGET}:     ${REGEX_${EXECUTABLE}}")
    add_custom_target(${TEST_TARGET} COMMAND ctest --force-new-ctest-process -R \"${REGEX_${EXECUTABLE}}\")
endfunction(add_boost_tests)


# Процедура добавления теста в систему сборки и тестирования.
# Параметры:
#  add_unit_test(EXECUTABLE EXTRA_ARGS)
#   EXECUTABLE -- имя теста.
#   EXTRA_ARGS -- списко зависимостей.
function(add_unit_test EXECUTABLE EXTRA_ARGS)
    #message(STATUS "Test candidate: `${EXECUTABLE}`")

    if(ALL_TESTS)
        set(${EXECUTABLE} ON)
    endif()

    if(EXECUTABLE)
        message(STATUS "Activate test: `${EXECUTABLE}`")
        # Определение требуемых библиотек.
        set(LIBRARYES ${EXTRA_ARGS})

        foreach(LIB ${ARGN})
            set(LIBRARYES ${LIBRARYES} ${LIB})
        endforeach()
        set(LIBRARYES ${LIBRARYES} boost_unit_test_framework)

        # Создание правила для сборки теста.
        add_executable(${EXECUTABLE} ${EXECUTABLE}.cpp)
        target_link_libraries(${EXECUTABLE} ${LIBRARYES})

        # Добавление теста из собранного бинарника в систему тестирования.
        add_boost_tests(${EXECUTABLE} "--catch_system_errors=no" ${EXECUTABLE}.cpp)
    endif()
endfunction(add_unit_test)
