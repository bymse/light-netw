#ifndef NETWORKS_CLIENT_H
#define NETWORKS_CLIENT_H

#include "netwcommon.h"

error_code request_data(const char *target_addr, const char *port, FILE *data);

#define CLIENT_PREFIX "CLIENT->"

#define PRINT_CLIENT_WSA_ERR(format_str) PRINT_WSA_ERR(CLIENT_PREFIX" error: "format_str)
#define PRINT_CLIENT_FORMAT(format_str, ...) PRINT_FORMAT(CLIENT_PREFIX" "format_str, #__VA_ARGS__)
#define PRINT_CLIENT(str) PRINT_CLIENT_FORMAT(str)

#endif //NETWORKS_CLIENT_H
