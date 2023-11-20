cmake_minimum_required(VERSION 3.20)
set(TARGET Totoro)
project(${TARGET})

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(src SRC)
aux_source_directory(include INCLUDE)
include_directories(include)
link_directories(lib)

# 依赖fmt
if(NOT ${TRAGET} fmt)
include(thirdparty/fmt.cmake)
endif()

add_executable(${TARGET} main.cpp ${SRC} ${INCLUDE} test/Test.h
        include/core/ForwardEpoller.h)

target_link_libraries(${TARGET} pthread fmt::fmt ssl crypto)