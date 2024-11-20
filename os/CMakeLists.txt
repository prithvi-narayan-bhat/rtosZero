set(OS_DIR ${CMAKE_CURRENT_SOURCE_DIR}) # Set root directory

set(OS_SRC ${OS_DIR}/os.c PARENT_SCOPE)

add_executable(os
    ${OS_DIR}/os.c
)

# Set executable output directory
set_target_properties(os
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/deploy
)

target_include_directories(os PUBLIC
    ${BOARD_INC_DIR}
    ${UTILS_INC_DIR}
    ${KERNEL_INC_DIR}
    ${APPS_INC_DIR}
)

target_link_libraries(os PRIVATE
    BOARD_LIB
    UTILS_LIB
    KERNEL_LIB
    APPS_LIB
)
