#ifndef NETWORKS_SERVER_H
#define NETWORKS_SERVER_H

#include "netwcommon.h"

#define BACKLOG 10   // how many pending connections queue will hold
#define SERVER_PREFIX "SERVER->"

error_code run_server(const netwopts *options);

#define PRINT_SERVER_WSA_ERR(format_str) PRINT_WSA_ERR(SERVER_PREFIX" error: "format_str)
#define PRINT_SERVER_FORMAT(format_str, ...) PRINT_FORMAT(SERVER_PREFIX" "format_str, __VA_ARGS__)
#define PRINT_SERVER(str) PRINT_SERVER_FORMAT(str"%s", "")

#define SERVER_HINTS &(addrinfo) {         \
            .ai_family = AF_INET6,          \
            .ai_socktype = SOCK_STREAM,     \
            .ai_flags = AI_PASSIVE,         \
            .ai_addr = NULL,                \
            .ai_canonname = NULL,           \
            .ai_next = NULL,                \
            .ai_protocol = 0,               \
            .ai_addrlen = 0,                \
}

#define GETADDR_FOR_BIND(port, target_addrinfo) getaddr_for(NULL, port, SERVER_PREFIX, SERVER_HINTS, target_addrinfo)

typedef struct sockaddr_storage sockaddr_storage;

#endif //NETWORKS_SERVER_H
