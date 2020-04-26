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
#define MAX_BUFF 200

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
                     exit(EXIT_FAILURE))

void usage(char *name){
        fprintf(stderr,"USAGE: %s fifo_file\n", name);
        exit(EXIT_FAILURE);
}
volatile sig_atomic_t last_signal =0;
//ustawianie handlera
int sethandler( void (*f)(int), int sigNo) {
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));
        act.sa_handler = f;
        if (-1==sigaction(sigNo, &act, NULL))
                return -1;
        return 0;
}
// zabójstwo z 20% szczęściem
void sigkillme_handler(int sig){
        printf("proces %d otrzymal sigint\n",getpid());
        if(rand()%5==0){
                printf("śmierć procesowi %d \n",getpid());
                exit(EXIT_SUCCESS);
        }
}
//handler dziecka
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
//sigint handler dla rodzica
void sig_handler(int sig){
        last_signal=sig;
}

// zadanie dziecka
void child_work(int fd, int R){
    srand(getpid());
    char c, buf[MAX_BUFF+1];
    unsigned char s;
    sethandler(sigkillme_handler, SIGINT);
    for(;;){
        if(TEMP_FAILURE_RETRY(read(fd,&c,1))<1) ERR("read 1 char");  
        s=rand()%(MAX_BUFF)+1;
        buf[0]=s;
        memset(buf+1,c,s);
        if(TEMP_FAILURE_RETRY(write(R,buf,s+1))<0) ERR("write child");
    }
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
            while(n<max) if(fds[n] && TEMP_FAILURE_RETRY(close(fds[n++]))) ERR("close");
            free(fds);
            if(TEMP_FAILURE_RETRY(close(tmpfd[1]))) ERR("close tmpfd 1");
            child_work(tmpfd[0],R);
            if(TEMP_FAILURE_RETRY(close(tmpfd[0]))) ERR("close tmpfd 0");
            if(TEMP_FAILURE_RETRY(close(R))) ERR("close R");

            exit(EXIT_SUCCESS);
        case -1: ERR("fork");
        }

        if(TEMP_FAILURE_RETRY(close(tmpfd[0]))) ERR("close tmpfd 0");
        fds[--n]=tmpfd[1];
    }
}
void parent_work(int n,int *fds,int R) {
       char buff[MAX_BUFF];
       unsigned char c;
       int status,i;
       srand(getpid());
       sethandler(sig_handler,SIGINT);
       for(;;){
               if(SIGINT==last_signal){
                i=rand()%n;
                while(0==fds[i%n] && i<2*n)i++;
                i%=n;
                if(fds[i]){
                        c='a'+rand()%('z'-'a');
                        status=TEMP_FAILURE_RETRY(write(fds[i],&c,1));
                        if(status!=1){
                               if(TEMP_FAILURE_RETRY(close(fds[i]))) ERR("close");
                                        fds[i]=0;
                        }
                } 
                last_signal=0;
               }
               
         status=read(R,&c,1);
         if(status<0 && errno==EINTR) continue;
         if(status<0) ERR("read one char parent");
         if(0==status) break;
         if(TEMP_FAILURE_RETRY(read(R,buff,c))<c) ERR("readind many chars");
         buff[(int)c]=0;
         printf("\n%s\n", buff);
       }
}

int main(int argc, char** argv){
    int n, *fds, R[2];
    n=atoi(argv[1]);

    
    if(NULL== (fds=(int*)malloc(sizeof(int)*n))) ERR("malloc");
    sethandler(SIG_IGN,SIGINT);
    sethandler(SIG_IGN,SIGPIPE);
    sethandler(sigchld_handler,SIGCHLD);

    if(pipe(R)) ERR("pipe R");
    create_pipes_and_forks(n,fds,R[1]);
    if(TEMP_FAILURE_RETRY(close(R[1]))) ERR("main close R1");
    parent_work(n,fds,R[0]);

    while(n--) if(fds[n]&&TEMP_FAILURE_RETRY(close(fds[n]))) ERR("close");
    if(TEMP_FAILURE_RETRY(close(R[0]))) ERR("close");
    free(fds);
    return EXIT_SUCCESS;
}