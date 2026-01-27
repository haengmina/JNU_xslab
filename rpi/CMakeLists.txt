cmake_minimum_required(VERSION 3.10)
project(EdgeController C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

include_directories(include)

add_executable(edge_controller src/main.c)
