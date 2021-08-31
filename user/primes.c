#include "kernel/types.h"
#include "user/user.h"

#define MAXBOUND 35
int pipeline[2];

int
isPrime(int j) {
    for(int i = 2; i < j; i++)
        if(j % i == 0) return 0;
    return 1;
}

void
printPrime() {
    int s = 1;
    read(pipeline[0], &s, sizeof(s));
    do {s++;} while(!isPrime(s));
    if(s > MAXBOUND) return;
    printf("prime %d\n", s);
    write(pipeline[1], &s, sizeof(s));
    if(fork() == 0) printPrime();
    wait(0);
    return;
}

int
main(int argc, char *argv[]) {
    //open pipe
    pipe(pipeline);
    int s = 1;
    write(pipeline[1], &s, sizeof(s));
    printPrime();
    close(pipeline[0]);
    close(pipeline[1]);
    exit(0);
}