#ifndef LIGHT_NETW_NETWBASE_H
#define LIGHT_NETW_NETWBASE_H

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
#include <ws2tcpip.h>
#include <winsock2.h>

//wrapper
#define W(statement) do{statement;}while(0)

#endif //LIGHT_NETW_NETWBASE_H
