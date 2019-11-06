cmake_minimum_required(VERSION 3.14)
project(light-netw C)

set(CMAKE_C_STANDARD 11)

add_library(light-netw.common SHARED
        common/src/netwcommon.c
        common/src/netwoptions.c
        common/src/filesys.c
        common/src/netwcleanup.c
        common/src/netwlogging.c)

add_executable(light-netw main.c client-server/src/server.c client-server/src/client.c)

target_link_libraries(light-netw.common Ws2_32)

target_link_libraries(light-netw light-netw.common)