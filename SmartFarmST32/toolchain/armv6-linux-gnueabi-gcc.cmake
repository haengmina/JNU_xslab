set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv6l)
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE armel)
set(CPACK_RPM_PACKAGE_ARCHITECTURE armv6l)

#Optional
#set(CMAKE_SYSROOT "")

if(NOT "${CMAKE_SYSROOT}" STREQUAL "")
    set(SYSROOT_TARGET_NAME "arm-linux-gnueabi")
endif()

set(TOOLCHAIN_PREFIX "armv6-linux-gnueabi-")

set(SIZE_OPTIMIZE_ENABLE FALSE)

include(${CMAKE_CURRENT_LIST_DIR}/toolchain_common.cmake)
