#include "../netwcommon.h"

#define CUST_MAKEWORD(a, b) ((WORD)(((unsigned)(a))|(((unsigned)(b))<<8u)))

error_code wsa_start();

error_code getaddr_for(const char *target_addr, const char *port, addrinfo *hints, addrinfo **target_addrinfo);

error_code send_data(SOCKET sockd, char *data, size_t data_leng);

error_code rcv_data(SOCKET sockd, char **data, size_t *data_size);


error_code init(const netwopts *options, addrinfo *hints, addrinfo **target_addrinfo) {
    error_code operes;
    if (options == NULL) {
        PRINT("inalid options\n");
        return Opterr;
    }

    if ((operes = wsa_start()) != Noerr) {
        return operes;
    }

    operes = getaddr_for(options->hostname, options->port, hints, target_addrinfo);
    return operes;
}

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


error_code send_packet(SOCKET sockd, packet *packet) {
    error_code operes;
    char *data = NULL;
    size_t data_s = packet->data_s + PACKET_HEADERS_SIZE; //char for state-code + char for \0

    if ((operes = re_memalloc(&data, data_s)) != Noerr) {
        return operes;
    }

    data[0] = packet->state_code;
    memcpy(data + 1, packet->data, packet->data_s);
    data[data_s - 1] = '\0';

    operes = send_data(sockd, data, data_s);
    free(data);
    return operes;
}

error_code send_data(SOCKET sockd, char *data, size_t data_leng) {
    if (send(sockd, data, data_leng, 0) == SOCKET_ERROR) {
        PRINT_WSA_ERR("send");
        return Senderr;
    }

    return Noerr;
}

error_code rcv_packet(SOCKET sockd, packet *packet, BOOL add_terminator) {
    error_code operes;
    size_t packet_s;
    char *data = NULL;
    *packet = (struct packet){
        .data_s = 0,
        .data = NULL,
        .state_code = Noerr
    };

    if ((operes = rcv_data(sockd, &data, &packet_s)) != Noerr) {
        free(data);
        return operes;
    }

    if (packet_s <= 0) {
        PRINT_ERROR("packet size %u", packet_s);
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

    } while (recv_leng > sizeof(buf) / sizeof(buf[0]));

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

void freepacket(packet *packet) {
    free(packet->data - 1);
}



