cmake_minimum_required(VERSION 3.20)
set(TARGET Totoro)
project(${TARGET})

set(CMAKE_CXX_STANDARD 11)

#aux_source_directory(src SRC)
include_directories(include)
include_directories(3rdparty)
link_directories(lib)

#依赖Mole
add_subdirectory(3rdparty/Mole)
include_directories(3rdparty/Mole/include)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


add_executable(${TARGET} main.cpp
        src/Socket.cpp
        src/Connection.cpp
        src/Http.cpp
        src/HttpServer.cpp
        src/SSLSocket.cpp
        src/Configure.cpp
        src/HttpClient.cpp
)

add_library(totoro.core SHARED
        # core
        src/Connection.cpp src/Socket.cpp src/Configure.cpp
)

add_library(totoro.http.server SHARED
        # core
        src/Connection.cpp src/Socket.cpp src/Configure.cpp
        # http
        src/Http.cpp src/HttpServer.cpp src/SSLSocket.cpp
)

add_library(totoro.http.client SHARED
        # socket
        src/Socket.cpp src/SSLSocket.cpp
        # http
        src/Http.cpp src/HttpClient.cpp
)

add_subdirectory(test)

target_link_libraries(${TARGET} pthread ssl crypto Mole.share)
target_link_libraries(totoro.core pthread ssl crypto Mole.share)
target_link_libraries(totoro.http.server pthread ssl crypto Mole.share)
target_link_libraries(totoro.http.client pthread ssl crypto Mole.share)