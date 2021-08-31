#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[]) {
    if(argc >= 2) {
        fprintf(2, "usage: pingdong");
        exit(1);
    }
    int fd_child[2], fd_parent[2];
    char buf[1];
    //open 2 pipes
    pipe(fd_child);
    pipe(fd_parent);
    uint bufSize = sizeof(buf);
    //child: read from parent, then write to parent
    if(fork() == 0) {
        close(fd_parent[0]);
        close(fd_child[1]);
        if(read(fd_child[0], buf, bufSize) != bufSize) {
            printf("child read error");
            exit(1);
        }
        close(fd_child[0]);
        printf("%d: received ping\n", getpid());

        if(write(fd_parent[1], buf, bufSize) != bufSize) {
            printf("child write error");
            exit(1);
        }
        close(fd_parent[1]);
        exit(0);
    }
    //parent
    close(fd_parent[1]);
    close(fd_child[0]);
    if(write(fd_child[1], buf, bufSize) != bufSize) {
        printf("parent write error");
        exit(1);
    }
    close(fd_child[1]);
    if(read(fd_parent[0], buf, bufSize) != bufSize) {
        printf("parent read error");
        exit(1);
    }
    close(fd_parent[0]);
    printf("%d: received pong\n", getpid());
    wait(0);
    exit(0);
}