#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include<ctype.h>
#include<signal.h>
#define MSG_SIZE (PIPE_BUF - sizeof(pid_t))
#define ERR(source) (perror(source),\
                     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     exit(EXIT_FAILURE))

void usage(char *name){
        fprintf(stderr,"USAGE: %s fifo_file\n", name);
        exit(EXIT_FAILURE);
}
void sethandler(void (*f)(int), int sig){
    struct sigaction act;
    memset(&act,0, sizeof(struct sigaction));
    act.sa_handler = f;
    if(-1==sigaction(sig,&act,NULL)) ERR("sigaction");
    
}
void sigpipe_handler(int sig){
    switch (sig)
    {
    case SIGPIPE:
        printf("Sigpipe have come\n");
        break;
    
    default:
        printf("Other signal");
        break;
    }
}

void read_from_fifo(int fifo){
    ssize_t count;
    char c;
    do{
        if((count=read(fifo,&c,1))<0) ERR("read");
        if(count>0 && isalpha(c)) printf("%c",c);
    }while(count>0);

    printf("\n");
}
int main(int argc, char** argv){
    sethandler(sigpipe_handler,SIGPIPE);
    int fifo;
    if(mkfifo(argv[1],S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)<0)
        if( errno!=EEXIST) ERR("creating fifo 1 prog");
    
    if((fifo=open(argv[1], O_RDONLY))<0) ERR("open");
    read_from_fifo(fifo);
    if(close(fifo)<0) ERR("close");
    

    if(unlink(argv[1])<0)
        if( errno== ENOENT) printf("\n\n already deleted 1");
        else ERR("something wrong 1");
    return EXIT_SUCCESS;
}
