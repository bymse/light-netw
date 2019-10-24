#include "server.h"

error_code bind_to(addrinfo *available_addrs, SOCKET *sockd);
error_code start_listen(SOCKET sockd);
error_code response_with_data(SOCKET sockd, FILE *data);

error_code get_connection(SOCKET sockd, SOCKET *incom_sockd, sockaddr_storage *incom_addr);
error_code send_data(SOCKET incom_sockd, FILE *data);


error_code run_server(const netwopts *options) {
    if (options == NULL) {
        PRINT_SERVER("inalid options");
        return Opterr;
    }
    
    error_code operes;

    if ((operes = wsa_start(SERVER_PREFIX)) != Noerr) {
        return operes;
    }

    addrinfo *target_addrinfo = NULL;
    SOCKET sockd = INVALID_SOCKET;

    if ((operes = GETADDR_FOR_BIND(options->port, &target_addrinfo)) != Noerr) {
        CLEANUP(target_addrinfo);
        return operes;
    }

    if ((operes = bind_to(target_addrinfo, &sockd)) != Noerr) {
        CLEANUP(target_addrinfo, sockd);
        return operes;
    }

    if ((operes = start_listen(sockd)) != Noerr) {
        CLEANUP(target_addrinfo, sockd);
        return operes;
    }

    PRINT_SERVER("waiting for connection \n");
    operes = response_with_data(sockd, data);

    CLEANUP(target_addrinfo, sockd);
    return operes;
}


error_code bind_to(addrinfo *available_addrs, SOCKET *sockd) {
    addrinfo *p;
    char yes[] = {1};
    for (p = available_addrs; p != NULL; p = p->ai_next) {
        if ((*sockd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == INVALID_SOCKET) {
            PRINT_SERVER_WSA_ERR("socket %u\n\t(keep trying)\n");
            continue;
        }

        if (setsockopt(*sockd, SOL_SOCKET, SO_REUSEADDR, yes,
                       sizeof(char)) == SOCKET_ERROR) {
            PRINT_SERVER_WSA_ERR("setsockopt %u\n");
            return Sockerr;
        }

        if (bind(*sockd, p->ai_addr, p->ai_addrlen)) {
            close(*sockd);
            PRINT_SERVER_WSA_ERR("bind %u\n(keep trying)\n");
            continue;
        }

        break;
    }

    if (p == NULL) {
        PRINT_SERVER("socket connect fail\n");
        return Binderr;
    }
    return Noerr;
}

error_code response_with_data(SOCKET sockd, FILE *data) {
    sockaddr_storage incom_addr;
    SOCKET incom_sock;
    error_code operes;

    if ((operes = get_connection(sockd, &incom_sock, &incom_addr)) != Noerr) {
        closesocket(incom_sock);
        return operes;
    }

    operes = send_data(incom_sock, data);
    closesocket(incom_sock);
    return operes;
}

error_code get_connection(SOCKET sockd, SOCKET *incom_sockd, sockaddr_storage *incom_addr) {

    int addr_length = sizeof(sockaddr_storage);

    if ((*incom_sockd = accept(sockd, (struct sockaddr *) incom_addr, &addr_length)) == INVALID_SOCKET) {
        PRINT_SERVER_WSA_ERR("accept %u\n");
        return Accepterr;
    }

    print_addr(incom_addr, SERVER_PREFIX);
    return Noerr;
}

error_code start_listen(SOCKET sockd) {

    if (listen(sockd, BACKLOG) == SOCKET_ERROR) {
        PRINT_SERVER_WSA_ERR("listen %u\n");
        return Listenerr;
    }

    return Noerr;
}

error_code send_data(SOCKET incom_sockd, FILE *data) {
    char data_buf[MAX_PACKET_S];
    size_t count = fread(data_buf, sizeof(char), MAX_PACKET_S, data);

    PRINT_SERVER("sending data start\n");
    
    if (send(incom_sockd, data_buf, count, 0) == SOCKET_ERROR) {
        PRINT_SERVER_WSA_ERR("send %u\n");
        return Senderr;
    }

    PRINT_SERVER("sending data end\n");
    return Noerr;
}