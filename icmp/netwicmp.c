#include "netwicmp.h"

error_code send_icmp(SOCKET sockd, const sockaddr_storage *target);

error_code rcv_icmp(SOCKET sockd, sockaddr_storage *target);


//todo union for netwoptions' independent params
error_code ping(const netwopts *options) {
    SET_PREFIX(PING_PREFIX);
    logs_init(options->logs_path);

    addrinfo *target = NULL;
    error_code operes;
    SOCKET sockd;

    if ((operes = netwinit(options, PING_HINTS(options->routing), &target)) != Noerr) {
        FINAL_CLEANUP(target);
        return operes;
    }

    //todo add for ipv6 IPPROTO_ICMPV6
    if ((sockd = socket(options->routing, SOCK_RAW, IPPROTO_ICMP)) == INVALID_SOCKET) {
        FINAL_CLEANUP(target);
        return Sockerr;
    }

    if ((operes = send_icmp(sockd, (const sockaddr_storage *) target->ai_addr)) != Noerr) {
        FINAL_CLEANUP(sockd, target);
        return operes;
    }

    operes = rcv_icmp(sockd, (sockaddr_storage *) target->ai_addr);

    FINAL_CLEANUP(sockd, target);
    return operes;
}

typedef struct icmp_header {
    u_char Type;
    u_char Code;
    u_short Checksum;
    u_short Identifier;
    u_short SequenceNumber;
} __attribute__((packed)) icmp_header;

#define PAYLOAD 20


error_code send_icmp(SOCKET sockd, const sockaddr_storage *target) {
    char *data = NULL;
    char payload[PAYLOAD] = {12, 12, 12, 12, 12, 12, 12};
    size_t size = sizeof(icmp_header) + PAYLOAD;
    re_memalloc(&data, size);
    memcpy(data, &(icmp_header) {
            .Type = 8,
            .Code = 0,
            .Identifier = 347,
            .Checksum = 0,
            .SequenceNumber = 1
    }, sizeof(icmp_header));
    memcpy(data + sizeof(icmp_header), payload, PAYLOAD);

    ((icmp_header *) data)->Checksum = Internet_checksum((u_short *) data, size);

    if ((size = sendto(sockd, (const char *) data, size,
                       0, (const struct sockaddr *) target,
                       sizeof(sockaddr_storage))) == INVALID_SOCKET) {
        WSA_ERR("sendto");
        CLEANUP(data);
        return Senderr;
    }

    WRITE_FORMAT("%llu bytes sent", size);
    CLEANUP(data);
    return Noerr;
}

error_code rcv_icmp(SOCKET sockd, sockaddr_storage *target) {
    char data[80];
    size_t size = sizeof(data) / sizeof data[0];
    int addrsize = sizeof(sockaddr_storage);
    if ((size = recvfrom(sockd, data, size,
                         0, (struct sockaddr *) target, &addrsize)) == INVALID_SOCKET) {
        WSA_ERR("ecvfrom");
        return Recverr;
    }

    WRITE_FORMAT("size: %llu, data: %s", size, data);
    print_addr("response from", target);
    return Noerr;
}
