#ifndef LIGHT_NETW_NETWCLEANUP_H
#define LIGHT_NETW_NETWCLEANUP_H

#include "netwbase.h"
#include "netwtypes.h"


void freepacket(packet_t *packet);

void freemem(char *ptr);

#define GET_OVERLOAD(_1, _2, _3, NAME, ...) NAME

#define BASECLEANUP(cleanme) _Generic((cleanme),                    \
                                        addrinfo*: freeaddrinfo,    \
                                        char*: free,                \
                                        u_char*: free,              \
                                        SOCKET: closesocket,        \
                                        packet_t *: freepacket,     \
                                        FILE *: fclose)             \
                                        (cleanme)                   \

#define _CLEANUP1(n1) W(BASECLEANUP(n1))
#define _CLEANUP2(n1, n2) W(BASECLEANUP(n1);BASECLEANUP(n2))
#define _CLEANUP3(n1, n2, n3) W(BASECLEANUP(n1); BASECLEANUP(n2); BASECLEANUP(n3))

#define CLEANUP(...) GET_OVERLOAD(__VA_ARGS__, _CLEANUP3, _CLEANUP2, _CLEANUP1)(__VA_ARGS__)

#define FINAL_CLEANUP(...) CLEANUP(__VA_ARGS__); WSACleanup(); CLEAR_PREFIX(); logs_cleanup()



#endif //LIGHT_NETW_NETWCLEANUP_H
