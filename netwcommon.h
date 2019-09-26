#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef WINVER
#define WINVER 0x0600
#endif

#ifndef NETWORKS_NETWCOMMON_H
#define NETWORKS_NETWCOMMON_H
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

#define _MAKEWORD(a, b)    ((WORD)(((unsigned)(a))|(((unsigned)(b))<<8u)))

#define _GET_OVERRIDE(_1, _2, _3, _4, NAME, ...) NAME

typedef struct addrinfo addrinfo;
