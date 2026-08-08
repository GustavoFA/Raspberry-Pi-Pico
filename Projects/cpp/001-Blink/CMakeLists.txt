# This is a template of CMakeLists.txt for a Raspberry Pi Pico project using FreeRTOS. It is intended to be copied into a new project folder and modified as needed.
cmake_minimum_required(VERSION 3.12)

# Automatically use the current folder name as the project/executable name
get_filename_component(NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
# Insert manually the name of your project if you want to use a different name
# set(NAME Example) # <-- Change this to your project name if you want to use a different name than the folder name

include(pico_sdk_import.cmake)

project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Shared FreeRTOS Kernel location (same for every project on this machine)
SET(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/port/FreeRTOS-Kernel" CACHE STRING "Local Config")
include_directories("${FREERTOS_CONFIG_FILE_DIRECTORY}")
SET(FREERTOS_KERNEL_PATH "$ENV{HOME}/FreeRTOS_Kernel/FreeRTOS-Kernel" CACHE STRING "Shared FreeRTOS Kernel")
include(FreeRTOS_Kernel_import.cmake)

add_subdirectory(src)

# Deploy/package steps
install(CODE "execute_process(COMMAND $ENV{HOME}/bin/picoDeploy.sh ${CMAKE_CURRENT_BINARY_DIR}/src/${NAME}.elf)")
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/src/${NAME}.uf2
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

set(PICO_BOARD pico_w CACHE STRING "Board type")

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)