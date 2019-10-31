#include "netwcommon.h"

#define CUST_MAKEWORD(a, b) ((WORD)(((unsigned)(a))|(((unsigned)(b))<<8u)))

error_code wsa_start() {
    WSAData wsaData;
    int err = 0;
    if ((err = WSAStartup(CUST_MAKEWORD(2, 2), &wsaData)) != NO_ERROR) {
        PRINT_ERROR("WSAStartup %d", err);
        return WSAStarterr;
    }
    return Noerr;
}

error_code getaddr_for(const char *target_addr, const char *port, addrinfo *hints, addrinfo **target_addrinfo) {
    if (getaddrinfo(target_addr, port, hints, target_addrinfo) != 0) {
        PRINT_WSA_ERR("getaddrinfo");
        return Addrerr;
    }
    return Noerr;
}

error_code send_data(SOCKET incom_sockd, char *data, size_t data_leng) {
    if (send(incom_sockd, data, data_leng, 0) == SOCKET_ERROR) {
        PRINT_WSA_ERR("send");
        return Senderr;
    }

    return Noerr;
}

error_code rcv_data(SOCKET sockd, char **data, size_t *data_size) {
    char buf[4096] = {0};
    *data_size = 0;
    int recv_leng;
    error_code operes;
    do {
        recv_leng = recv(sockd, buf, sizeof(buf) / sizeof(buf[0]), 0);
        if (recv_leng < 0) {
            PRINT_WSA_ERR("recv");
            return Recverr;
        }
        if (recv_leng > 0) {

            if ((operes = re_memalloc(data, *data_size + recv_leng)) != Noerr) {
                free(*data);
                *data_size = 0;
                return operes;
            }
            memcpy(*data + *data_size, buf, recv_leng * sizeof(buf[0]));
            *data_size += recv_leng;
        }

    } while (recv_leng > 0);

    return Noerr;
}

void print_addr(sockaddr_storage *addr) {
    unsigned long name_leng = INET6_ADDRSTRLEN;
    char targ_name[name_leng];
    if (WSAAddressToStringA((struct sockaddr *) addr, sizeof(sockaddr_storage), NULL, targ_name, &name_leng) != 0) {
        PRINT_WSA_ERR("WSAAddressToStringA");
    } else
        PRINT_FORMAT("connecting to %s\n", targ_name);
}

error_code re_memalloc(char **ptr, size_t size) {
    if ((*ptr = realloc(*ptr, size)) == NULL) {
        PRINT_ERROR("memory allocation %i", errno);
        return Memerr;
    }

    return Noerr;
}

