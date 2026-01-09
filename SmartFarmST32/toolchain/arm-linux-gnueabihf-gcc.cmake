set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armhf)
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf)
set(CPACK_RPM_PACKAGE_ARCHITECTURE armv7hl)

#Optional
#set(CMAKE_SYSROOT "")

if(NOT "${CMAKE_SYSROOT}" STREQUAL "")
    set(SYSROOT_TARGET_NAME "arm-linux-gnueabihf")
endif()

set(TOOLCHAIN_PREFIX "arm-linux-gnueabihf-")

set(SIZE_OPTIMIZE_ENABLE FALSE)

include(${CMAKE_CURRENT_LIST_DIR}/toolchain_common.cmake)
