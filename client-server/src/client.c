#include "../client.h"

error_code connect_to(addrinfo *target_addrinfo, SOCKET *sockd);

error_code client_process_connection(SOCKET sockd, const netwopts *options);

error_code reqfile(SOCKET sockd, const netwopts *options);


error_code run_client(const netwopts *options) {
    SET_PREFIX(CLIENT_PREFIX);
    logs_init(options->logs_path);
    error_code operes;
    SOCKET sockd = INVALID_SOCKET;
    addrinfo *target_addrinfo = NULL;

    if ((operes = netwinit(options, CLIENT_HINTS(options->routing), &target_addrinfo)) != Noerr) {
        FINAL_CLEANUP(target_addrinfo);
        return operes;
    }

    WRITE_FORMAT("work, target addr: %s:%s, file: %s", options->hostname, options->port, options->input_param);

    if ((operes = connect_to(target_addrinfo, &sockd)) != Noerr) {
        FINAL_CLEANUP(target_addrinfo, sockd);
        return operes;
    }
    CLEANUP(target_addrinfo);

    operes = client_process_connection(sockd, options);

    WRITE_FORMAT("work end for target addr: %s:%s", options->hostname, options->port);

    FINAL_CLEANUP(sockd);
    return operes;
}


error_code connect_to(addrinfo *target_addrinfo, SOCKET *sockd) {
    addrinfo *p;
    for (p = target_addrinfo; p != NULL; p = p->ai_next) {
        if ((*sockd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            WSA_ERR("socket");
            continue;
        }

        if (connect(*sockd, p->ai_addr, p->ai_addrlen) == -1) {
            WSA_ERR("connect");
            CLEANUP(*sockd);
            continue;
        }
        break;
    }

    if (p == NULL) {
        WRITE("socket connect fail");
        return Sockerr;
    }

    char addr_str[INET6_ADDRSTRLEN];
    if (tostr_addr((sockaddr_storage *) p->ai_addr, addr_str) == Noerr)
        WRITE_FORMAT("connecting to %s", addr_str);
    return Noerr;

}

error_code client_process_connection(SOCKET sockd, const netwopts *options) {
    error_code operes;
    switch (options->type) {
        case Client_filereq:
            operes = reqfile(sockd, options);
            break;
        default:
            operes = Opterr;
            break;
    }
    return operes;
}

error_code reqfile(SOCKET sockd, const netwopts *options) {
    error_code operes;
    packet_t packet = {
            .state_code = Noerr,
            .data = (char *) options->input_param,
            .data_s = strlen(options->input_param) + 1
    };
    LOG_FORMAT("request file %s", options->input_param);

    if ((operes = send_packet(sockd, &packet)) != Noerr) {
        ERR("send file name %u", operes);
        return operes;
    }

    if ((operes = rcv_packet(sockd, &packet, FALSE)) != Noerr) {
        CLEANUP(&packet);
        return operes;
    }

    if (packet.state_code != Noerr) {
        WRITE_FORMAT("Error response from server %i", packet.state_code);
    } else {
        LOG_FORMAT("recived data size: %llu", packet.data_s);
        operes = write_file(options->output_param, packet.data, packet.data_s);
    }

    CLEANUP(&packet);
    return operes;
}
