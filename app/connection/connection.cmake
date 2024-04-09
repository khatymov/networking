include_directories( ${CMAKE_CURRENT_LIST_DIR} )

include(${CMAKE_CURRENT_LIST_DIR}/socket_messenger/socket_messenger.cmake)

set( CONNECTION
        ${CMAKE_CURRENT_LIST_DIR}/connection_interface.h
        ${CMAKE_CURRENT_LIST_DIR}/connection_interface.cpp
        ${CMAKE_CURRENT_LIST_DIR}/file_writer_connection.h
        ${CMAKE_CURRENT_LIST_DIR}/file_writer_connection.cpp
        ${CMAKE_CURRENT_LIST_DIR}/socket_messenger/socket_messenger.h
        ${CMAKE_CURRENT_LIST_DIR}/tmp_file_writer_connection.h
   )
