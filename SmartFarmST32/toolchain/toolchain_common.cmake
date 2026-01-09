cmake_minimum_required(VERSION 3.13)

if((NOT "${TOOLCHAIN_ROOT}" STREQUAL "") AND (NOT "${TOOLCHAIN_ROOT}" MATCHES "/bin/$"))
    if("${TOOLCHAIN_ROOT}" MATCHES "/bin$")
        string(APPEND TOOLCHAIN_ROOT "/")
    else()
        string(APPEND TOOLCHAIN_ROOT "/bin/")
    endif()
endif()
set(CMAKE_C_COMPILER ${TOOLCHAIN_ROOT}${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_ROOT}${TOOLCHAIN_PREFIX}g++)
set(CMAKE_OBJCOPY ${TOOLCHAIN_ROOT}${TOOLCHAIN_PREFIX}objcopy)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

if(NOT "${CMAKE_SYSROOT}" STREQUAL "")
    set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
    set(CMAKE_SYSROOT_COMPILE ${CMAKE_SYSROOT})
    set(CMAKE_SYSROOT_LINK ${CMAKE_SYSROOT})

    #refer to "$(pkg-config --variable pc_path pkg-config)"
    set(ENV{PKG_CONFIG_LIBDIR} "\
${CMAKE_SYSROOT}/usr/local/lib/${SYSROOT_TARGET_NAME}/pkgconfig:\
${CMAKE_SYSROOT}/usr/lib/${SYSROOT_TARGET_NAME}/pkgconfig:\
${CMAKE_SYSROOT}/usr/local/lib/pkgconfig:\
${CMAKE_SYSROOT}/usr/local/share/pkgconfig:\
${CMAKE_SYSROOT}/usr/lib/pkgconfig:\
${CMAKE_SYSROOT}/usr/share/pkgconfig")
    set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})
    set(ENV{PKG_CONFIG_DIR} "")

    include_directories(SYSTEM ${CMAKE_SYSROOT}/usr/include/${SYSROOT_TARGET_NAME})
    add_link_options(-Wl,-rpath-link,${CMAKE_SYSROOT}/usr/lib/${SYSROOT_TARGET_NAME})
    add_link_options(
        -B${CMAKE_SYSROOT}/lib/${SYSROOT_TARGET_NAME}
        -B${CMAKE_SYSROOT}/usr/lib/${SYSROOT_TARGET_NAME} 
        -B${CMAKE_SYSROOT}/usr/local/lib/${SYSROOT_TARGET_NAME}
    )
endif()

