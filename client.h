#ifndef NETWORKS_CLIENT_H
#define NETWORKS_CLIENT_H

#include "netwcommon.h"

error_code request_data(const char *target_addr, const char *port, FILE *data);

#define CLIENT_PREFIX "CLIENT->"

#define PRINT_CLIENT_WSA_ERR(format_str) PRINT_WSA_ERR(CLIENT_PREFIX" error: "format_str)
#define PRINT_CLIENT_FORMAT(format_str, ...) PRINT_FORMAT(CLIENT_PREFIX" "format_str, #__VA_ARGS__)
#define PRINT_CLIENT(str) PRINT_CLIENT_FORMAT(str)

//todo: check without AI_CANONNAME        
#define CONNECT_HINTS &(addrinfo) {         \
            .ai_family = AF_INET6,          \
            .ai_socktype = SOCK_STREAM,     \
            .ai_flags = AI_CANONNAME,       \
            .ai_addr = NULL,                \
            .ai_canonname = NULL,           \
            .ai_next = NULL,                \
            .ai_protocol = 0,               \
            .ai_addrlen = 0,                \
}

#define GETADDR_FOR_CONNECT(target_addr, port, target_addrinfo) getaddr_for(target_addr, port, CLIENT_PREFIX, CONNECT_HINTS, target_addrinfo)


#endif //NETWORKS_CLIENT_H
