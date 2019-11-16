#include "../netwlogging.h"


char *GLOBAL_PREFIX = "";
FILE *LOGS = NULL;
pid_t CURPID;

error_code logs_init(char *pathtologs) {
    if (pathtologs == NULL)
        pathtologs = LOGS_NAME;
    if ((LOGS = fopen(pathtologs, "a")) == NULL) {
        PRINT("Logger init fail");
        return Logerr;
    }
    CURPID = getpid();
    return Noerr;
}

void logs_cleanup() {
    CLEANUP(LOGS);
}


