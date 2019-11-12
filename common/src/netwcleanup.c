#include "../netwcleanup.h"

void freepacket(packet_t *packet) {
    free(packet->data - 1);
}