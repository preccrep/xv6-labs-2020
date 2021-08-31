#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int
main(int argc, char *argv[])
{
    char buf[2048], ch;
    char *p = buf;
    char *tokens[MAXARG];
    int token_size = argc - 1; //xargs will not be executed
    int offset = 0;
    if(argc <= 1){
		fprintf(2, "usage: xargs <command> [argv...]\n");
		exit(1);
	}
    for(int i = 0; i < token_size; i++)
        tokens[i] = argv[i + 1];
    while(read(0, &ch, 1) > 0) {
        if(ch == ' ' || ch == '\t') {
            buf[offset++] = 0;
            tokens[token_size++] = p;
            p = buf + offset;
            if(read(0, &ch, 1) <= 0) break;
        }
        if(ch != '\n') buf[offset++] = ch;
        else {
            tokens[token_size++] = p;
            p = buf + offset;
            if(!fork()) exit(exec(tokens[0], tokens));
            wait(0);
            token_size = argc - 1;
        }
    }
    exit(0);
}

