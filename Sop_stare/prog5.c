#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include<signal.h> 
#include<ctype.h>
#include <string.h>
#include <time.h>
#define DESC 2

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
                     exit(EXIT_FAILURE))

void usage(char *name){
        fprintf(stderr,"USAGE: %s fifo_file\n", name);
        exit(EXIT_FAILURE);
}

int sethandler( void (*f)(int), int sigNo) {
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));
        act.sa_handler = f;
        if (-1==sigaction(sigNo, &act, NULL))
                return -1;
        return 0;
}

void sigchld_handler(int sig) {
        pid_t pid;
        for(;;){
                pid=waitpid(0, NULL, WNOHANG);
                if(0==pid) return;
                if(0>=pid) {
                        if(ECHILD==errno) return;
                        ERR("waitpid:");
                }
        }
}
void child_work(int fd, int R){
    srand(getpid());
    char c = 'a'+rand()%('z'-'a');
    if(write(R,&c,1)<0) ERR("write");
    return;
}
void create_pipes_and_forks(int n, int* fds, int R){
    int tmpfd[2];
    int max = n;
    while(n){
        if(pipe(tmpfd)) ERR("pipe tmpfd");
        switch (fork())
        {
        case 0:
            while(n<max) if(fds[n] && close(fds[n++])) ERR("close");
            free(fds);
            if(close(tmpfd[1])) ERR("close tmpfd 1");
            child_work(tmpfd[0],R);
            if(close(tmpfd[0])) ERR("close tmpfd 0");
            if(close(R)) ERR("close R");

            exit(EXIT_SUCCESS);
        case -1: ERR("fork");
        }

        if(close(tmpfd[0])) ERR("close tmpfd 0");
        fds[--n]=tmpfd[1];
    }
}
void parent_work(int n,int *fds,int R) {
        char c;
        int status;
        srand(getpid());
        while((status=TEMP_FAILURE_RETRY( read(R,&c,1)) )==1) printf("%c",c);
        if(status<0) ERR("read from R");
        printf("\n");
}

int main(int argc, char** argv){
    int n, *fds, R[2];
    n=atoi(argv[1]);

    if(NULL== (fds=(int*)malloc(sizeof(int)*n))) ERR("malloc");
    sethandler(sigchld_handler,SIGCHLD);

    if(pipe(R)) ERR("pipe R");
    create_pipes_and_forks(n,fds,R[1]);
    if(close(R[1])) ERR("main close R1");
    parent_work(n,fds,R[0]);
    //if(close(R[0])) ERR("main close R0");

    while(n--) if(fds[n]&&close(fds[n])) ERR("close");
    if(close(R[0])) ERR("close");
    free(fds);
    return EXIT_SUCCESS;
}