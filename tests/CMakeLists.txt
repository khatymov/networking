cmake_minimum_required(VERSION 3.2)
project(run_unit_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-ggdb -fprofile-arcs -ftest-coverage")
SET(CMAKE_CXX_FLAGS "-g ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

find_package(GTest REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/../app/file/file.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/../app/packet/packet.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/packet/packet_test.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/../app/connection/connection.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/connection/connection_test.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/../app/cryptographer/cryptographer.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/encryption/encryption_test.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/../app/server/server.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/network/network_test.cmake)



add_executable(
        ${PROJECT_NAME}
        test.cpp
        ${PACKET}
        ${PACKET_TEST}
        ${CONNECTION}
#        ${CONNECTION_TEST}
        ${FILE}
        ${CRYPTOGRAPHER}
        ${ENCRYPTION_TEST}
        ${NETWORK_TEST}
        ${SERVER}
)

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
