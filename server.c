#include "server.h"

#define BACKLOG 10   // how many pending connections queue will hold

int accpt(SOCKET servsock)
{
    struct sockaddr their_addr; // connector's address information
    socklen_t sin_size;
    int length = 100;
    char connector[length];

    SOCKET connsock;

    while (1) {  // main accept() loop
        sin_size = sizeof their_addr;
        connsock = accept(servsock, &their_addr, &sin_size);
        if (connsock == -1) {
            PWASAERR("accept error %u\n");
            return -1;
        }
        int err = 0;
        if ((err = WSAAddressToString(&their_addr, sizeof(struct sockaddr), NULL, connector, (LPDWORD) &length))) {
            PWASAERR("WSAAddressToStringA error %u\n(skip)\n");
            wprintf(L"server: error %i\n", err);
        }
        else
            wprintf(L"server: got connection from %s\n", connector);

        char data[] = "Hello, world!";

        wprintf(L"sending data...\n");

        if (send(connsock, data, sizeof(data) / (sizeof(char)), 0) != sizeof(data) / (sizeof(char))) {
            PWASAERR("send error %u\n");
            closesocket(connsock);
            return -1;
        }

        wprintf(L"sending successe\n");
        closesocket(connsock);
        break;
    }

    closesocket(connsock);
    return 0;
}

int run_server()
{
    WSADATA wsaData;
    STARTUP(&wsaData);

    struct addrinfo hints, *serv_info, *p;
    SOCKET sockfd = INVALID_SOCKET;

    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((getaddrinfo(NULL, PORT, &hints, &serv_info)) != 0) {
        PWASAERR("getaddrinfo: %s\n");
        CLEANUP(serv_info);
        return -1;
    }

    // loop through all the results and bind to the first we can
    for (p = serv_info; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            PWASAERR("socket error %u\n(keep trying)\n");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &yes,
                       sizeof(int)) == -1) {
            PWASAERR(L"setsockopt error %u\n");
            CLEANUP(serv_info);
            return -1;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen)) {
            close(sockfd);
            PWASAERR("bind error %u\n(keep trying)\n");
            continue;
        }

        break;
    }

    if (p == NULL) {
        wprintf(L"server: failed to bind\n");
        CLEANUP(serv_info, sockfd);
        return -1;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        PWASAERR("listen error %u\n");
        CLEANUP(serv_info, sockfd);
        return -1;
    }

    wprintf(L"server: waiting for connections...\n");

    int res = accpt(sockfd);

    CLEANUP(serv_info, sockfd);

    return res;
}