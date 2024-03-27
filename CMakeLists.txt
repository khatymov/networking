cmake_minimum_required(VERSION 3.2)
project(my_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_CURRENT_LIST_DIR}/my_class/my_class.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/src/move/move.cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

add_executable(${PROJECT_NAME}
                main.cpp
                ${MY_CLASS}
                ${MOVE}
)

target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wextra)

add_subdirectory(tests)
