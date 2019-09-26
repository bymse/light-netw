#include "netwcommon.h"

#define PORT "3490"  // the port users will be connecting to

#define STARTUP(wsaData)                                            \
        do{                                                         \
            int __res = 0;                                          \
            if ( (__res = WSAStartup(                               \
                    _MAKEWORD(2,2),                                 \
                    (WSADATA*)(wsaData)) != NO_ERROR)) {              \
                wprintf(L"WSAStartup error: %d\n", __res);          \
            return 1;                                               \
            }}while(0)                                              \

#define _CLEANUP0() WSACleanup(); printf("cleanup done\r\n")
#define _CLEANUP1(addr)                 \
    do{if((addr) != NULL)                 \
        freeaddrinfo((void*)((addr)));    \
    _CLEANUP0();                        \
    }while(0)                           \


#define _CLEANUP2(addr, socket)         \
    do{if((socket) > 0)                   \
        closesocket((socket)+0L);         \
    _CLEANUP1((addr));                    \
    }while(0)                           \

#define _CLEANUP3(addr, socket, file)   \
    do{if((file) != NULL){                \
        fflush((void *)(file));           \
        fclose((void *)(file));           \
    }                                   \
    _CLEANUP2(addr, socket);            \
    }while(0)                           \

#define CLEANUP(...) _GET_OVERRIDE("ignored", ##__VA_ARGS__, _CLEANUP3, _CLEANUP2, _CLEANUP1, _CLEANUP0)(__VA_ARGS__)
#define PERROR(format_str) wprintf(L""format_str"", WSAGetLastError())
#define BACKLOG 10     // how many pending connections queue will hold

int main(void) {
    WSADATA wsaData;
    STARTUP(&wsaData);

    struct addrinfo hints, *serv_info, *p;
    int oper_res, sockfd = INVALID_SOCKET, new_fd;

    struct sockaddr their_addr; // connector's address information
    socklen_t sin_size;

    int yes = 1;
    char s[INET_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((oper_res = getaddrinfo(NULL, PORT, &hints, &serv_info)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(oper_res));
        CLEANUP(serv_info);
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = serv_info; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            PERROR("socket error %u\n(keep trying)\n");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &yes,
                       sizeof(int)) == -1) {
            PERROR(L"setsockopt error %u\n");
            CLEANUP(serv_info);
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen)) {
            close(sockfd);
            PERROR("bind error %u\n(keep trying)\n");
            continue;
        }

        break;
    }

    //freeaddrinfo(serv_info); // all done with this structure

    if (p == NULL) {
        wprintf(L"server: failed to bind\n");
        CLEANUP(serv_info, sockfd);
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        PERROR("listen error %u\n");
        CLEANUP(serv_info, sockfd);
        exit(1);
    }

    wprintf(L"server: waiting for connections...\n");

    while (1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, &their_addr, &sin_size);
        if (new_fd == -1) {
            PERROR("accept error %u\n(keep trying)\n");
            continue;
        }

        if (WSAAddressToStringA(&their_addr, INET_ADDRSTRLEN, NULL, s, (LPDWORD) INET_ADDRSTRLEN))
            PERROR("WSAAddressToStringA error %u\n(skip)");
        else
            wprintf(L"server: got connection from %s\n", s);

        char data[] = "Hello, world!";

        wprintf(L"sending data...\n");

        if (send(new_fd, data, sizeof(data) / (sizeof(char)), 0)) {
            PERROR("send error %u\n");
            closesocket(new_fd);
            CLEANUP(serv_info, sockfd);
            return 1;
        }

        wprintf(L"sending successe\n");
        closesocket(new_fd);
        break;
    }

    CLEANUP(serv_info, sockfd);

    return 0;
}
