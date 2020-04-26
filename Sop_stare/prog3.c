#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include<signal.h>
#include<ctype.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#define DESC 2
#define MSG_SIZE (PIPE_BUF - sizeof(pid_t))
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
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
void sig_handler(int sig) {
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
void child_work(int pout,int Rin){
    srand(getpid());
    char c = 'a'+rand()%('z'-'a'+1);
    //printf("%d dziala dziecko \n",getpid());
    //printf("%d %c \n",getpid(),c);
    if(write(Rin,&c,1)<1) ERR("write to R");
}
void parent_work(int n,int *fds,int R) {
        char c;
        int status;
        srand(getpid());
        while((status=read(R,&c,1))==1) printf("%c",c);
        if(status<0) 
            ERR("read from R");
        printf("\n");

}
void create_children(int n, int *fds, int Rin){
    int tfil[DESC];
    int max=n;
    while(n){
        if(pipe(tfil)) ERR("pipe creating");

        switch(fork()){
            case 0:
                // tu się dzieje tylko dziecko
                //printf("dupa %d\n",n);
                //printf("%d dziala \n\n",n);
                while(n<max) if(fds[n] && close(fds[n++])) ERR("close");
                free(fds);
                if(close(tfil[1])) ERR("close tfil 1 child");
                
                child_work(tfil[0],Rin);
                
                if(close(tfil[0])) ERR("close tfil 0 child");
                if(close(Rin)) ERR("close Rin child");
                exit(EXIT_SUCCESS);
                //koniec dziecka
            case -1:
                ERR("fork error");
        }
        if(close(tfil[0])) ERR("close tfil 0 child");
        fds[--n]=tfil[1];
    }
    
}


int main(int argc, char** argv){
   int n,*fds, R[DESC];
   n = atoi(argv[1]);

   if(NULL==(fds=(int*)malloc(sizeof(int)*n))) 
        ERR("Malloc at the beginning");
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGINT);
    sigprocmask(SIG_BLOCK,&set,NULL);
    sethandler(sig_handler,SIGCHLD);
    if(pipe(R)) ERR("Pipe create R");
    
    create_children(n,fds,R[1]);
    if(close(R[1])) ERR("R 1 close");
    parent_work(n,fds,R[0]);
    
    printf("POWROT DRUGI \n");
    while(n--) if(fds[n] && close(fds[n])) ERR("close fds parent");
    if(close(R[0])) ERR("R 1 close");
    free(fds);
    
   exit(EXIT_SUCCESS);
}