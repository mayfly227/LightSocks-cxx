cmake_minimum_required(VERSION 3.10)
project(LightSocks)

set(CMAKE_CXX_STANDARD 11)
aux_source_directory(. SOURCE_CODE)

include_directories(lib/json)
IF (WIN32)
    include_directories(lib/event/include)
ELSEIF (UNIX)
    include_directories(/usr/include)
ENDIF ()

include_directories(lib)
IF (WIN32)
    link_directories(lib/event/lib)
ELSEIF (UNIX)
    link_directories(/usr/lib)
ENDIF ()
add_subdirectory(lib/base64)


add_executable(LightSocksClient-cpp ${SOURCE_CODE})

target_link_libraries(LightSocksClient-cpp base64)

IF (WIN32)
    target_link_libraries(LightSocksClient-cpp event.lib)
ELSEIF (APPLE)
    MESSAGE(STATUS "Now is Apple systems.")
ELSEIF (UNIX)
    MESSAGE(STATUS "Now is UNIX-like OS's. Including aPPLE os x  and CygWin")
    target_link_libraries(LightSocksClient-cpp event)
ENDIF ()