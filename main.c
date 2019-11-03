#include "light-netw.h"

int main(int argc, char *argv[]) {
    netwopts options = {
            .port = NULL,
            .input_path = NULL,
            .hostname = NULL,
            .output_path = NULL,
            .type = Invalid_type,
            .routing = AF_INET
    };
    error_code operes = 0;
    if ((operes = parse_flags(argc, argv, &options)) != Noerr) {
        return operes;
    }

    if (options.port == NULL)
        options.port = DEFAULT_PORT;

    switch (options.type) {
        case Server_dirshare:
            if (SetCurrentDirectory(options.input_path) == 0) {
                PRINT_ERROR("SetCurrentDirectory %lu", GetLastError());
                operes = Patherr;
                break;
            }
        case Server_message:
            operes = run_server(&options);
            break;

        case Client_filereq:
        case Client_message:
            operes = run_client(&options);
            break;

        case Invalid_type:
        case _run_type_count:
            operes = Opterr;
            PRINT_FORMAT("invalid run type: %i\r\n", options.type);
            break;
    }

    PRINT_FORMAT("\r\n--> result code: %i <--\r\n", operes);

    return operes;
}
