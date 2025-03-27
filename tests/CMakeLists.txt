cmake_minimum_required(VERSION 3.2)
project(run_unit_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-ggdb -fprofile-arcs -ftest-coverage")
SET(CMAKE_CXX_FLAGS "-g ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

find_package(GTest REQUIRED)
find_package(Boost REQUIRED)
find_package(cryptopp REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)
find_package(OpenSSL REQUIRED CONFIG)

# number of packages to test
add_definitions(-DQUEUE_DATA_NUM=6)
add_definitions(-DTEST)
add_definitions(-DDEBUG)

include_directories(../src)

include(${CMAKE_CURRENT_LIST_DIR}/network/network_test.cmake)

add_executable(
        ${PROJECT_NAME}
        test.cpp
        pch.h
        pch.cpp
        ${NETWORK_TEST}
)

# Precompiled header
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../src/pch.h)

target_link_libraries(
        ${PROJECT_NAME}
        network_lib
        GTest::gtest
        GTest::gtest_main
        boost::boost
        spdlog::spdlog
        cryptopp-static
        openssl::openssl
)

enable_testing()

add_test(run_unit_tests ${PROJECT_NAME})

target_compile_definitions(run_unit_tests PUBLIC PATH_TO_ASSETS="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

