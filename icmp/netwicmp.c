#include "netwicmp.h"

error_code icmp_init(const netwopts *options, addrinfo **addr, SOCKET *sockd);


error_code ping_with4packets(SOCKET sockd, const sockaddr_storage *target);

error_code trace_route(SOCKET sockd, const sockaddr_storage *target, size_t max_hops);


error_code send_icmp(SOCKET sockd, const sockaddr_storage *target, u_short identifier, u_short sequence);

error_code rcv_icmp(SOCKET sockd, sockaddr_storage *source, u_char buf[static IP_ICMP_BUF_SIZE], size_t *size);


error_code sendto_raw(SOCKET sockd, const sockaddr_storage *target, const u_char *data, size_t *size);

error_code rcvfrom_raw(SOCKET sockd, sockaddr_storage *source,
                       u_char data[static BUF_SIZE], size_t *size);

static inline int gethops(const char *input) {
    int res = strtol(input, NULL, 0);
    return 0 < res && res <= 255 ? res : 30;
}

//todo union for netwoptions' independent params
//todo add for ipv6 IPPROTO_ICMPV6
error_code ping(const netwopts *options) {
    SET_PREFIX(ICMP_PREFIX);
    logs_init(options->logs_path);

    addrinfo *target;
    error_code operes;
    SOCKET sockd;

    //cleanup in
    if ((operes = icmp_init(options, &target, &sockd))) {
        return operes;
    }

    operes = ping_with4packets(sockd, (const sockaddr_storage *) target->ai_addr);
    FINAL_CLEANUP(sockd, target);
    return operes;
}

error_code tracert(const netwopts *options) {
    SET_PREFIX(ICMP_PREFIX);
    logs_init(options->logs_path);

    addrinfo *target = NULL;
    error_code operes;
    SOCKET sockd = INVALID_SOCKET;

    //cleanup in
    if ((operes = icmp_init(options, &target, &sockd))) {
        return operes;
    }

    operes = trace_route(sockd, (const sockaddr_storage *) target->ai_addr, gethops(options->input_param));
    FINAL_CLEANUP(sockd, target);
    return operes;
}

error_code icmp_init(const netwopts *options, addrinfo **addr, SOCKET *sockd) {
    error_code operes;
    if ((operes = netwinit(options, ICMP_HINTS(options->routing), addr)) != Noerr) {
        FINAL_CLEANUP(*addr);
        return operes;
    }

    if ((*sockd = socket(options->routing, SOCK_RAW, IPPROTO_ICMP)) == INVALID_SOCKET) {
        WSA_ERR("socket");
        FINAL_CLEANUP(*addr);
        return Sockerr;
    }

    return Noerr;
}

error_code ping_with4packets(SOCKET sockd, const sockaddr_storage *target) {
    error_code operes;

    clock_t time_fid = time(NULL);
    u_short identifier = (time_fid >> 16) + time_fid;

    size_t rcvsize = IP_ICMP_BUF_SIZE;
    u_char rcvbuf[IP_ICMP_BUF_SIZE];

    sockaddr_storage source;

    for (int i = 0; i < 4; ++i) {

        PRINT_FORMAT("PACKET %i", i + 1);
        print_addr("Sending to", target);

        clock_t start = clock();
        if ((operes = send_icmp(sockd, target, identifier, i + 3)) != Noerr) {
            return operes;
        }

        if ((operes = rcv_icmp(sockd, &source, rcvbuf, &rcvsize)) != Noerr) {
            return operes;
        }
        clock_t end = clock();

        print_addr("Recived from", &source);

        icmp_header *header = (icmp_header *) (rcvbuf + IP_HEADER_SIZE);
        if (header->Type == DestinationUnreach) {
            PRINT("Destination Unreachable...");
        } else if (header->Identifier != identifier || header->SequenceNumber != i + 3) {
            PRINT("Invalid packet ids");
        }

        PRINT_FORMAT("Time: %li ms", ((end - start)));
        PRINT("----------");
    }

    return Noerr;
}

error_code trace_route(SOCKET sockd, const sockaddr_storage *target, size_t max_hops) {
    error_code operes;
    size_t size = IP_ICMP_BUF_SIZE;
    u_char buf[IP_ICMP_BUF_SIZE];
    sockaddr_storage source;
    for (size_t hop = 0; hop < max_hops; ++hop) {
        u_int ttl = hop + 1;
        if (setsockopt(sockd, IPPROTO_IP, IP_TTL, (const char *) &ttl, sizeof(u_int)) == INVALID_SOCKET) {
            WSA_ERR("setsockopt");
            return Sockerr;
        }

        if ((operes = send_icmp(sockd, target, 123, 1)) != Noerr) {
            return operes;
        }

        if ((operes = rcv_icmp(sockd, &source, buf, &size)) != Noerr) {
            return operes;
        }

        print_addr("Recived from", &source);
    }
    return Noerr;
}

error_code send_icmp(SOCKET sockd, const sockaddr_storage *target, u_short identifier, u_short sequence) {

    size_t size = sizeof(icmp_header) + ICMP_PAYLOAD;
    u_char data[size];

    size_t payload_start = sizeof(icmp_header);

    memcpy(data, &(icmp_header) {
            .Type = 8,
            .Code = 0,
            .Identifier = identifier,
            .Checksum = 0,
            .SequenceNumber = sequence
    }, sizeof(icmp_header));

    for (size_t i = payload_start; i < size; ++i) {
        data[i] = i;
    }

    ((icmp_header *) data)->Checksum = Internet_checksum((u_short *) data, size);

    return sendto_raw(sockd, target, data, &size);
}

error_code rcv_icmp(SOCKET sockd, sockaddr_storage *source, u_char buf[static IP_ICMP_BUF_SIZE], size_t *size) {
    error_code operes;
    if ((operes = rcvfrom_raw(sockd, source, buf, size)) != Noerr) {
        return operes;
    }

    if (*size <= IP_HEADER_SIZE + sizeof(icmp_header)) {
        WRITE_FORMAT("Invalid icmp packet recived, IP packet size: %llu", *size);
        return Packerr;
    }

    if (Internet_checksum((const u_short *) (buf + IP_HEADER_SIZE),
                          *size - IP_HEADER_SIZE) != 0) {
        WRITE("Invalid icmp-packet checksum");
    }
    return Noerr;
}


error_code sendto_raw(SOCKET sockd, const sockaddr_storage *target, const u_char *data, size_t *size) {

    if ((*size = sendto(sockd, (const char *) data, *size,
                        0, (const struct sockaddr *) target,
                        sizeof(sockaddr_storage))) == INVALID_SOCKET) {
        WSA_ERR("sendto");
        return Senderr;
    }

    return Noerr;
}

error_code rcvfrom_raw(SOCKET sockd, sockaddr_storage *source,
                       u_char data[static BUF_SIZE], size_t *size) {

    int addrsize = sizeof(sockaddr_storage);
    if ((*size = recvfrom(sockd, (char *) data, *size,
                          0,
                          (struct sockaddr *) source,
                          &addrsize)) == INVALID_SOCKET) {
        WSA_ERR("recvfrom");
        return Recverr;
    }

    return Noerr;
}




