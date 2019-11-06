#ifndef LIGHT_NETW_NETWLOGGING_H
#define LIGHT_NETW_NETWLOGGING_H

#include "netwbase.h"
#include "netwtypes.h"


error_code logmess(char *formatmess, ...);

error_code printmess(char *formatmess, ...);

char *to_strerr(error_code err);


extern char *GLOBAL_PREFIX;

#define SET_PREFIX(prefix) GLOBAL_PREFIX = ""prefix"-> "
#define CLEAR_PREFIX() GLOBAL_PREFIX = ""

#define PRINT_FORMAT(format_str, ...) printmess("%s "format_str"", GLOBAL_PREFIX, __VA_ARGS__)

#define PRINT(str) PRINT_FORMAT(str"%s", "")

#define PRINT_ERROR(message, ...) PRINT_FORMAT("error: "message"\r\n", __VA_ARGS__)

#define PRINT_WSA_ERR(str) PRINT_ERROR(""str" %u\r\n", WSAGetLastError())

#endif //LIGHT_NETW_NETWLOGGING_H
