cmake_minimum_required(VERSION 3.13)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

#Optional
#set(CMAKE_SYSROOT "")

if(NOT "${CMAKE_SYSROOT}" STREQUAL "")
    set(SYSROOT_TARGET_NAME "arm-none-eabi")
endif()

set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(TOOLCHAIN_PREFIX "arm-none-eabi-")
include(${CMAKE_CURRENT_LIST_DIR}/toolchain_common.cmake)
set(CMAKE_SIZE_UTIL ${TOOLCHAIN_ROOT}${TOOLCHAIN_PREFIX}size)

set(SIZE_OPTIMIZE_ENABLE TRUE)

add_compile_options(
$<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
$<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
-fno-exceptions
-ffunction-sections
-fdata-sections
)
add_link_options(--specs=nano.specs -u _printf_float -Xlinker --gc-sections)

# cmake script for generating binary and hex format and for flashing
add_custom_command(OUTPUT ${PROJECT_NAME}.srec
    DEPENDS ${PROJECT_NAME}
    COMMAND ${CMAKE_OBJCOPY} -Osrec ${PROJECT_NAME} ${PROJECT_NAME}_v${PROJECT_VERSION}.srec
)
add_custom_command(OUTPUT ${PROJECT_NAME}.hex
    DEPENDS ${PROJECT_NAME}
    COMMAND ${CMAKE_OBJCOPY} -Oihex ${PROJECT_NAME} ${PROJECT_NAME}_v${PROJECT_VERSION}.hex
)
add_custom_command(OUTPUT ${PROJECT_NAME}.bin
    DEPENDS ${PROJECT_NAME}
    COMMAND ${CMAKE_OBJCOPY} -Obinary ${PROJECT_NAME} ${PROJECT_NAME}_v${PROJECT_VERSION}.bin
)

if(NOT TARGET ${PROJECT_NAME}_generate_output)
add_custom_target(${PROJECT_NAME}_generate_output ALL DEPENDS 
    ${PROJECT_NAME}.srec
    ${PROJECT_NAME}.hex
    ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE_UTIL} ${PROJECT_NAME}
)

message("Must be DEFINE -mcpu, -mfloat-abi, -mfpu
example:
    set(CPU_OPTION -mcpu=cortex-m3 -mfloat-abi=soft)    
    or
    set(CPU_OPTION -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16)
    or
    set(CPU_OPTION -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16)
    and
    add_compile_options($\{CPU_OPTION\})
    add_link_options($\{CPU_OPTION\})
    ")
endif()
