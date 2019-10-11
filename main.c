#include "simplenetw.h"


int main(int argc, char *argv[]) {

    if (argc != 2) {
        perror("incorrect number of arguments\n");
        return -1;
    }

    int res = 0;
    
    if (argv[1][0] == 's') {
        res = run_server();
    } else if (argv[1][0] == 'c') {
        res = request_data();
    }
    if (res < 0) {
        printf("error");
        return -1;
    }
    
    printf("END\n");

    return 0;
}
