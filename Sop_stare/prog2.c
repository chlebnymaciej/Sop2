#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include<signal.h>
#include<ctype.h>
//#define PIPE_BUF _PC_PIPE_BUF
#define MSG_SIZE (PIPE_BUF - sizeof(pid_t))
#define ERR(source) (perror(source),\
                     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     exit(EXIT_FAILURE))

void usage(char *name){
        fprintf(stderr,"USAGE: %s fifo_file\n", name);
        exit(EXIT_FAILURE);
}
/*void sethandler(void (*f)(int), int sig){
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
}*/
void write_to_fifo(int fifo, int file){
    ssize_t count;
    char buffer[PIPE_BUF];
    char *buf;
    *((pid_t *) buffer) = getpid();
    buf = buffer + sizeof(pid_t);
    do{
        if((count= read(file,buf,MSG_SIZE))<0) ERR("read");
        if(count<MSG_SIZE) memset(buf+count,0,MSG_SIZE-count);
        if(count>0)
            if(write(fifo,buffer,PIPE_BUF)<0)ERR("write");
    }while(count==MSG_SIZE);
    
}
void read_from_fifo(int fifo){
    ssize_t count;
    char c;
    do{
        if((count=read(fifo,&c,1))<0) ERR("read");
        if(count>0 && isalpha(c)) printf("%c",c);
    }while(count>0);
}
int main(int argc, char** argv){
   // sethandler(sigpipe_handler,SIGPIPE);
    int fifo,file;
    if(mkfifo(argv[1],S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)<0)
        if( errno!=EEXIST) ERR("creating fifo");
    
    if((file=open(argv[2], O_RDONLY))<0) ERR("open file");
    if((fifo=open(argv[1], O_WRONLY))<0) ERR("open fifo");
    write_to_fifo(fifo,file);
    if(close(fifo)<0) ERR("close");
    if(close(file)<0) ERR("close");
    

    if(unlink(argv[1])<0)
        if( errno== ENOENT) printf(" 2 already deleted \n");
        else err("something wrong 2");
    else printf("deleted 2 \n");
    return EXIT_SUCCESS;
}