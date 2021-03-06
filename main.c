#include "light-netw.h"


int main(int argc, char *argv[]) {
    netwopts options = {
            .port = NULL,
            .input_param = NULL,
            .hostname = NULL,
            .output_param = NULL,
            .type = Invalid_type,
            .routing = AF_INET
    };
    error_code operes = 0;
    if ((operes = parse_flags(argc, argv, &options)) != Noerr) {
        return operes;
    }
    srand(time(NULL));

    switch (options.type) {
        case Server_dirshare:
            if (options.port == NULL)
                options.port = DEFAULT_PORT;
            operes = run_server(&options);
            break;

        case Client_filereq:
            if (options.port == NULL)
                options.port = DEFAULT_PORT;
            operes = run_client(&options);
            break;

        case Ping:
            operes = ping(&options);
            break;
        case Tracert:
            operes = tracert(&options);
            break;
        case Invalid_type:
        default:
            operes = Opterr;
            PRINT_FORMAT("invalid run type: %i", options.type);
            break;

    }

    PRINT("");
    PRINT_FORMAT("--> result code: %i <--", operes);

    return operes;
}
