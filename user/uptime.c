#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

//SYS_uptime
int main(int argc, char *argv[]) {
    int nt = uptime();
    printf("the system has run %d tick.\n", nt);
    exit(0);
}