cmake_minimum_required(VERSION 3.14)
project(Networks C)

set(CMAKE_C_STANDARD 11)

find_library(WINSOCK_LIBRARY Ws2_32)

add_executable(Networks main.c simplenetw.h netwcommon.h netwcommon.c server.c server.h client.c client.h netwoptions.c netwoptions.h filesys.c filesys.h)

target_link_libraries(Networks ${WINSOCK_LIBRARY})