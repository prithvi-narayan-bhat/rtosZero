set(APPS_DIR ${CMAKE_CURRENT_SOURCE_DIR})           # Set root directory

set(APPS_SRC_DIR ${APPS_DIR}/src)                   # Set source directory
set(APPS_INC_DIR ${APPS_DIR}/include)               # Set include directory

set(APPS_SRCS                                       # Set sources
    ${APPS_SRC_DIR}/tasks.c
)

add_library(APPS_LIB                                # Create library
    ${APPS_SRCS}
    ${APPS_INC_DIR}
)

target_include_directories(APPS_LIB PUBLIC          # Export headers
    ${APPS_INC_DIR}
)

target_link_libraries(APPS_LIB PUBLIC
    BOARD_LIB
    KERNEL_LIB
)