#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(2, "usage:sleep time(clock num)\n");
        exit(1);
    }
    int t = atoi(argv[1]);
    sleep(t);
    exit(0);
}