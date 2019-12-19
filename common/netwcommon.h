#ifndef NETWORKS_NETWCOMMON_H
#define NETWORKS_NETWCOMMON_H

#include <ctype.h>
#include "netwbase.h"
#include "netwtypes.h"
#include "netwlogging.h"
#include "netwcleanup.h"

#define DEFAULT_PORT "3490"
#define MAX_PACKET_S 1300
#define PACKET_HEADERS_SIZE (sizeof(char) + sizeof(char))


error_code netwinit(const netwopts *options, addrinfo *hints, addrinfo **target_addrinfo);

error_code accept_connect_async(SOCKET sockd, SOCKET *incom_sockd, char stop_key);

error_code send_packet(SOCKET sockd, packet_t *packet);

error_code rcv_packet(SOCKET sockd, packet_t *packet, BOOL add_terminator);

error_code rcv_packet_async(SOCKET sockd, packet_t *packet, BOOL add_terminator, char stop_key);

void print_addr(const char *phrase, const sockaddr_storage *addr);

static inline error_code re_memalloc(char **ptr, size_t size) {
    if ((*ptr = realloc(*ptr, size)) == NULL) {
        ERR("memory allocation %i", errno);
        return Memerr;
    }

    return Noerr;
}

#endif //NETWORKS_NETWCOMMON_H