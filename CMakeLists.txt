cmake_minimum_required(VERSION 3.14)
project(Networks C)

set(CMAKE_C_STANDARD 11)

find_library(WINSOCK_LIBRARY winsock32)

add_executable(Networks main.c)

target_link_libraries(Networks ${WINSOCK_LIBRARY})