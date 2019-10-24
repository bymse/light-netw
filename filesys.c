#include "filesys.h"

error_code check_file(char *path) {
    HANDLE hFind;
    WIN32_FIND_DATA FindFileData;

    if ((hFind = FindFirstFile(path, &FindFileData)) == INVALID_HANDLE_VALUE) {
        PRINT_FORMAT("file %s error: %lu\n", path, GetLastError());
        return Opterr;
    }

    FindClose(hFind);
    return Patherr;
}

error_code check_dir(char *path) {
    DWORD attrib = GetFileAttributes(path);

    if (attrib == INVALID_FILE_ATTRIBUTES
        || (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        PRINT_FORMAT("directory %s error: %lu\n", path, GetLastError());
        return Opterr;
    }

    return Patherr;
}

error_code open_file(char *path, FILE **data) {
    if ((*data = fopen(path, OPEN_MODE)) == NULL) {
        PRINT_FORMAT("open file %s error: %u", path, errno);
        return Filerr;
    }
    return Noerr;
}