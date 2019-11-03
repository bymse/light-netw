cmake_minimum_required(VERSION 3.14)
project(light-netw C)

set(CMAKE_C_STANDARD 11)

find_library(WINSOCK_LIBRARY Ws2_32)

add_executable(light-netw main.c light-netw.h netwcommon.h netwcommon.c server.c server.h client.c client.h netwoptions.c netwoptions.h filesys.c filesys.h)

target_link_libraries(light-netw ${WINSOCK_LIBRARY})