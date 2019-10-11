#include "netwcommon.h"

#define CUST_MAKEWORD(a, b)    ((WORD)(((unsigned)(a))|(((unsigned)(b))<<8u)))

error_code wsa_start(WSAData *wsaData, const char *prefix) {
    int err = 0;
    if ((err = WSAStartup(CUST_MAKEWORD(2, 2), wsaData))
        != NO_ERROR) {
        wprintf(L"%s: WSAStartup %d\n", prefix, err);
        return WSAStarterr;
    }
    return Noerr;
}