#ifndef LIGHT_NETW_NETWCLEANUP_H
#define LIGHT_NETW_NETWCLEANUP_H

#include "netwcommon.h"

#define GET_OVERRIDE(_1, _2, _3, NAME, ...) NAME


//todo add logging

#define _CLEANUP0() WSACleanup();
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


#define CLEANUP(...) GET_OVERRIDE("ignored", ##__VA_ARGS__, _CLEANUP2, _CLEANUP1, _CLEANUP0)(__VA_ARGS__)

error_code test();

#endif //LIGHT_NETW_NETWCLEANUP_H
