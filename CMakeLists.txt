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

add_executable(${PROJECT_NAME}
                main.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wextra)


target_link_libraries(${PROJECT_NAME} boost::boost)