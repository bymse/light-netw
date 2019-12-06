#ifndef LIGHT_NETW_NETWICMP_H
#define LIGHT_NETW_NETWICMP_H

#include "../common/netwbase.h"
#include "../common/netwcommon.h"
#include "../common/netwfcs.h"


#define PING_PREFIX "PING"

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


error_code ping(const netwopts *options);

#endif //LIGHT_NETW_NETWICMP_H
