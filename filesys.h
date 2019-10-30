#ifndef NETWORKS_FILESYS_H
#define NETWORKS_FILESYS_H

#include <windows.h>
#include "netwcommon.h"

error_code try_read_file(char *path, char **data, unsigned long *data_size, char *prefix);

error_code open_file(char *path, HANDLE *filed, char *prefix);

error_code read_all(HANDLE file, char **data, unsigned long *data_length, char *prefix);

#endif //NETWORKS_FILESYS_H
