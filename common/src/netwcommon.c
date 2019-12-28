#include "../netwcommon.h"

#define CUST_MAKEWORD(a, b) ((WORD)(((unsigned)(a))|(((unsigned)(b))<<8u)))

error_code wsa_start();

error_code getaddr_for(const char *target_addr, const char *port, addrinfo *hints, addrinfo **target_addrinfo);

error_code send_data(SOCKET sockd, char *data, size_t data_leng);

error_code wait_read(SOCKET sockd, char stop_key);

error_code rcv_data(SOCKET sockd, char **data, size_t *data_size);


error_code netwinit(const netwopts *options, addrinfo *hints, addrinfo **target_addrinfo) {
    error_code operes;
    if (options == NULL) {
        PRINT("inalid options");
        return Opterr;
    }

    if ((operes = wsa_start()) != Noerr) {
        return operes;
    }

    operes = getaddr_for(options->hostname, options->port, hints, target_addrinfo);
    LOG("netwinit ended");
    return operes;
}

error_code wsa_start() {
    WSAData wsaData;
    int err = 0;
    if ((err = WSAStartup(CUST_MAKEWORD(2, 2), &wsaData)) != NO_ERROR) {
        ERR("WSAStartup %d", err);
        return WSAStarterr;
    }
    LOG("WSA has been started");
    return Noerr;
}

error_code getaddr_for(const char *target_addr, const char *port, addrinfo *hints, addrinfo **target_addrinfo) {
    if (getaddrinfo(target_addr, port, hints, target_addrinfo) != 0) {
        WSA_ERR("getaddrinfo");
        return Addrerr;
    }
    LOG_FORMAT("got addr for %s:%s", target_addr == NULL ? "" : target_addr, port);
    return Noerr;
}

error_code accept_connect_stoppable(SOCKET sockd, SOCKET *incom_sockd, char stop_key) {
    PRINT_FORMAT("Press %c for stop", stop_key);
    error_code operes = wait_read(sockd, (char) tolower(stop_key));
    if (operes != Noerr) {
        return operes;
    }
    sockaddr_storage incom_addr;
    int addr_length = sizeof(sockaddr_storage);

    if ((*incom_sockd = accept(sockd, (struct sockaddr *) &incom_addr, &addr_length)) == INVALID_SOCKET) {
        WSA_ERR("accept");
        return Accepterr;
    }

    char addr_str[INET6_ADDRSTRLEN];
    if (tostr_addr(&incom_addr, addr_str) == Noerr)
        WRITE_FORMAT("connection from %s", addr_str);
    return Noerr;
}

error_code send_packet(SOCKET sockd, packet_t *packet) {
    error_code operes;
    char *data = NULL;
    size_t data_s = packet->data_s + PACKET_HEADERS_SIZE; //char for state-code + char for \0

    if ((operes = re_memalloc(&data, data_s)) != Noerr) {
        return operes;
    }

    data[0] = packet->state_code;
    memcpy(data + 1, packet->data, packet->data_s);
    data[data_s - 1] = L'\0';

    operes = send_data(sockd, data, data_s);
    CLEANUP(data);
    return operes;
}

error_code send_data(SOCKET sockd, char *data, size_t data_leng) {
    LOG_FORMAT("send data, size: %llu to socket %llx", data_leng, sockd);
    int sent = 0;
    if ((sent = send(sockd, data, data_leng, 0)) == SOCKET_ERROR) {
        WSA_ERR("send");
        return Senderr;
    }
    LOG_FORMAT("data has been sent, res-size: %i to socket %llx", sent, sockd);
    return Noerr;
}

error_code rcv_packet(SOCKET sockd, packet_t *packet, BOOL add_terminator) {
    error_code operes;
    size_t packet_s;
    char *data = NULL;
    *packet = (struct packet_t) {
            .data_s = 0,
            .data = NULL,
            .state_code = Noerr
    };

    if ((operes = rcv_data(sockd, &data, &packet_s)) != Noerr) {
        CLEANUP(data);
        return operes;
    }

    if (packet_s <= 0) {
        ERR("packet size %llu", packet_s);
        operes = Packerr;
    } else {
        packet->state_code = data[0];
        packet->data = data + 1;
        packet->data_s = add_terminator
                         ? packet_s - 1
                         : packet_s - PACKET_HEADERS_SIZE;
    }

    return operes;
}

inline error_code rcv_packet_stoppable(SOCKET sockd, packet_t *packet, BOOL add_terminator, char stop_key) {
    PRINT_FORMAT("Press %c for stop", stop_key);
    error_code operes = wait_read(sockd, (char) tolower(stop_key));
    if (operes == Cancerr) {
        return operes;
    }

    return operes == Noerr
           ? rcv_packet(sockd, packet, add_terminator)
           : operes;
}

error_code wait_read(SOCKET sockd, char stop_key) {
    fd_set master;
    fd_set read_fds;
    HANDLE input;
    INPUT_RECORD key;
    INPUT_RECORD keys[8];
    u_long records;

    TIMEVAL timeout = {.tv_sec = 0, .tv_usec = 500000};

    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(sockd, &master);

    input = GetStdHandle(STD_INPUT_HANDLE);

    //ignored in windows
    int fdmax = sockd;
    //cause set can be modif

    while (1) {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, &timeout) == SOCKET_ERROR) {
            WSA_ERR("select");
            return Selerr;
        }

        if (FD_ISSET(sockd, &read_fds)) {
            return Noerr;
        }

        ReadConsoleInput(input, keys, sizeof(keys) / sizeof(keys[0]), &records);
        for (int index = 0; index < records; index++) {
            key = keys[index];
            if (key.EventType == KEY_EVENT &&
                key.Event.KeyEvent.bKeyDown) {
                if (tolower(key.Event.KeyEvent.uChar.AsciiChar) == stop_key) {
                    PRINT("stopping");
                    return Cancerr;
                } else {
                    PRINT_FORMAT("Press %c for stop", stop_key);
                }
            }
        }
    }
}

error_code rcv_data(SOCKET sockd, char **data, size_t *data_size) {
    char buf[4096] = {0};
    *data_size = 0;
    int recv_leng;
    error_code operes;
    do {
        LOG_FORMAT("recv starting for socket %llux", sockd);
        recv_leng = recv(sockd, buf, sizeof(buf) / sizeof(buf[0]), 0);
        if (recv_leng < 0) {
            WSA_ERR("recv");
            return Recverr;
        }
        if (recv_leng > 0) {

            if ((operes = re_memalloc(data, *data_size + recv_leng)) != Noerr) {
                if (*data_size != 0)
                    CLEANUP(*data);
                *data_size = 0;
                return operes;
            }
            memcpy(*data + *data_size, buf, recv_leng * sizeof(buf[0]));
            *data_size += recv_leng;
        }

    } while (recv_leng > sizeof(buf) / sizeof(buf[0]));

    LOG_FORMAT("recv ending, res-size: %llu for socket %llux", *data_size, sockd);

    return Noerr;
}

error_code tostr_addr(const sockaddr_storage *addr, char *addr_str) {
    unsigned long name_leng = INET6_ADDRSTRLEN;
    if (WSAAddressToString((struct sockaddr *) addr, sizeof(sockaddr_storage), NULL, addr_str, &name_leng) != 0) {
        WSA_ERR("WSAAddressToStringA");
        return Addrerr;
    }
    return Noerr;
}