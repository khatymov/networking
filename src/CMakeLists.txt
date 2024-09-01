cmake_minimum_required(VERSION 3.19)
project(network_lib)
# Source files
set(SOURCE_FILES
    hash_calculator.cpp
    client_handler.cpp
)

set(HEADER_FILES
    defs.h
    my_packet.h
    thread_safe_queue.h
    data_processor_interface.h
    connection.h
    file_reader.h
    file_writer.h
    decryptor.h
    encryptor.h
    session.h
    server.h
    client_handler.h
    consoleParams.h
    hash_calculator.h
)

# Define the executable target
add_library(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

# Precompiled header
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.h)
