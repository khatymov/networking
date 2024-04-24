cmake_minimum_required(VERSION 3.2)
project(networking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

find_package(Boost REQUIRED)

IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF()

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/externals/spdlog/include
)

include(${CMAKE_CURRENT_LIST_DIR}/app/common/common.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/app/server/server.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/app/connection/connection.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/app/file/file.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/app/client/client.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/app/packet/packet.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/app/timer/timer.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/app/cryptographer/cryptographer.cmake)

add_executable(${PROJECT_NAME}
                main.cpp
                ${COMMON}
                ${CONNECTION}
                ${SERVER}
                ${CLIENT}
                ${FILE}
                ${PACKET}
                ${TIMER}
                ${CRYPTOGRAPHER}
)

#target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wextra)

target_link_libraries(${PROJECT_NAME} boost::boost -lcrypto)

add_subdirectory(tests)
