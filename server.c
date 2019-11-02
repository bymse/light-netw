#include "server.h"

error_code bind_to(addrinfo *available_addrs, SOCKET *sockd);

error_code start_listen(SOCKET sockd);

error_code get_connection(SOCKET sockd, SOCKET *incom_sockd);


error_code server_process_connection(SOCKET sockd, const netwopts *options);

error_code dirshare(SOCKET sockd);

error_code send_file(SOCKET sockd, char *file_name);


error_code run_server(const netwopts *options) {
    SET_PREFIX(SERVER_PREFIX);
    error_code operes;
    addrinfo *target_addrinfo = NULL;
    SOCKET sockd = INVALID_SOCKET, income_sockd = INVALID_SOCKET;

    if ((operes = init(options, SERVER_HINTS, &target_addrinfo)) != Noerr) {
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

    PRINT("waiting for connection...\r\n");
    if ((operes = get_connection(sockd, &income_sockd)) != Noerr) {
        CLEANUP(target_addrinfo, sockd);
        return operes;
    }

    operes = server_process_connection(income_sockd, options);

    closesocket(income_sockd);
    CLEANUP(target_addrinfo, sockd);
    CLEAR_PREFIX();
    return operes;
}


error_code bind_to(addrinfo *available_addrs, SOCKET *sockd) {
    addrinfo *p;
    char yes[] = {1};
    for (p = available_addrs; p != NULL; p = p->ai_next) {
        if ((*sockd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == INVALID_SOCKET) {
            PRINT_WSA_ERR("socket");
            continue;
        }

        if (setsockopt(*sockd, SOL_SOCKET, SO_REUSEADDR, yes,
                       sizeof(char)) == SOCKET_ERROR) {
            PRINT_WSA_ERR("setsockopt");
            return Sockerr;
        }

        if (bind(*sockd, p->ai_addr, p->ai_addrlen)) {
            close(*sockd);
            PRINT_WSA_ERR("bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        PRINT("socket connect fail\r\n");
        return Binderr;
    }
    return Noerr;
}

error_code start_listen(SOCKET sockd) {

    if (listen(sockd, BACKLOG) == SOCKET_ERROR) {
        PRINT_WSA_ERR("listen");
        return Listenerr;
    }

    return Noerr;
}

error_code get_connection(SOCKET sockd, SOCKET *incom_sockd) {

    sockaddr_storage incom_addr;
    int addr_length = sizeof(sockaddr_storage);

    if ((*incom_sockd = accept(sockd, (struct sockaddr *) &incom_addr, &addr_length)) == INVALID_SOCKET) {
        PRINT_WSA_ERR("accept");
        return Accepterr;
    }

    print_addr(&incom_addr);
    return Noerr;
}


error_code server_process_connection(SOCKET sockd, const netwopts *options) {
    error_code operes;
    switch (options->type) {
        case Server_dirshare:
            operes = dirshare(sockd);
            break;
        case Server_message:
            PRINT("operation not implemented");
        default:
            operes = Opterr;
            break;
    }
    return operes;
}

error_code dirshare(SOCKET sockd) {
    error_code operes;
    packet packet = {
            .state_code = Noerr,
            .data = NULL,
            .data_s = -1
    };
    if ((operes = rcv_packet(sockd, &packet)) != Noerr) {
        freepacket(&packet);
        return operes;
    }

    if (packet.state_code == Noerr) {
        operes = send_file(sockd, packet.data);
    } else {
        PRINT_FORMAT("state code from client %u", packet.state_code);
    }

    freepacket(&packet);
    return operes;
}

error_code send_file(SOCKET sockd, char *file_name) {
    error_code operes;
    unsigned long data_size = 0;
    char *data = NULL;
    packet packet;

    operes = try_read_file(file_name, &data, &data_size);

    if (operes == Noerr) {
        packet.data = data;
        packet.data_s = data_size;
        packet.state_code = operes;
    } else {
        packet.data_s = 0;
        packet.state_code = operes;
    }

    operes = send_packet(sockd, &packet);

    free(data);
    return operes;
}


