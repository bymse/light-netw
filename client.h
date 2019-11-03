#ifndef NETWORKS_CLIENT_H
#define NETWORKS_CLIENT_H

#include "netwcommon.h"
#include "filesys.h"

#define CLIENT_PREFIX "CLIENT"

error_code run_client(const netwopts *options);

//todo: check without AI_CANONNAME        
#define CLIENT_HINTS(routing) &((addrinfo) {         \
            .ai_family = routing,          \
            .ai_socktype = SOCK_STREAM,     \
            .ai_flags = AI_CANONNAME,       \
            .ai_addr = NULL,                \
            .ai_canonname = NULL,           \
            .ai_next = NULL,                \
            .ai_protocol = 0,               \
            .ai_addrlen = 0,                \
})

#endif //NETWORKS_CLIENT_H
