#include "netwcommon.h"

#define CUST_MAKEWORD(a, b)    ((WORD)(((unsigned)(a))|(((unsigned)(b))<<8u)))

error_code wsa_start(const char *prefix) {
    WSAData wsaData;
    int err = 0;
    if ((err = WSAStartup(CUST_MAKEWORD(2, 2), &wsaData))
        != NO_ERROR) {
        wprintf(L"%s: WSAStartup %d\n", prefix, err);
        return WSAStarterr;
    }
    return Noerr;
}

error_code
getaddr_for(const char *target_addr, const char *port, const char *prefix, addrinfo *hints, addrinfo *target_addrinfo) {
    if (getaddrinfo(target_addr, port, hints, &target_addrinfo) != 0) {
        PRINT_FORMAT("%s", prefix);
        PRINT_WSA_ERR("getaddrinfo %u\n");
        return Addrerr;
    }
    return Noerr;
}

void print_addr(struct sockaddr *addr, const char *prefix) {
    unsigned long name_leng = INET6_ADDRSTRLEN;
    char targ_name[name_leng];
    if (WSAAddressToStringA(addr, sizeof(struct sockaddr), NULL, targ_name, &name_leng) != 0) {
        PRINT_FORMAT("%s", prefix);
        PRINT_WSA_ERR("getaddrinfo %u\n");
    } else
    PRINT_FORMAT(L"%s connecting to %s\n", prefix, targ_name);
}