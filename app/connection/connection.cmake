include_directories( ${CMAKE_CURRENT_LIST_DIR} )

include(${CMAKE_CURRENT_LIST_DIR}/socket_messenger/socket_messenger.cmake)

set( CONNECTION
        ${CMAKE_CURRENT_LIST_DIR}/connection.h
        ${CMAKE_CURRENT_LIST_DIR}/socket_file_connection.h
        ${CMAKE_CURRENT_LIST_DIR}/socket_file_connection.cpp
   )
