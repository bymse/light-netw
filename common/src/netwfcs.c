#include "../netwfcs.h"

u_short Internet_checksum(u_short *data, u_short size) {
    unsigned long cksum = 0;
    while (size > 1) {
        cksum += *data++;
        size -= sizeof(u_short);
    }
    if (size) {
        cksum += *(u_char *) data;
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    return (u_short) (~cksum);
}