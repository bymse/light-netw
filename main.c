#include "simplenetw.h"

int main(int argc, char *argv[]) {
    netwopts options;
    error_code operes = 0;
    if ((operes = parse_flags(argc, argv, &options)) != Noerr) {
        return operes;
    }

    if (options.port == NULL)
        options.port = DEFAULT_PORT;

    switch (options.type) {
        case Server_dirshare:
            if (SetCurrentDirectory(options.datapath) == 0) {
                PRINT_FORMAT("Directory error: %lu", GetLastError());
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
            operes = Notyerr;
            PRINT_FORMAT("invalid run type: %i \n", options.type);
            break;
    }

    PRINT_FORMAT("\n--> result code: %i <--\n", operes);

    return operes;
}
