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

char *GLOBAL_PREFIX;

#define SET_PREFIX(prefix) GLOBAL_PREFIX = ""prefix"-> "
#define CLEAR_PREFIX() GLOBAL_PREFIX = ""

typedef enum error_code {
    Opterr = -100,
    Patherr = -99,
    Filerr = -98,
    Memerr = -97,
    Packerr = -50,
    WSAStarterr = -10,
    Addrerr = -9,
    Sockerr = -8,
    Recverr = -7,
    Binderr = -6,
    Listenerr = -5,
    Accepterr = -4,
    Senderr = -3,
    Noerr = 0
} error_code;


typedef enum run_type {
    Invalid_type,
    Server_dirshare,
    Server_message,
    Client_filereq,
    Client_message,

    _run_type_count
} runtype;

static char const *const run_type_names[_run_type_count] = {
        [Invalid_type] = "i",
        [Server_dirshare] = "sd",
        [Server_message] = "sm",
        [Client_filereq] = "cf",
        [Client_message] = "cm",
};


typedef struct netwopts {
    char *port;
    char *hostname;
    char *input_path;
    char *output_path;
    runtype type;
    int routing; //v4 for IPv4, v6 for IPv6
} netwopts;

typedef struct packet {
    error_code state_code;
    size_t data_s;
    char *data;
} packet;

typedef struct addrinfo addrinfo;
typedef struct WSAData WSAData;
typedef struct sockaddr_storage sockaddr_storage;

error_code init(const netwopts *options, addrinfo *hints, addrinfo **target_addrinfo);

error_code send_packet(SOCKET sockd, packet *packet);

error_code rcv_packet(SOCKET sockd, packet *packet);

void print_addr(sockaddr_storage *addr);

error_code re_memalloc(char **ptr, size_t size);

void freepacket(packet *packet);

//region MACRO

#define GET_OVERRIDE(_1, _2, _3, _4, NAME, ...) NAME

#define PRINT_FORMAT(format_str, ...) printf("%s "format_str"", GLOBAL_PREFIX, __VA_ARGS__)

#define PRINT(str) PRINT_FORMAT(str"%s", "")

#define PRINT_ERROR(message, ...) PRINT_FORMAT("error: "message"\r\n", __VA_ARGS__)

#define PRINT_WSA_ERR(str) PRINT_ERROR(""str" %u\r\n", WSAGetLastError())

#define _CLEANUP0() WSACleanup(); PRINT("cleanup done\r\n")
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

//endregion

#endif
