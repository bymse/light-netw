#include "client.h"

#define MAXDATASIZE 100 // max number of bytes we can get at once 

int send_request() {
    WSADATA wsaData;
    STARTUP(&wsaData);
    int numbytes;
    SOCKET sockfd = INVALID_SOCKET;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char server[100];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
        //fprintf(stderr, "getaddrinfo: %d\n", rv);
        PWASAERR("getaddrinfo: %d\n");
        CLEANUP(servinfo);
        return -1;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            PWASAERR("socket error %u\n(keep trying)\n");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            PWASAERR("client: connect");
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        wprintf(L"client: failed to bind\n");
        CLEANUP(servinfo, sockfd);
        return -1;
    }

    if (WSAAddressToStringA(p->ai_addr, sizeof(struct sockaddr), NULL, server, (LPDWORD) 100))
        PWASAERR("WSAAddressToStringA error %u\n(skip)\n");
    else
        wprintf(L"client: connecting to %s\n", p->ai_canonname);


    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
        CLEANUP(servinfo, sockfd);
        return -1;
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n", buf);

    CLEANUP(servinfo, sockfd);

    return 0;
}

