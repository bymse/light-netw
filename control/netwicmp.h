#ifndef LIGHT_NETW_NETWICMP_H
#define LIGHT_NETW_NETWICMP_H

#include <time.h>
#include "../common/netwbase.h"
#include "../common/netwcommon.h"
#include "../common/netwfcs.h"

#define ICMP_PREFIX "ICMP"


#define IP_HEADER_SIZE 20u
#define ICMP_PAYLOAD 100
#define IP_ICMP_BUF_SIZE IP_HEADER_SIZE + ICMP_PAYLOAD * 4 + sizeof(icmp_header)

#define RCV_TO_SEC 4
static TIMEVAL rcvto_val = {.tv_sec = RCV_TO_SEC, .tv_usec = 1000};

typedef enum icmp_types {
    EchoReply = 0,
    DestinationUnreach = 3,
    SourceQuench = 4,
    RedirectMessage = 5,
    EchoRequest = 8
} icmp_type;


#define ICMP_HINTS(routing) &((addrinfo) {  \
            .ai_family = routing,           \
            .ai_socktype = SOCK_RAW,        \
            .ai_protocol = IPPROTO_ICMP,    \
            .ai_flags = AI_CANONNAME,       \
            .ai_addr = NULL,                \
            .ai_canonname = NULL,           \
            .ai_next = NULL,                \
            .ai_addrlen = 0,                \
})

typedef struct icmp_header {
    u_char type;
    u_char code;
    u_short checksum;
    u_short identifier;
    u_short sequence_no;
} __attribute__((packed)) icmp_header;

enum packet_state {
    Sent = 0,
    Recived = 1,
    Timeout = 2,
    Error = 3,
};

typedef struct icmp_transmission_info {
    icmp_header header;
    clock_t time;
    sockaddr_storage addr;
    enum packet_state state : 2;
} icmp_transmis;

error_code ping(const netwopts *options);

error_code tracert(const netwopts *options);

#endif //LIGHT_NETW_NETWICMP_H
