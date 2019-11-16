#include "../filesys.h"


error_code try_read_file(char *path, char **data, unsigned long *data_size) {
    error_code operes;
    HANDLE file;
    if ((operes = open_file(path, &file, TRUE)) != Noerr) {
        return operes;
    }

    operes = read_all(file, data, data_size);
    CloseHandle(file);
    LOG_FORMAT("close file %s", path);
    return operes;
}

error_code open_file(char *path, HANDLE *filed, BOOL for_read) {
    DWORD open_opt = for_read ? OPEN_EXISTING : CREATE_ALWAYS;
    if ((*filed = CreateFileA((char *) path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, open_opt,
                              FILE_ATTRIBUTE_NORMAL, NULL)) == NULL) {
        ERR("CreateFileA(%s) %lu", path, GetLastError());
        return Filerr;
    }
    LOG_FORMAT("open file %s", path);
    return Noerr;
}

error_code read_all(HANDLE file, char **data, unsigned long *data_length) {
    int file_size = GetFileSize(file, NULL);
    if (file_size < 0) {
        LOG_FORMAT("invalid file length : %i", file_size);
        return Filerr;
    }
    error_code operes;
    if ((operes = re_memalloc(data, file_size)) != Noerr) {
        return operes;
    }

    if (ReadFile(file, *data, file_size, data_length, NULL) == FALSE) {
        ERR("ReadFile %lu", GetLastError());
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
        ERR("WriteFile %lu", GetLastError());
        operes = Filerr;
    } else operes = Noerr;

    CloseHandle(file);
    LOG_FORMAT("close file %s", path);
    return operes;
}

error_code check_dir(char *path) {
    DWORD attrib = GetFileAttributes(path);

    if (attrib == INVALID_FILE_ATTRIBUTES
        || (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        ERR("GetFileAttributes(%s) %lu", path, GetLastError());
        return Patherr;
    }

    return Noerr;
}
