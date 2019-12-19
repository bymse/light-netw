#ifndef LIGHT_NETW_NETWICMP_H
#define LIGHT_NETW_NETWICMP_H

#include <time.h>
#include "../common/netwbase.h"
#include "../common/netwcommon.h"
#include "../common/netwfcs.h"


#define PING_PREFIX "PING"

#define BUF_SIZE 1500u
#define IP_HEADER_SIZE 20u
#define ICMP_PAYLOAD 100
#define IP_ICMP_BUF_SIZE IP_HEADER_SIZE + ICMP_PAYLOAD + sizeof(icmp_header)

#define PING_HINTS(routing) &((addrinfo) {  \
            .ai_family = routing,           \
            .ai_socktype = SOCK_RAW,        \
            .ai_protocol = IPPROTO_ICMP,    \
            .ai_flags = AI_CANONNAME,       \
            .ai_addr = NULL,                \
            .ai_canonname = NULL,           \
            .ai_next = NULL,                \
            .ai_addrlen = 0,                \
})

typedef struct icmp_header {
    u_char Type;
    u_char Code;
    u_short Checksum;
    u_short Identifier;
    u_short SequenceNumber;
} __attribute__((packed)) icmp_header;

error_code ping(const netwopts *options);

#endif //LIGHT_NETW_NETWICMP_H
