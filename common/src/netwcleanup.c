#include "../netwcleanup.h"

void freepacket(packet_t *packet) {
    CLEANUP(packet->data - 1);
}
