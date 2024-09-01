include_directories( ${CMAKE_CURRENT_LIST_DIR} )

set( NETWORK_TEST
        ${CMAKE_CURRENT_LIST_DIR}/thread_safe_queue_test.h
        ${CMAKE_CURRENT_LIST_DIR}/thread_safe_queue_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/connection_test.h
        ${CMAKE_CURRENT_LIST_DIR}/connection_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/crypto_test.h
        ${CMAKE_CURRENT_LIST_DIR}/crypto_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/file_test.h
        ${CMAKE_CURRENT_LIST_DIR}/file_test.cpp
   )
