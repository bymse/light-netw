#ifndef NETWORKS_NETWCOMMON_H
#define NETWORKS_NETWCOMMON_H

#include <ctype.h>
#include "netwbase.h"
#include "netwtypes.h"
#include "netwlogging.h"
#include "netwcleanup.h"

#define DEFAULT_PORT "3490"
#define MAX_PACKET_SIZE 1500u
#define PACKET_HEADERS_SIZE (sizeof(char) + sizeof(char))
#define _TOSTR(val) #val
#define TOSTR(val) _TOSTR(val)

error_code netwinit(const netwopts *options, addrinfo *hints, addrinfo **target_addrinfo);

error_code accept_connect_stoppable(SOCKET sockd, SOCKET *incom_sockd, char stop_key);

error_code send_packet(SOCKET sockd, packet_t *packet);

error_code rcv_packet(SOCKET sockd, packet_t *packet, BOOL add_terminator);

error_code sendto_raw(SOCKET sockd, const sockaddr_storage *target, const u_char *data, size_t *size);

error_code rcvfrom_raw(SOCKET sockd, sockaddr_storage *source,
                       u_char data[static MAX_PACKET_SIZE], size_t *size);

error_code rcv_packet_stoppable(SOCKET sockd, packet_t *packet, BOOL add_terminator, char stop_key);

error_code tostr_addr(const sockaddr_storage *addr, char addr_str[static INET6_ADDRSTRLEN]);

error_code waitrcv_charcnl(SOCKET sockd, char stop_key);

error_code waitrcv_timeout(SOCKET sockd, TIMEVAL *timeout);

static inline error_code re_memalloc(char **ptr, size_t size) {
    if ((*ptr = realloc(*ptr, size)) == NULL) {
        ERR("memory allocation %i", errno);
        return Memerr;
    }

    return Noerr;
}

#endif //NETWORKS_NETWCOMMON_H