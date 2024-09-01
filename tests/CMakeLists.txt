cmake_minimum_required(VERSION 3.2)
project(run_unit_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-ggdb -fprofile-arcs -ftest-coverage")
SET(CMAKE_CXX_FLAGS "-g ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

find_package(GTest REQUIRED)

# number of packages to test
add_definitions(-DQUEUE_DATA_NUM=6)
add_definitions(-DTEST)
add_definitions(-DDEBUG)

include_directories(../src)

include(${CMAKE_CURRENT_LIST_DIR}/network/network_test.cmake)

add_executable(
        ${PROJECT_NAME}
        test.cpp
        ${NETWORK_TEST}
)

# Precompiled header
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../src/pch.h)

target_link_libraries(
        ${PROJECT_NAME}
        gtest
        boost::boost
        -lcrypto
        -lcryptopp
)

enable_testing()

add_test(run_unit_tests ${PROJECT_NAME})

target_compile_definitions(run_unit_tests PUBLIC PATH_TO_ASSETS="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
