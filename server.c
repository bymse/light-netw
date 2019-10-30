#include "server.h"

error_code bind_to(addrinfo *available_addrs, SOCKET *sockd);

error_code start_listen(SOCKET sockd);

error_code get_connection(SOCKET sockd, SOCKET *incom_sockd);


error_code process_connection(SOCKET incom_sockd, const netwopts *options);

error_code dirshare(SOCKET sockd);

error_code send_file(SOCKET sockd, char *file_name);


error_code send_data(SOCKET incom_sockd, char *data, size_t data_leng);

error_code rcv_data(SOCKET sockd, char **data, size_t *data_size);


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
    SOCKET sockd = INVALID_SOCKET, income_sockd = INVALID_SOCKET;

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
    if ((operes = get_connection(sockd, &income_sockd)) != Noerr) {
        CLEANUP(target_addrinfo, sockd);
        return operes;
    }

    operes = process_connection(income_sockd, options);

    closesocket(income_sockd);
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

error_code start_listen(SOCKET sockd) {

    if (listen(sockd, BACKLOG) == SOCKET_ERROR) {
        PRINT_SERVER_WSA_ERR("listen %u\n");
        return Listenerr;
    }

    return Noerr;
}

error_code get_connection(SOCKET sockd, SOCKET *incom_sockd) {

    sockaddr_storage incom_addr;
    int addr_length = sizeof(sockaddr_storage);

    if ((*incom_sockd = accept(sockd, (struct sockaddr *) &incom_addr, &addr_length)) == INVALID_SOCKET) {
        PRINT_SERVER_WSA_ERR("accept %u\n");
        return Accepterr;
    }

    print_addr(&incom_addr, SERVER_PREFIX);
    return Noerr;
}


error_code process_connection(SOCKET incom_sockd, const netwopts *options) {
    error_code operes;
    switch (options->type) {
        case Server_dirshare:
            operes = dirshare(incom_sockd);
            break;
        case Server_message:
        case Invalid_type:
        case Client_filereq:
        case Client_message:
        case _run_type_count:
        default:
            PRINT_SERVER("operation not implemented");
            operes = Opterr;
            break;
    }
    return operes;
}

error_code dirshare(SOCKET sockd) {
    char *data = NULL;
    size_t data_size = 0;
    error_code operes;
    if ((operes = rcv_data(sockd, &data, &data_size)) != Noerr) {
        free(data);
        return operes;
    }

    operes = send_file(sockd, data);

    free(data);
    return operes;
}

error_code send_file(SOCKET sockd, char *file_name) {
    error_code operes;
    unsigned long data_size = 0;
    char *data;

    if ((operes = try_read_file(file_name, &data, &data_size, SERVER_PREFIX)) != Noerr) {
        free(data);
        return operes;
    }

    operes = send_data(sockd, data, data_size);

    free(data);
    return operes;
}


error_code send_data(SOCKET incom_sockd, char *data, size_t data_leng) {
    PRINT_SERVER("sending data start\n");

    if (send(incom_sockd, data, data_leng, 0) == SOCKET_ERROR) {
        PRINT_SERVER_WSA_ERR("send %u\n");
        return Senderr;
    }

    PRINT_SERVER("sending data end\n");
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
            PRINT_SERVER_WSA_ERR("recv %u\n");
            return Recverr;
        }
        if (recv_leng > 0) {

            if ((operes = re_memalloc(data, *data_size + recv_leng, SERVER_PREFIX)) != Noerr) {
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


