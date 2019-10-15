#include "client.h"

error_code connect_to(addrinfo *target_addrinfo, SOCKET *socketd);

error_code recive(SOCKET sockd, FILE *data);

error_code request_data(const char *target_addr, const char *port, FILE *data) {
    error_code oper_res;

    if ((oper_res = wsa_start(CLIENT_PREFIX)) != Noerr) {
        return oper_res;
    }

    SOCKET sockd = INVALID_SOCKET; // NOLINT(hicpp-signed-bitwise)
    addrinfo *target_addrinfo = NULL;

    if ((oper_res = GETADDR_FOR_CONNECT(target_addr, port, target_addrinfo)) != Noerr) {
        CLEANUP(target_addrinfo);
        return oper_res;
    }

    if ((oper_res = connect_to(target_addrinfo, &sockd)) != Noerr) {
        CLEANUP(target_addrinfo, sockd);
        return oper_res;
    }

    oper_res = recive(sockd, data);

    CLEANUP(target_addrinfo, sockd);
    return oper_res;
}

error_code connect_to(addrinfo *target_addrinfo, SOCKET *socketd) {
    addrinfo *p;
    for (p = target_addrinfo; p != NULL; p = p->ai_next) {
        if ((*socketd = socket(p->ai_family, p->ai_socktype,
                               p->ai_protocol)) == -1) {
            PRINT_CLIENT_WSA_ERR("socket %u\n\t(keep trying)\n");
            continue;
        }

        if (connect(*socketd, p->ai_addr, p->ai_addrlen) == -1) {
            PRINT_CLIENT_WSA_ERR("connect %u\n\t(keep trying)\n");
            close(*socketd);
            continue;
        }
        break;
    }

    if (p == NULL) {
        PRINT_CLIENT(L"socket connect fail\n");
        return Sockerr;
    }

    print_addr(p->ai_addr, CLIENT_PREFIX);
    return Noerr;
}

error_code recive(SOCKET sockd, FILE *data) {
    int count;
    char buf[MAX_PACKET_S + 1];

    if ((count = recv(sockd, buf, MAX_PACKET_S, 0)) < 0) {
        PRINT_CLIENT_WSA_ERR(L"recv %u\n");
        return Recverr;
    }

    PRINT_CLIENT_FORMAT("%u bytes were recived");

    buf[count] = '\0';

    fwrite(buf, sizeof(char), count + 1, data);
    return Noerr;
}

