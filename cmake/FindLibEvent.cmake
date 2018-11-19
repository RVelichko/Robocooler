# Поиск заголовочных файлов и библиотек LibEvent
#
#  LibEvent_INCLUDE_DIR -- директория заговолоков LibEvent.
#  LibEvent_LIBRARIES   -- библиотеки LibEvent.
#  LibEvent_FOUND       -- результат поиска LibEvent.

find_path(LibEvent_INCLUDE_DIR event.h PATHS /usr/local/include /opt/local/include)

set(LibEvent_LIB_PATHS /usr/local/lib /opt/local/lib)
find_library(LibEvent_LIB NAMES event PATHS ${LibEvent_LIB_PATHS})

if(LibEvent_LIB AND LibEvent_INCLUDE_DIR)
    set(LibEvent_FOUND TRUE)
    set(LibEvent_LIBRARIES ${LibEvent_LIB})
else()
    set(LibEvent_FOUND FALSE)
endif()

mark_as_advanced(
    LibEvent_LIBRARIES
    LibEvent_INCLUDE_DIR
    )
