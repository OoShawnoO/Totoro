cmake_minimum_required(VERSION 3.20)

project(Totoro-test)

set(CMAKE_CXX_STANDARD 17)

find_package(GTest CONFIG REQUIRED)

add_executable(totoro.test main.cpp)

target_link_libraries(
        totoro.test
        totoro.core totoro.http.server totoro.http.client
        GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main
)