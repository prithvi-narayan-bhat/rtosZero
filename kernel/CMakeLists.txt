set(KERNEL_DIR ${CMAKE_CURRENT_SOURCE_DIR})                     # Set root directory

set(KERNEL_SRC_DIR ${KERNEL_DIR}/src)                           # Set source directory
set(KERNEL_INC_DIR ${KERNEL_DIR}/include)                       # Set include directory

set(KERNEL_SRCS                                                 # Set sources
    ${KERNEL_SRC_DIR}/kernel.c
    ${KERNEL_SRC_DIR}/faults.c
    ${KERNEL_SRC_DIR}/mm.c
    ${KERNEL_SRC_DIR}/systemRegisters.s
)

add_library(KERNEL_LIB ${KERNEL_SRCS} ${KERNEL_INC_DIR})        # Create library

target_include_directories(KERNEL_LIB PUBLIC ${KERNEL_INC_DIR}) # Export headers

target_link_libraries(KERNEL_LIB PUBLIC
    BOARD_LIB
    UTILS_LIB
)