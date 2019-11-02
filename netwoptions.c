#include "netwoptions.h"

runtype get_type(char *name) {
    for (runtype type = 0; type < _run_type_count; type++) {
        if (!strcmp(name, run_type_names[type]))
            return type;
    }
    return Invalid_type;
}

error_code parse_flags(int argc, char *argv[], netwopts *options) {
    int opt;
    while ((opt = getopt(argc, argv, OPT_STR)) != -1) {
        switch (opt) {
            case ADDR_FLAG:
                options->hostname = optarg;
                break;
            case PORT_FLAG:
                options->port = optarg;
                break;
            case INPUT_PATH:
                options->input_path = optarg;
                break;
            case OUTPUT_PATH:
                options->output_path = optarg;
                break;
            case TYPE_FLAG:
                options->type = get_type(optarg);
                break;
            default:
                PRINT("options error\n");
                return Opterr;
        }
    }

    return Noerr;
}