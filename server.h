#ifndef NETWORKS_SERVER_H
#define NETWORKS_SERVER_H

#include "netwcommon.h"
#include "filesys.h"

#define BACKLOG 10   // how many pending connections queue will hold
#define SERVER_PREFIX "SERVER"

error_code run_server(const netwopts *options);

#define SERVER_HINTS &((addrinfo) {         \
            .ai_family = AF_INET6,          \
            .ai_socktype = SOCK_STREAM,     \
            .ai_flags = AI_PASSIVE,         \
            .ai_addr = NULL,                \
            .ai_canonname = NULL,           \
            .ai_next = NULL,                \
            .ai_protocol = 0,               \
            .ai_addrlen = 0,                \
})

#define GETADDR_FOR_BIND(port, target_addrinfo) getaddr_for(NULL, port, SERVER_HINTS, target_addrinfo)

#endif //NETWORKS_SERVER_H
