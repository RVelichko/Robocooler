# Поиск заголовочных файлов и библиотек Thrift
#
#  Thrift_VERSION      - версия Thrift
#  Thrift_INCLUDE_DIR  - директория заголочных файлов Thrift
#  Thrift_LIBRARIES    - библиотеки Thrift
#  Thrift_FOUND        - true если Thrift найдена в системе

exec_program(thrift ARGS -version OUTPUT_VARIABLE Thrift_VERSION RETURN_VALUE Thrift_RETURN)

find_path(Thrift_INCLUDE_DIR Thrift.h PATHS /usr/local/include/thrift /opt/local/include/thrift)

set(Thrift_LIB_PATHS /usr/local/lib /opt/local/lib)
find_library(Thrift_LIB NAMES thrift PATHS ${Thrift_LIB_PATHS})
find_library(Thrift_NB_LIB NAMES thriftnb PATHS ${Thrift_LIB_PATHS})

if(Thrift_VERSION MATCHES "^Thrift version"
   AND LibEvent_LIBRARIES
   AND LibEvent_INCLUDE_DIR
   AND Thrift_LIB
   AND Thrift_NB_LIB
   AND Thrift_INCLUDE_DIR)
    set(Thrift_FOUND TRUE)
    set(Thrift_LIBRARIES ${Thrift_LIB} ${Thrift_NB_LIB})
else()
    set(Thrift_FOUND FALSE)
    if (NOT LibEvent_LIBRARIES OR NOT LibEvent_INCLUDE_DIR)
        message(STATUS "[E] Для Thrift необходима libevent")
    endif()
endif()

mark_as_advanced(
  Thrift_LIBRARIES
  Thrift_FOUND
  Thrift_INCLUDE_DIR
)
