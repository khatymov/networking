cmake_minimum_required(VERSION 3.2)
project(network LANGUAGES CXX)

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

include_directories(src)

add_executable(${PROJECT_NAME}
                main.cpp
)

add_subdirectory(src)
#target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wextra)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib boost::boost -lcrypto -lcryptopp)

add_subdirectory(tests)
