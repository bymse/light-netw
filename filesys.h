#ifndef NETWORKS_FILESYS_H
#define NETWORKS_FILESYS_H

#include <windows.h>
#include "netwcommon.h"

#define OPEN_MODE "wb"

error_code check_dir(char *path);

error_code check_file(char *path);

error_code open_file(char *path, FILE **data);

#endif //NETWORKS_FILESYS_H
