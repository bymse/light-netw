#ifndef NETWORKS_SERVER_H
#define NETWORKS_SERVER_H

#include "common/netwcommon.h"
#include "common/filesys.h"

#define BACKLOG 10   // how many pending connections queue will hold
#define SERVER_PREFIX "SERVER"

error_code run_server(const netwopts *options);

#define SERVER_HINTS(routing) &((addrinfo) {         \
            .ai_family = routing,          \
            .ai_socktype = SOCK_STREAM,     \
            .ai_flags = AI_PASSIVE,         \
            .ai_addr = NULL,                \
            .ai_canonname = NULL,           \
            .ai_next = NULL,                \
            .ai_protocol = 0,               \
            .ai_addrlen = 0,                \
})

#endif //NETWORKS_SERVER_H
