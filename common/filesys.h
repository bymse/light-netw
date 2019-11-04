#ifndef NETWORKS_FILESYS_H
#define NETWORKS_FILESYS_H

#include "netwcommon.h"

error_code try_read_file(char *path, char **data, unsigned long *data_size);

error_code write_file(char *path, char *data, unsigned long data_s);

error_code open_file(char *path, HANDLE *filed, BOOL for_read);

error_code read_all(HANDLE file, char **data, unsigned long *data_length);

#endif //NETWORKS_FILESYS_H
