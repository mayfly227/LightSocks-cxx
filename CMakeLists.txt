cmake_minimum_required(VERSION 3.10)
project(LightSocks)

set(CMAKE_CXX_STANDARD 11)

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

add_executable(LightSocks-client main_client.cpp client.cpp util.cpp)
add_executable(LightSocks-server main_server.cpp server.cpp util cipher.cpp socks5.cpp socks5.h)

target_link_libraries(LightSocks-client base64)
target_link_libraries(LightSocks-server base64)

IF (WIN32)
    target_link_libraries(LightSocks-client event.lib)
    target_link_libraries(LightSocks-server event.lib)
ELSEIF (APPLE)
ELSEIF (UNIX)
    target_link_libraries(LightSocks-client event.a)
    target_link_libraries(LightSocks-server event.a pthread)
ENDIF ()