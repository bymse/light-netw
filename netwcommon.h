#ifndef NETWORKS_NETWCOMMON_H
#define NETWORKS_NETWCOMMON_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef WINVER
#define WINVER 0x0600
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <signal.h>


#define DEFAULT_PORT "3490"
#define MAX_PACKET_S 1300

typedef struct addrinfo addrinfo;
typedef struct WSAData WSAData;

enum error_code {
    WSAStarterr = -10,
    Addrerr = -9,
    Sockerr = -8,
    Recverr = -7,
    Noerr = 0
};
typedef enum error_code error_code;

error_code wsa_start(WSAData *wsaData, const char *prefix);

#define GET_OVERRIDE(_1, _2, _3, _4, NAME, ...) NAME

#define _CLEANUP0() WSACleanup(); wprintf(L"cleanup done\r\n")
#define _CLEANUP1(addr)                     \
    do{if((addr) != NULL)                   \
        freeaddrinfo((void*)((addr)));      \
    _CLEANUP0();                            \
    }while(0)                               \


#define _CLEANUP2(addr, socket)             \
    do{if((socket) > 0)                     \
        closesocket((socket)+0L);           \
    _CLEANUP1((addr));                      \
    }while(0)                               \

#define _CLEANUP3(addr, socket, file)       \
    do{if((file) != NULL){                  \
        fflush((void *)(file));             \
        fclose((void *)(file));             \
    }                                       \
    _CLEANUP2(addr, socket);                \
    }while(0)                               \

#define CLEANUP(...) GET_OVERRIDE("ignored", ##__VA_ARGS__, _CLEANUP3, _CLEANUP2, _CLEANUP1, _CLEANUP0)(__VA_ARGS__)

#define PRINT_FORMAT(format_str, ...) wprintf(L""format_str"", __VA_ARGS__);

#define PRINT_WSA_ERR(format_str) wprintf(L""format_str"", WSAGetLastError())



#endif
