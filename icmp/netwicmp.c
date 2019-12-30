#include "netwicmp.h"
#include "netwicmp_table.c"

error_code icmp_init(const netwopts *options, addrinfo **addr, SOCKET *sockd);


error_code ping_with4packets(SOCKET sockd, const sockaddr_storage *target);

error_code trace_route(SOCKET sockd, const sockaddr_storage *target, size_t max_hops);

error_code send_icmp(SOCKET sockd, const sockaddr_storage *target, u_short identifier, u_short sequence,
                     icmp_header *sent_header);

error_code rcv_icmp(SOCKET sockd, sockaddr_storage *source, u_char buf[static IP_ICMP_BUF_SIZE], size_t *size);


error_code sendto_raw(SOCKET sockd, const sockaddr_storage *target, const u_char *data, size_t *size);

error_code rcvfrom_raw(SOCKET sockd, sockaddr_storage *source,
                       u_char data[static BUF_SIZE], size_t *size);


static inline int gethops(const char *input) {
    int res = strtol(input, NULL, 0);
    return 0 < res && res <= 255 ? res : 30;
}

#define RCVD_AFTER_TO 1000

static inline const char *statestr_for(int code) {
    switch (code) {
        case Noerr:
            return "OK";
        case DestinationUnreach:
            return "Destination Unreachable";
        case Timerr:
            return "Timeout, over "TOSTR(RCV_TO_SEC)" s";
        case RCVD_AFTER_TO:
            return "Recived after timeout";
        default:
            return "Uknown state";
    }
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

    //prevent permament block
    const u_int timeout = RCV_TO_SEC * 2 * 1000;
    if (setsockopt(*sockd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(u_int)) == INVALID_SOCKET) {
        WSA_ERR("setsockopt");
        FINAL_CLEANUP(*addr, *sockd);
        return Sockerr;
    }

    return Noerr;
}

enum packet_state {
    Sent = 0,
    Recived = 1,
};

typedef struct icmp_transmission_info {
    icmp_header header;
    clock_t time;
    sockaddr_storage addr;
    enum packet_state state : 1;
} icmp_transmis;

void analyze(size_t packets_c, icmp_transmis packets[packets_c]) {
    int sent_no = -1;
    int rcvd_no = -1;
    const sockaddr_storage *to = NULL;
    const sockaddr_storage *from = NULL;
    clock_t time_span = -1;

    int state_code = 0;

    PRINT_TABLE_HEADER();
    for (int index = 0; index < packets_c; ++index) {
        if (packets[index].state == Sent) {
            __auto_type sent_packet = packets[index];

            sent_no = sent_packet.header.sequence_no;
            to = &sent_packet.addr;

            if (index < packets_c - 1) {
                __auto_type next_packet = packets[index + 1];


                if (next_packet.state == Recived
                    && next_packet.header.sequence_no == sent_packet.header.sequence_no) {

                    rcvd_no = next_packet.header.sequence_no;
                    from = &next_packet.addr;
                    time_span = next_packet.time - sent_packet.time;
                    state_code = next_packet.header.code;
                    index++;
                } else state_code = Timerr;
            }
        } else if (packets[index].state == Recived) {
            __auto_type rcvd_packet = packets[index];

            rcvd_no = rcvd_packet.header.sequence_no;
            from = &rcvd_packet.addr;
            state_code = rcvd_packet.header.type == 0
                         ? RCVD_AFTER_TO
                         : rcvd_packet.header.type;

            for (int back_index = index - 1; back_index >= 0; --back_index) {
                __auto_type prev_packet = packets[back_index];
                if (prev_packet.state == Sent &&
                    prev_packet.header.sequence_no == rcvd_no) {

                    time_span = rcvd_packet.time - prev_packet.time;
                    break;
                }
            }

        } else {
            WRITE("Invalid packet state");
            continue;
        }

        print_table_row(sent_no,
                        to,
                        from,
                        rcvd_no,
                        time_span,
                        statestr_for(state_code));

        sent_no = -1;
        rcvd_no = -1;
        to = NULL;
        from = NULL;
        time_span = -1;

        state_code = 0;
    }
}


error_code ping_with4packets(SOCKET sockd, const sockaddr_storage *target) {
    const int packets_to_sent_count = 4;

    size_t packets_count = 0;
    icmp_transmis packets[packets_to_sent_count * 2];

    error_code operes = Noerr;
    sockaddr_storage source;

    size_t rcvsize = IP_ICMP_BUF_SIZE;
    u_char rcvbuf[IP_ICMP_BUF_SIZE];

    clock_t time_fid = time(NULL);
    u_short identifier = (time_fid >> 16) ^time_fid;

    icmp_header sent_header;

    int packet_no;
    for (packet_no = 1; packet_no <= packets_to_sent_count; ++packet_no) {

        PRINT_FORMAT("Working with packet %i", packet_no);
        clock_t start = clock();
        if ((operes = send_icmp(sockd, target, identifier, packet_no, &sent_header)) != Noerr) {
            break;
        }

        if ((operes = rcv_icmp(sockd, &source, rcvbuf, &rcvsize)) != Noerr && operes != Timerr) {
            break;
        }
        clock_t end = clock();

        icmp_header *rcvd_header = (icmp_header *) (rcvbuf + IP_HEADER_SIZE);

        packets[packets_count].state = Sent;
        packets[packets_count].time = start;
        packets[packets_count].header = sent_header;
        packets[packets_count].addr = *target;

        packets_count++;

        if (operes != Timerr) {
            packets[packets_count].state = Recived;
            packets[packets_count].time = end;
            packets[packets_count].header = *rcvd_header;
            packets[packets_count].addr = source;
            packets_count++;
        }

    }
    
    analyze(packets_count, packets);
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

        if ((operes = send_icmp(sockd, target, 123, 1, NULL)) != Noerr) {
            return operes;
        }

        if ((operes = rcv_icmp(sockd, &source, buf, &size)) != Noerr) {
            return operes;
        }
    }
    return Noerr;
}

error_code send_icmp(SOCKET sockd, const sockaddr_storage *target, u_short identifier, u_short sequence,
                     icmp_header *sent_header) {

    size_t size = sizeof(icmp_header) + ICMP_PAYLOAD;
    u_char data[size];

    size_t payload_start = sizeof(icmp_header);
    icmp_header header_to_sent = {
            .type = 8,
            .code = 0,
            .identifier = identifier,
            .checksum = 0,
            .sequence_no = sequence
    };

    memcpy(data, &header_to_sent, sizeof(icmp_header));

    for (size_t i = payload_start; i < size; ++i) {
        data[i] = i;
    }

    ((icmp_header *) data)->checksum = Internet_checksum((u_short *) data, size);

    if (sent_header != NULL)
        *sent_header = header_to_sent;

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




