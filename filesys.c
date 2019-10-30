#include "filesys.h"


error_code try_read_file(char *path, char **data, unsigned long *data_size, char *prefix) {
    error_code operes;
    HANDLE file;
    if ((operes = open_file(path, &file, prefix)) != Noerr) {
        return operes;
    }

    operes = read_all(file, data, data_size, prefix);
    CloseHandle(file);
    return operes;
}

error_code open_file(char *path, HANDLE *filed, char *prefix) {
    if ((*filed = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, NULL)) == NULL) {
        PRINT_FORMAT("%s open file %s error: %lu", prefix, path, GetLastError());
        return Filerr;
    }
    return Noerr;
}

error_code read_all(HANDLE file, char **data, unsigned long *data_length, char *prefix) {
    unsigned long file_size = GetFileSize(file, NULL);
    if ((*data = malloc(file_size)) == NULL) {
        PRINT_FORMAT("%s error: memory allocation %i", prefix, errno);
        return Memerr;
    }

    if (ReadFile(file, *data, file_size, data_length, NULL) == FALSE) {
        PRINT_FORMAT("%s error: read file %lu", prefix, GetLastError());
        return Filerr;
    }

    return Noerr;
}

error_code check_dir(char *path, char *prefix) {
    DWORD attrib = GetFileAttributes(path);

    if (attrib == INVALID_FILE_ATTRIBUTES
        || (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        PRINT_FORMAT("%s directory %s error: %lu\n", prefix, path, GetLastError());
        return Patherr;
    }

    return Noerr;
}
