#include "client.h"

error_code connect_to(addrinfo *target_addrinfo, SOCKET *sockd);

error_code client_process_connection(SOCKET sockd, const netwopts *options);

error_code reqfile(SOCKET sockd, const netwopts *options);


error_code run_client(const netwopts *options) {
    SET_PREFIX(CLIENT_PREFIX);
    error_code operes;
    SOCKET sockd = INVALID_SOCKET;
    addrinfo *target_addrinfo = NULL;

    if ((operes = init(options, CLIENT_HINTS, &target_addrinfo)) != Noerr) {
        CLEANUP(target_addrinfo);
        return operes;
    }

    if ((operes = connect_to(target_addrinfo, &sockd)) != Noerr) {
        CLEANUP(target_addrinfo, sockd);
        return operes;
    }

    operes = client_process_connection(sockd, options);

    CLEANUP(target_addrinfo, sockd);
    CLEAR_PREFIX();
    return operes;
}


error_code connect_to(addrinfo *target_addrinfo, SOCKET *sockd) {
    addrinfo *p;
    for (p = target_addrinfo; p != NULL; p = p->ai_next) {
        if ((*sockd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            PRINT_WSA_ERR("socket");
            continue;
        }

        if (connect(*sockd, p->ai_addr, p->ai_addrlen) == -1) {
            PRINT_WSA_ERR("connect");
            close(*sockd);
            continue;
        }
        break;
    }

    if (p == NULL) {
        PRINT("socket connect fail\r\n");
        return Sockerr;
    }

    print_addr((sockaddr_storage *) p->ai_addr);
    return Noerr;
}

error_code client_process_connection(SOCKET sockd, const netwopts *options) {
    error_code operes;
    switch (options->type) {
        case Client_filereq:
            operes = reqfile(sockd, options);
            break;
        case Client_message:
            PRINT("operation not implemented");
        default:
            operes = Opterr;
            break;
    }
    return operes;
}

error_code reqfile(SOCKET sockd, const netwopts *options) {
    error_code operes;
    packet packet = {
            .state_code = Noerr,
            .data = options->input_path,
            .data_s = strlen(options->input_path) + 1
    };

    if ((operes = send_packet(sockd, &packet)) != Noerr) {
        PRINT_ERROR("send file name %u", operes);
        return operes;
    }

    if ((operes = rcv_packet(sockd, &packet)) != Noerr) {
        freepacket(&packet);
        return operes;
    }

    if (packet.state_code != Noerr) {
        PRINT_FORMAT("Error response from server %i\r\n", packet.state_code);
    } else {
        operes = write_file(options->output_path, packet.data, packet.data_s);
    }

    freepacket(&packet);
    return operes;
}
