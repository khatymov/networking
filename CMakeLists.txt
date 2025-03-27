cmake_minimum_required(VERSION 3.2)
project(network LANGUAGES CXX)

# Set C++ standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

find_package(Boost REQUIRED)
find_package(cryptopp REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)
find_package(OpenSSL REQUIRED CONFIG)

IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF()

IF (cryptopp_FOUND)
    INCLUDE_DIRECTORIES(${cryptopp_INCLUDE_DIR})
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

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${PROJECT_NAME}_lib
        boost::boost
        spdlog::spdlog
        cryptopp-static
        openssl::openssl)

add_subdirectory(tests)
