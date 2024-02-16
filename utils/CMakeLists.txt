set(UTILS_DIR ${CMAKE_CURRENT_SOURCE_DIR})                      # Set root directory

set(UTILS_SRC_DIR ${UTILS_DIR}/src)                             # Set source directory
set(UTILS_INC_DIR ${UTILS_DIR}/include)                         # Set include directory

set(UTILS_SRCS # Set sources
    ${UTILS_SRC_DIR}/commands.c
    ${UTILS_SRC_DIR}/strings.c
    ${UTILS_SRC_DIR}/shell.c
    ${UTILS_SRC_DIR}/wait.s
)

add_library(UTILS_LIB ${UTILS_SRCS} ${UTILS_INC_DIR})           # Create library

target_include_directories(UTILS_LIB PUBLIC ${UTILS_INC_DIR})   # Export headers

target_link_libraries(UTILS_LIB PUBLIC
    BOARD_LIB
    KERNEL_LIB
    )
# target_link_libraries(UTILS_LIB PUBLIC KERNEL_LIB)