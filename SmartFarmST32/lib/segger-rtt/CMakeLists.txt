#if want toolchain setting
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "")
endif()
#or
#cmake -DCMAKE_TOOLCHAIN_FILE="" ..

set(CMAKE_EXPORT_COMPILE_COMMANDS True CACHE INTERNAL "")

cmake_minimum_required(VERSION 3.13)
project(segger-rtt
    VERSION 7.54.0
    DESCRIPTION "Segger Real Time Transfer(RTT) GCC Library"
    HOMEPAGE_URL "https://git.wlsdn-dev.synology.me/project-01/interface/segger-rtt"
)

# specify the C/C++ standard
if(NOT DEFINED CMAKE_CXX_STANDARD_REQUIRED)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
endif()
if(NOT DEFINED CMAKE_C_STANDARD_REQUIRED)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED True)
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(compile_options
    #add an option to all of language
    -Wall

    #add an option to specific build type
    $<$<CONFIG:Release>:$<IF:$<BOOL:${SIZE_OPTIMIZE_ENABLE}>,-Os,-O2>>

    $<$<CONFIG:RelWithDebInfo>:$<IF:$<BOOL:${SIZE_OPTIMIZE_ENABLE}>,-Os,-O2>>
    $<$<CONFIG:RelWithDebInfo>:$<IF:$<BOOL:${SIZE_OPTIMIZE_ENABLE}>,-g,-g3>>

    $<$<CONFIG:Debug>:$<IF:$<BOOL:${SIZE_OPTIMIZE_ENABLE}>,-Os,-Og>>
    $<$<CONFIG:Debug>:$<IF:$<BOOL:${SIZE_OPTIMIZE_ENABLE}>,-g,-g3>>

    #add an option to specific language
    #$<$<COMPILE_LANGUAGE:C>:-DC_DEF>
    #$<$<COMPILE_LANGUAGE:CXX>:-DCXX_DEF>
)

set(link_options
    
)

set(link_directory_list
    "${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}"
)

set(header_directory_list
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

set(source_directory_list
    "${CMAKE_CURRENT_LIST_DIR}/src"
)

set(source_exclude_pattern
    ""
)

unset(header_file_list)
foreach(target_directory ${header_directory_list})
    file(GLOB_RECURSE tmp_list CONFIGURE_DEPENDS "${target_directory}/*.h*")
    list(APPEND header_file_list ${tmp_list})
endforeach()

unset(source_file_list)
foreach(target_directory ${source_directory_list})
    file(GLOB_RECURSE tmp_list CONFIGURE_DEPENDS "${target_directory}/*.c*")
    list(APPEND source_file_list ${tmp_list})
endforeach()

foreach(target_pattern ${source_exclude_pattern})
    list(FILTER source_file_list EXCLUDE REGEX "${target_pattern}")
endforeach()

add_library(segger_rtt OBJECT ${source_file_list})
target_link_directories(segger_rtt PUBLIC ${link_directory_list})
target_include_directories(segger_rtt PUBLIC ${header_directory_list})
target_compile_options(segger_rtt PUBLIC ${compile_options})
target_link_options(segger_rtt PUBLIC ${link_options})

include_directories(${header_directory_list}) #for subproject
add_compile_options(${compile_options}) #for subproject
add_link_options(${link_options}) #for subproject

#if want include subproject
#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/subproject)

#target_link_libraries(segger_rtt cmake_target_name)

#if want cmake modules based library link
#find_package(Threads REQUIRED)
#target_link_libraries(segger_rtt pthread)

#if want pkg-config based library link
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(LIBPNG REQUIRED IMPORTED_TARGET libpng)
#target_link_libraries(segger_rtt PkgConfig::LIBPNG)

