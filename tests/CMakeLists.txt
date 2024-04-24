cmake_minimum_required(VERSION 3.2)
project(run_unit_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-ggdb -fprofile-arcs -ftest-coverage")

SET(CMAKE_CXX_FLAGS "-g ${CMAKE_CXX_FLAGS}")

find_package(GTest REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/../app/packet/packet.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../app/connection/connection.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../app/file/file.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/packet/packet_test.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/connection/connection_test.cmake)

add_executable(
        ${PROJECT_NAME}
        test.cpp
        ${PACKET}
        ${PACKET_TEST}
        ${CONNECTION}
        ${CONNECTION_TEST}
        ${FILE}
)
target_link_libraries(
        ${PROJECT_NAME}
        gtest
        -lcrypto
)

enable_testing()

add_test(run_unit_tests ${PROJECT_NAME})