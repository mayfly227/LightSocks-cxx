cmake_minimum_required(VERSION 3.10)
project(LightSocks)

set(CMAKE_CXX_STANDARD 11)
#aux_source_directory(. SOURCE_CODE)

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

#添加子库
add_subdirectory(lib/base64)

#add_executable(LightSocks-client ${SOURCE_CODE})
add_executable(LightSocks-client main_client.cpp client.cpp util.cpp)
add_executable(LightSocks-server main_server.cpp server.cpp util cipher.cpp)

target_link_libraries(LightSocks-client base64)
target_link_libraries(LightSocks-server base64)

IF (WIN32)
    target_link_libraries(LightSocks-client event.lib)
    target_link_libraries(LightSocks-server event.lib)
ELSEIF (APPLE)
    MESSAGE(STATUS "Now is Apple systems.")
ELSEIF (UNIX)
    MESSAGE(STATUS "Now is UNIX-like OS's. Including aPPLE os x  and CygWin")
    target_link_libraries(LightSocks-client event)
    target_link_libraries(LightSocks-server event)
ENDIF ()