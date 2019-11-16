#ifndef LIGHT_NETW_NETWTYPES_H
#define LIGHT_NETW_NETWTYPES_H

#include "netwbase.h"

typedef enum error_code {
    Logerr = -101,
    Opterr = -100,
    Patherr = -99,
    Filerr = -98,
    Memerr = -97,
    Packerr = -50,
    WSAStarterr = -10,
    Addrerr = -9,
    Sockerr = -8,
    Recverr = -7,
    Binderr = -6,
    Listenerr = -5,
    Accepterr = -4,
    Senderr = -3,
    Noerr = 0
} error_code;


typedef enum run_type {
    Invalid_type,
    Server_dirshare,
    Server_message,
    Client_filereq,
    Client_message,

    _run_type_count
} runtype;

static char const *const run_type_names[_run_type_count] = {
        [Invalid_type] = "i",
        [Server_dirshare] = "sd",
        [Server_message] = "sm",
        [Client_filereq] = "cf",
        [Client_message] = "cm",
};


typedef struct netwopts {
    char *port;
    char *hostname;
    char *input_path;
    char *output_path;
    char *logs_path;
    runtype type;
    int routing; //v4 for IPv4, v6 for IPv6
} netwopts;

typedef struct packet_t {
    error_code state_code;
    size_t data_s;
    char *data;
} packet_t;

typedef struct addrinfo addrinfo;
typedef struct WSAData WSAData;
typedef struct sockaddr_storage sockaddr_storage;

#endif //LIGHT_NETW_NETWTYPES_H
