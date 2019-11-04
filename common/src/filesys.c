#include "../filesys.h"


error_code try_read_file(char *path, char **data, unsigned long *data_size) {
    error_code operes;
    HANDLE file;
    if ((operes = open_file(path, &file, TRUE)) != Noerr) {
        return operes;
    }

    operes = read_all(file, data, data_size);
    CloseHandle(file);
    return operes;
}

error_code open_file(char *path, HANDLE *filed, BOOL for_read) {
    DWORD open_opt = for_read ? OPEN_EXISTING : CREATE_ALWAYS;
    if ((*filed = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, open_opt,
                              FILE_ATTRIBUTE_NORMAL, NULL)) == NULL) {
        PRINT_ERROR("CreateFileA(%s) %lu", path, GetLastError());
        return Filerr;
    }
    return Noerr;
}

error_code read_all(HANDLE file, char **data, unsigned long *data_length) {
    unsigned long file_size = GetFileSize(file, NULL);
    error_code operes;
    if ((operes = re_memalloc(data, file_size)) != Noerr) {
        return operes;
    }

    if (ReadFile(file, *data, file_size, data_length, NULL) == FALSE) {
        PRINT_ERROR("ReadFile %lu", GetLastError());
        return Filerr;
    }

    return Noerr;
}

error_code write_file(char *path, char *data, unsigned long data_s) {
    error_code operes;
    HANDLE file;
    DWORD numberOfBytesWritten;

    if ((operes = open_file(path, &file, FALSE)) != Noerr) {
        return operes;
    }

    if (WriteFile(file, data, data_s, &numberOfBytesWritten, NULL) == FALSE) {
        PRINT_ERROR("WriteFile %lu", GetLastError());
        operes = Filerr;
    } else operes = Noerr;

    CloseHandle(file);
    return operes;
}

error_code check_dir(char *path) {
    DWORD attrib = GetFileAttributes(path);

    if (attrib == INVALID_FILE_ATTRIBUTES
        || (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        PRINT_ERROR("GetFileAttributes(%s) %lu", path, GetLastError());
        return Patherr;
    }

    return Noerr;
}
