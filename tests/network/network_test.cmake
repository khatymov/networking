include_directories( ${CMAKE_CURRENT_LIST_DIR} )

set( NETWORK_TEST
        ${CMAKE_CURRENT_LIST_DIR}/network_test.h
        ${CMAKE_CURRENT_LIST_DIR}/network_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/network_modules_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/my_packet.h
        ${CMAKE_CURRENT_LIST_DIR}/thread_safe_queue.h
        ${CMAKE_CURRENT_LIST_DIR}/connection.h
        ${CMAKE_CURRENT_LIST_DIR}/file_writer.h
        ${CMAKE_CURRENT_LIST_DIR}/data_processor_interface.h
        ${CMAKE_CURRENT_LIST_DIR}/decryptor.h
        ${CMAKE_CURRENT_LIST_DIR}/server.h
        ${CMAKE_CURRENT_LIST_DIR}/server.cpp
        ${CMAKE_CURRENT_LIST_DIR}/client_handler.h
        ${CMAKE_CURRENT_LIST_DIR}/client_handler.cpp
        ${CMAKE_CURRENT_LIST_DIR}/consoleParams.h
        ${CMAKE_CURRENT_LIST_DIR}/thread_safe_queue_test.h
        ${CMAKE_CURRENT_LIST_DIR}/thread_safe_queue_test.cpp
   )
