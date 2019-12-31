#include "netwicmp.h"
#include "netwicmp_table.h"

error_code icmp_init(const netwopts *options, addrinfo **addr, SOCKET *sockd);

error_code trace_route(SOCKET sockd, const sockaddr_storage *target, size_t max_hops);

error_code ping_with_count_packets(SOCKET sockd, const sockaddr_storage *target, size_t packets_to_sent_count);


error_code send_icmp_transmis(SOCKET sockd, icmp_transmis *packet);

error_code rcv_icmp_transmis(SOCKET sockd, icmp_transmis *packet);


error_code send_icmp(SOCKET sockd, const sockaddr_storage *target, icmp_header *header);

error_code rcv_icmp(SOCKET sockd, sockaddr_storage *source, u_char buf[static IP_ICMP_BUF_SIZE], size_t *size);


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

    PRINT_FORMAT("Let's ping %s", options->hostname);
    operes = ping_with_count_packets(sockd, (const sockaddr_storage *) target->ai_addr, 4);
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

    //prevent permament block
    const u_int timeout = RCV_TO_SEC * 2 * 1000;
    if (setsockopt(*sockd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(u_int)) == INVALID_SOCKET) {
        WSA_ERR("setsockopt");
        FINAL_CLEANUP(*addr, *sockd);
        return Sockerr;
    }

    return Noerr;
}


error_code ping_with_count_packets(SOCKET sockd, const sockaddr_storage *target, size_t packets_to_sent_count) {
    error_code operes = Noerr;

    packets_to_sent_count = packets_to_sent_count > 10
                            ? 10
                            : packets_to_sent_count;

    icmp_transmis packets[packets_to_sent_count * 2];
    size_t packets_count = 0;
    size_t to_count = 0;

    clock_t time_fid = time(NULL);
    u_short identifier = (time_fid >> 16) ^time_fid;

    int packet_no;
    for (packet_no = 1; packet_no <= packets_to_sent_count; ++packet_no) {

        PRINT_FORMAT("Working with packet %i", packet_no);

        packets[packets_count].header = (icmp_header) {
                .type = EchoRequest,
                .sequence_no = packet_no,
                .identifier = identifier
        };
        packets[packets_count].addr = *target;

        if ((operes = send_icmp_transmis(sockd, &packets[packets_count])) != Noerr) {
            break;
        }

        packets_count++;

        if ((operes = rcv_icmp_transmis(sockd, &packets[packets_count])) != Noerr && operes != Timerr) {
            break;
        }

        if (operes != Timerr)
            packets_count++;
        else
            to_count++;

    }

    if (to_count > 0 && (operes == Noerr || operes == Timerr)) {
        WRITE_FORMAT("Trying to receive timeout packets, count: %zu", to_count);
        while (to_count-- > 0) {
            if ((operes = rcv_icmp_transmis(sockd, &packets[packets_count])) != Noerr) {
                break;
            }
            packets_count++;
        }
        if (operes == Timerr) {
            operes = Noerr;
        }
    }

    print_ping_table(packets_count, packets);
    return operes;
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
    }
    return Noerr;
}

error_code send_icmp_transmis(SOCKET sockd, icmp_transmis *packet) {
    error_code operes;

    packet->time = clock();
    if ((operes = send_icmp(sockd, &(packet->addr), &(packet->header))) != Noerr) {
        packet->state = Error;
        return operes;
    }

    packet->state = Sent;
    return operes;
}

error_code rcv_icmp_transmis(SOCKET sockd, icmp_transmis *packet) {
    error_code operes;
    size_t buf_length = IP_ICMP_BUF_SIZE;
    u_char buf[IP_ICMP_BUF_SIZE];

    if ((operes = rcv_icmp(sockd, &(packet->addr), buf, &buf_length)) != Noerr) {
        packet->state = operes == Timerr ? Timeout : Error;
        return operes;
    }

    packet->time = clock();
    packet->state = Recived;
    packet->header = *((icmp_header *) (buf + IP_HEADER_SIZE));

    return operes;
}

error_code send_icmp(SOCKET sockd, const sockaddr_storage *target, icmp_header *header) {

    size_t size = sizeof(icmp_header) + ICMP_PAYLOAD;
    u_char data[size];

    size_t payload_start = sizeof(icmp_header);

    memcpy(data, header, sizeof(icmp_header));

    for (size_t i = payload_start; i < size; ++i) {
        data[i] = i;
    }

    ((icmp_header *) data)->checksum = Internet_checksum((u_short *) data, size);

    return sendto_raw(sockd, target, data, &size);
}

error_code rcv_icmp(SOCKET sockd, sockaddr_storage *source, u_char buf[static IP_ICMP_BUF_SIZE], size_t *size) {
    error_code operes;

    if ((operes = waitrcv_timeout(sockd, &rcvto_val)) != Noerr)
        return operes;

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




