cmake_minimum_required(VERSION 3.19)
project(network_lib)

set(SOURCE_FILES
    hash_calculator.cpp
    pch.cpp
    timer.cpp
)

set(HEADER_FILES
    client_handler.h
    consoleParams.h
    connection.h
    data_processor_interface.h
    decryptor.h
    defs.h
    encryptor.h
    file_reader.h
    file_writer.h
    hash_calculator.h
    my_packet.h
    pch.h
    server.h
    session.h
    thread_safe_queue.h
    timer.h
)

# Define the executable target
add_library(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

# Precompiled header
target_precompile_headers(${PROJECT_NAME} PRIVATE pch.h)
