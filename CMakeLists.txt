cmake_minimum_required(VERSION 3.10)
project(RTOSZer0 C ASM)

# Check for board type
if(DEFINED BOARD_TYPE)
    message("Target Board: ${BOARD_TYPE}")
    add_definitions(-DBOARD_TYPE=${BOARD_TYPE})
else()
    message(FATAL_ERROR "Please specify the board type using -DBOARD_TYPE=<your_board_type>")
endif()

# Set compiler and linker
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)

# Assembly compiler flags
unset(CMAKE_ASM_FLAGS CACHE)
set(CMAKE_ASM_FLAGS "-x assembler-with-cpp -mcpu=cortex-m4 -mthumb --specs=nosys.specs -c" CACHE STRING "" FORCE)

# C Compiler and linker flags
set(CMAKE_C_FLAGS "-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -Os -ffunction-sections -fdata-sections -MD -std=c99 -ffreestanding")
set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_SOURCE_DIR}/ld_scripts/tm4c123gh6pm.ld -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -specs=nano.specs -Wl,--gc-sections")

add_subdirectory(utils)
add_subdirectory(boards)
add_subdirectory(kernel)
add_subdirectory(os)
add_subdirectory(apps)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)     # Set the output directory for executables
