if(DEFINED BOARD_TYPE)
    set(BOARD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${BOARD_TYPE})    # Set root directory
else()
    message (FATAL_ERROR "Board not found")
endif()

set(BOARD_SRC_DIR ${BOARD_DIR}/src)                             # Set source directory
set(BOARD_INC_DIR ${BOARD_DIR}/include)                         # Set include directory

file(GLOB BOARD_SRCS "${BOARD_SRC_DIR}/*.c")

add_library(BOARD_LIB ${BOARD_SRCS} ${BOARD_INC_DIR})           # Create library

target_include_directories(BOARD_LIB PUBLIC ${BOARD_INC_DIR})   # Export headers

target_link_libraries(BOARD_LIB PUBLIC UTILS_LIB)