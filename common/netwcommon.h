#ifndef NETWORKS_NETWCOMMON_H
#define NETWORKS_NETWCOMMON_H

#include "netwbase.h"
#include "netwtypes.h"
#include "netwlogging.h"
#include "netwcleanup.h"

#define DEFAULT_PORT "3490"
#define MAX_PACKET_S 1300
#define PACKET_HEADERS_SIZE (sizeof(char) + sizeof(char))


error_code netwinit(const netwopts *options, addrinfo *hints, addrinfo **target_addrinfo);

error_code send_packet(SOCKET sockd, packet_t *packet);

error_code rcv_packet(SOCKET sockd, packet_t *packet, BOOL add_terminator);

void print_addr(sockaddr_storage *addr);

static inline error_code re_memalloc(char **ptr, size_t size) {
    if ((*ptr = realloc(*ptr, size)) == NULL) {
        ERR("memory allocation %i", errno);
        return Memerr;
    }

    return Noerr;
}

#endif //NETWORKS_NETWCOMMON_H