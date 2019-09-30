cmake_minimum_required(VERSION 3.14)
project(Networks C)

set(CMAKE_C_STANDARD 11)

find_library(WINSOCK_LIBRARY Ws2_32)

add_executable(Networks main.c netwcommon.h server.c server.h client.c client.h simplenetw.h)

target_link_libraries(Networks ${WINSOCK_LIBRARY})