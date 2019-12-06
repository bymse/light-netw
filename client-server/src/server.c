#include "../server.h"

error_code bind_to(addrinfo *available_addrs, SOCKET *sockd);

error_code start_listen(SOCKET sockd);


error_code server_process_connection(SOCKET sockd, const netwopts *options);

error_code dirshare(SOCKET sockd);

error_code send_file(SOCKET sockd, char *file_name);


error_code run_server(const netwopts *options) {

    SET_PREFIX(SERVER_PREFIX);
    logs_init(options->logs_path);
    if (SetCurrentDirectory((LPCSTR) options->input_param) == 0) {
        ERR("SetCurrentDirectory %lu", GetLastError());
        return Patherr;
    }

    error_code operes;
    addrinfo *target_addrinfo = NULL;
    SOCKET sockd = INVALID_SOCKET, income_sockd = INVALID_SOCKET;

    if ((operes = netwinit(options, SERVER_HINTS(options->routing), &target_addrinfo)) != Noerr) {
        FINAL_CLEANUP(target_addrinfo);
        return operes;
    }

    WRITE_FORMAT("work, port: %s, target dir: %s", options->port, options->input_param);

    if ((operes = bind_to(target_addrinfo, &sockd)) != Noerr) {
        FINAL_CLEANUP(target_addrinfo, sockd);
        return operes;
    }

    CLEANUP(target_addrinfo);

    if ((operes = start_listen(sockd)) != Noerr) {
        FINAL_CLEANUP(sockd);
        return operes;
    }

    PRINT("waiting for connection...");
    if ((operes = accept_connect_async(sockd, &income_sockd, 'q')) != Noerr) {
        FINAL_CLEANUP(sockd);
        return operes;
    }

    operes = server_process_connection(income_sockd, options);

    WRITE_FORMAT("work end for :%s", options->port);
    
    FINAL_CLEANUP(income_sockd, sockd);
    return operes;
}


error_code bind_to(addrinfo *available_addrs, SOCKET *sockd) {
    addrinfo *p;
    char yes[] = {1};
    for (p = available_addrs; p != NULL; p = p->ai_next) {
        if ((*sockd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == INVALID_SOCKET) {
            WSA_ERR("socket");
            continue;
        }

        if (setsockopt(*sockd, SOL_SOCKET, SO_REUSEADDR, yes,
                       sizeof(char)) == SOCKET_ERROR) {
            WSA_ERR("setsockopt");
            return Sockerr;
        }

        if (bind(*sockd, p->ai_addr, p->ai_addrlen)) {
            CLEANUP(*sockd);
            WSA_ERR("bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        PRINT("socket connect fail");
        return Binderr;
    }
    return Noerr;
}

error_code start_listen(SOCKET sockd) {

    if (listen(sockd, BACKLOG) == SOCKET_ERROR) {
        WSA_ERR("listen");
        return Listenerr;
    }

    LOG_FORMAT("start listening, socket: %llx", sockd);

    return Noerr;
}


error_code server_process_connection(SOCKET sockd, const netwopts *options) {
    error_code operes;
    switch (options->type) {
        case Server_dirshare:
            operes = dirshare(sockd);
            break;
        default:
            operes = Opterr;
            break;
    }
    return operes;
}

error_code dirshare(SOCKET sockd) {
    error_code operes;
    packet_t packet = {
            .state_code = Noerr,
            .data = NULL,
            .data_s = -1
    };

    LOG("start dirshare");

    if ((operes = rcv_packet_async(sockd, &packet, TRUE, 'q')) != Noerr) {
        CLEANUP(&packet);
        return operes;
    }

    if (packet.state_code == Noerr) {
        WRITE_FORMAT("requested file name: %s", packet.data);
        operes = send_file(sockd, packet.data);
    } else {
        WRITE_FORMAT("state code from client %u", packet.state_code);
        if (packet.data_s < 100)
            WRITE_FORMAT("file name from client %s", packet.data);
        struct packet_t err_packet = {
                .data = "Invalid format", .data_s = sizeof("Invalid format"),
                .state_code = Packerr
        };
        operes = send_packet(sockd, &err_packet);
    }

    LOG("dirshare end");

    CLEANUP(&packet);
    return operes;
}

error_code send_file(SOCKET sockd, char *file_name) {
    error_code operes;
    unsigned long data_size = 0;
    char *data = NULL;
    packet_t packet;

    operes = try_read_file(file_name, &data, &data_size);

    if (operes == Noerr) {
        LOG_FORMAT("response with file: %s", file_name);
        packet.data = data;
        packet.data_s = data_size;
        packet.state_code = operes;
    } else {
        WRITE("response empty");
        packet.data_s = 0;
        packet.state_code = operes;
    }

    operes = send_packet(sockd, &packet);

    CLEANUP(data);
    return operes;
}


