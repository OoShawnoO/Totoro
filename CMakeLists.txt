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

# 依赖fmt
if(NOT ${TRAGET} fmt)
include(3rdparty/fmt.cmake)
endif()

add_executable(${TARGET} main.cpp
        src/Socket.cpp
        src/Connection.cpp
        src/Http.cpp
        src/HttpServer.cpp
        src/SSLSocket.cpp
        src/Configure.cpp
        src/HttpClient.cpp
)

target_link_libraries(${TARGET} pthread fmt::fmt ssl crypto Mole)