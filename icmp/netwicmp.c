#include "netwicmp.h"

error_code ping_with4packets(SOCKET sockd, const sockaddr_storage *target);

error_code send_icmp(SOCKET sockd, const sockaddr_storage *target, u_short identifier, u_short sequence);

error_code rcv_icmp(SOCKET sockd, const sockaddr_storage *target, u_char buf[static IP_ICMP_BUF_SIZE], size_t *size);


error_code sendto_raw(SOCKET sockd, const sockaddr_storage *target, const u_char *data, size_t *size);

error_code rcvfrom_raw(SOCKET sockd, const sockaddr_storage *target,
                       u_char data[static BUF_SIZE], size_t *size);


//todo union for netwoptions' independent params
error_code ping(const netwopts *options) {
    SET_PREFIX(PING_PREFIX);
    logs_init(options->logs_path);

    addrinfo *target = NULL;
    error_code operes;
    SOCKET sockd;

    if ((operes = netwinit(options, PING_HINTS(options->routing), &target)) != Noerr) {
        FINAL_CLEANUP((addrinfo *) target);
        return operes;
    }

    //todo add for ipv6 IPPROTO_ICMPV6
    if ((sockd = socket(options->routing, SOCK_RAW, IPPROTO_ICMP)) == INVALID_SOCKET) {
        WSA_ERR("socket");
        FINAL_CLEANUP((addrinfo *) target);
        return Sockerr;
    }


    operes = ping_with4packets(sockd, (const sockaddr_storage *) target->ai_addr);
    FINAL_CLEANUP(sockd, (addrinfo *) target);
    return operes;
}


error_code ping_with4packets(SOCKET sockd, const sockaddr_storage *target) {
    error_code operes;

    clock_t time_fid = time(NULL);
    u_short identifier = (time_fid >> 16) + time_fid;
    size_t size = IP_ICMP_BUF_SIZE;
    u_char buf[IP_ICMP_BUF_SIZE];

    for (int i = 0; i < 4; ++i) {
        clock_t start = clock();
        PRINT_FORMAT("PACKET %i", i + 1);
        print_addr("Sending to", target);
        if ((operes = send_icmp(sockd, target, identifier, i + 3)) != Noerr) {
            return operes;
        }

        if ((operes = rcv_icmp(sockd, target, buf, &size)) != Noerr) {
            return operes;
        }
        clock_t end = clock();

        print_addr("Recived from", target);
        icmp_header *header = (icmp_header *) (buf + IP_HEADER_SIZE);
        if (header->Identifier == identifier && header->SequenceNumber == i + 3) {
            PRINT_FORMAT("Time: %li ms", ((end - start)));
        } else {
            PRINT("Invalid packet ids");
        }

        PRINT("----------");

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

error_code rcv_icmp(SOCKET sockd, const sockaddr_storage *target, u_char buf[static IP_ICMP_BUF_SIZE], size_t *size) {
    error_code operes;
    if ((operes = rcvfrom_raw(sockd, target, buf, size)) != Noerr) {
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

error_code rcvfrom_raw(SOCKET sockd, const sockaddr_storage *target,
                       u_char data[static BUF_SIZE], size_t *size) {

    int addrsize = sizeof(sockaddr_storage);
    if ((*size = recvfrom(sockd, (char *) data, *size,
                          0,
                          (struct sockaddr *) target,
                          &addrsize)) == INVALID_SOCKET) {
        WSA_ERR("recvfrom");
        return Recverr;
    }

    return Noerr;
}

