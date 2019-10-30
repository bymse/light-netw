#include "netwcommon.h"

#define CUST_MAKEWORD(a, b)    ((WORD)(((unsigned)(a))|(((unsigned)(b))<<8u)))

error_code wsa_start(const char *prefix) {
    WSAData wsaData;
    int err = 0;
    if ((err = WSAStartup(CUST_MAKEWORD(2, 2), &wsaData))
        != NO_ERROR) {
        PRINT_FORMAT("%s: WSAStartup %d\n", prefix, err);
        return WSAStarterr;
    }
    return Noerr;
}

error_code getaddr_for(const char *target_addr, const char *port, addrinfo *hints, addrinfo **target_addrinfo,
                       const char *prefix) {
    if (getaddrinfo(target_addr, port, hints, target_addrinfo) != 0) {
        PRINT_FORMAT("%s", prefix);
        PRINT_WSA_ERR("getaddrinfo %u\n");
        return Addrerr;
    }
    return Noerr;
}

void print_addr(sockaddr_storage *addr, const char *prefix) {
    unsigned long name_leng = INET6_ADDRSTRLEN;
    char targ_name[name_leng];
    if (WSAAddressToStringA((struct sockaddr *) addr, sizeof(sockaddr_storage), NULL, targ_name, &name_leng) !=
        0) {
        PRINT_FORMAT("%s", prefix);
        PRINT_WSA_ERR("WSAAddressToStringA error %u\n");
    } else
        PRINT_FORMAT("%s connecting to %s\n", prefix, targ_name);
}

error_code re_memalloc(char **ptr, size_t size, char *prefix) {
    if ((*ptr = realloc(*ptr, size)) == NULL) {
        PRINT_FORMAT("%s error: memory allocation %i", prefix, errno);
        return Memerr;
    }

    return Noerr;
}