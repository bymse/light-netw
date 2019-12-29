#include "netwicmp.h"

error_code icmp_init(const netwopts *options, addrinfo **addr, SOCKET *sockd);


error_code ping_with4packets(SOCKET sockd, const sockaddr_storage *target);

error_code trace_route(SOCKET sockd, const sockaddr_storage *target, size_t max_hops);


error_code send_icmp(SOCKET sockd, const sockaddr_storage *target, u_short identifier, u_short sequence);

error_code rcv_icmp(SOCKET sockd, sockaddr_storage *source, u_char buf[static IP_ICMP_BUF_SIZE], size_t *size);


error_code sendto_raw(SOCKET sockd, const sockaddr_storage *target, const u_char *data, size_t *size);

error_code rcvfrom_raw(SOCKET sockd, sockaddr_storage *source,
                       u_char data[static BUF_SIZE], size_t *size);

void print_table_row_(size_t sent_no,
                      const sockaddr_storage *to,
                      const sockaddr_storage *from,
                      size_t rcvd_no,
                      clock_t time_span,
                      const char *state_str);

static inline int gethops(const char *input) {
    int res = strtol(input, NULL, 0);
    return 0 < res && res <= 255 ? res : 30;
}

static inline const char *statestr_for(int code) {
    switch (code) {
        case Noerr:
            return "OK";
        case DestinationUnreach:
            return "Destination Unreachable";
        case Timerr:
            return "Timeout";
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
    const u_int timeout = 8000u;
    if (setsockopt(*sockd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(u_int)) == INVALID_SOCKET) {
        WSA_ERR("setsockopt");
        FINAL_CLEANUP(*addr, *sockd);
        return Sockerr;
    }

    return Noerr;
}

/*
No. |To              |From            |No. |Time ms|State
1   |123.456.789.000 |987.765.432.100 |1   |10000  |Destination Unreachable   
 */
//                          No   To   From No  Tim State
#define TABLE_COLUMN_FORMAT "%4s|%16s|%16s|%4s|%7s|%s\r\n"
#define PRINT_TABLE_HEADER() RAW_PRINT("\r\n"TABLE_COLUMN_FORMAT, "No.", "Sent to", "Recived from", "No.", "Time ms", "State")

void print_table_row_(size_t sent_no,
                      const sockaddr_storage *to,
                      const sockaddr_storage *from,
                      size_t rcvd_no,
                      clock_t time_span,
                      const char *state_str) {
    char toaddr_str[INET6_ADDRSTRLEN] = {0};
    char fromaddr_str[INET6_ADDRSTRLEN] = {0};

    if (to != NULL) {
        tostr_addr(to, toaddr_str);
    }

    if (from != NULL) {
        tostr_addr(from, fromaddr_str);
    }

    char sent_no_str[3 + 1] = {0};
    char rcvd_no_str[3 + 1] = {0};

    if (sent_no >= 0) {
        sprintf(sent_no_str, "%zu", sent_no);
    }

    if (rcvd_no >= 0) {
        sprintf(rcvd_no_str, "%zu", rcvd_no);
    }

    char time_span_str[4 + 1] = {0};

    if (time_span >= 0) {
        sprintf(time_span_str, "%li", time_span);
    }

    RAW_PRINT(TABLE_COLUMN_FORMAT, sent_no_str, toaddr_str, fromaddr_str, rcvd_no_str, time_span_str, state_str);
}


error_code ping_with4packets(SOCKET sockd, const sockaddr_storage *target) {
    const size_t packets_count = 4;
    struct {
        clock_t sent_at;
        clock_t recived_at;
        icmp_header recived_header;
        sockaddr_storage source;
    } recived[packets_count];

    error_code operes = Noerr;
    sockaddr_storage source;

    size_t rcvsize = IP_ICMP_BUF_SIZE;
    u_char rcvbuf[IP_ICMP_BUF_SIZE];

    clock_t time_fid = time(NULL);
    u_short identifier = (time_fid >> 16) ^time_fid;

    size_t passed_seq_no;
    for (passed_seq_no = 1; passed_seq_no <= packets_count; ++passed_seq_no) {

        PRINT_FORMAT("Working with packet %llu", passed_seq_no);
        clock_t start = clock();
        if ((operes = send_icmp(sockd, target, identifier, passed_seq_no)) != Noerr) {
            break;
        }

        if ((operes = rcv_icmp(sockd, &source, rcvbuf, &rcvsize)) != Noerr && operes != Timerr) {
            break;
        }
        clock_t end = clock();

        icmp_header *header = (icmp_header *) (rcvbuf + IP_HEADER_SIZE);

        recived[passed_seq_no - 1].sent_at = start;
        recived[passed_seq_no - 1].recived_at = operes == Timerr ? -1 : end;
        recived[passed_seq_no - 1].recived_header = *header;
        recived[passed_seq_no - 1].source = source;
    }

    PRINT_TABLE_HEADER();
    for (size_t seq_no = 0; seq_no < passed_seq_no - 1; ++seq_no) {
        __auto_type cur = recived[seq_no];
        BOOL was_timeout = cur.recived_at < 0;

        clock_t time_span = was_timeout
                            ? -1
                            : cur.recived_at - cur.sent_at;

        const char *state = statestr_for(was_timeout ? Timerr : cur.recived_header.type);

        print_table_row_(seq_no + 1,
                         target,
                         was_timeout ? NULL : &cur.source,
                         was_timeout ? -1 : cur.recived_header.sequence_no,
                         time_span,
                         state);
    }

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

        if ((operes = send_icmp(sockd, target, 123, 1)) != Noerr) {
            return operes;
        }

        if ((operes = rcv_icmp(sockd, &source, buf, &size)) != Noerr) {
            return operes;
        }
    }
    return Noerr;
}

error_code send_icmp(SOCKET sockd, const sockaddr_storage *target, u_short identifier, u_short sequence) {

    size_t size = sizeof(icmp_header) + ICMP_PAYLOAD;
    u_char data[size];

    size_t payload_start = sizeof(icmp_header);

    memcpy(data, &(icmp_header) {
            .type = 8,
            .code = 0,
            .identifier = identifier,
            .checksum = 0,
            .sequence_no = sequence
    }, sizeof(icmp_header));

    for (size_t i = payload_start; i < size; ++i) {
        data[i] = i;
    }

    ((icmp_header *) data)->checksum = Internet_checksum((u_short *) data, size);

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




