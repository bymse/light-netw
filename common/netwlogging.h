#ifndef LIGHT_NETW_NETWLOGGING_H
#define LIGHT_NETW_NETWLOGGING_H

#include "netwbase.h"
#include "netwtypes.h"
#include "netwcleanup.h"

error_code logmess(char *formatmess, ...);

error_code printmess(char *formatmess, ...);

error_code logs_init(char *pathtologs);

void logs_cleanup();

char *to_strerr(error_code err);

#define LOGS_NAME "light-netw.logs.log"
extern char *GLOBAL_PREFIX;
extern FILE *LOGS;
extern pid_t CURPID;


#define SET_PREFIX(prefix) GLOBAL_PREFIX = ""prefix"-> "
#define CLEAR_PREFIX() GLOBAL_PREFIX = ""

#define PRINT_FORMAT(format_str, ...) printf("%s"format_str"\r\n", GLOBAL_PREFIX, __VA_ARGS__)
#define PRINT(str) PRINT_FORMAT(str"%s", "")

#define LOG_FORMAT(format_str, ...) fprintf(LOGS, "%lli: %s"format_str"\r\n", CURPID, GLOBAL_PREFIX, __VA_ARGS__)
#define LOG(str) LOG_FORMAT(str"%s", "")

#define WRITE_FORMAT(str, ...) W(PRINT_FORMAT(str, __VA_ARGS__); LOG_FORMAT(str, __VA_ARGS__))
#define WRITE(str) W(PRINT(str); LOG(str))

#define ERR(message, ...) WRITE_FORMAT("error: "message"", __VA_ARGS__)
#define WSA_ERR(str) ERR(""str" %u", WSAGetLastError())

#endif //LIGHT_NETW_NETWLOGGING_H
