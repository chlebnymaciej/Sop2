#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <mqueue.h>


#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
                                     exit(EXIT_FAILURE))


void sethandler( void (*f)(int, siginfo_t*, void*), int sigNo) {
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));
        act.sa_sigaction = f;
        act.sa_flags=SA_SIGINFO;
        if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}


void usage(){
        fprintf(stderr,"USAGE: mqueue n \n");
        fprintf(stderr,"100 >n > 0 - number of children\n");
        exit(EXIT_FAILURE);
}

typedef struct thread_data {
    int num;
    mqd_t desc;
} thread_data;

void my_thread(union sigval thdata){
    thread_data *data = thdata.sival_ptr;
    printf("watek %d\n", data->num);

    static struct sigevent not;
    not.sigev_notify = SIGEV_THREAD;
    not.sigev_notify_function = my_thread;
    not.sigev_value.sival_ptr = thdata.sival_ptr;

    //if(mq_notify(queues[i], &not)) ERR("mq_notify");

    /*for(;;){
        if(mq_receive(*pin,(char*)&ni,1,&msg_prio)<1) {
            if(errno == EAGAIN) break;
            else ERR("mq_receive");
        }*/
}


int main(int argc, char** argv) {

        if(argc != 2) usage();

        int n = atoi(argv[1]);
        if (n <= 0 || n > 100) usage();
        srand(time(NULL));
        mqd_t *queues = malloc(n * sizeof(mqd_t));
        thread_data *numbers = malloc(n * sizeof(thread_data));

        for(int i = 0; i < n; i++){
            char name[16];
            sprintf(name, "/myqueue%d", i + 1);
            struct mq_attr attrib;
            attrib.mq_maxmsg = 4;
            attrib.mq_msgsize = sizeof(int);

            if((queues[i] = TEMP_FAILURE_RETRY(mq_open(name, O_RDWR | O_CREAT, 0600, &attrib))) == -1) ERR("mq open in");
            printf("%s - %d\n", name, i + 1);

            numbers[i].num = i + 1;

            static struct sigevent not;
            not.sigev_notify = SIGEV_THREAD;
            not.sigev_notify_function = my_thread;
            not.sigev_value.sival_ptr = &numbers[i];

            if(mq_notify(queues[i], &not)) ERR("mq_notify");
        }

        int num;
        char buffer[32];
        while(scanf("%s", buffer) != EOF){
            num = atoi(buffer);
            printf("%d\n", num);

            if(TEMP_FAILURE_RETRY(mq_send(queues[rand() % n], (const char*)&num, sizeof(int), 1))) ERR("mq_send");
        }

        sleep(2);

        for(int i = 0; i < n; i++){
            char name[32];
            sprintf(name, "/myqueue%d", i + 1);

            if(mq_unlink(name)) ERR("mq unlink");

        }

        
        return EXIT_SUCCESS;
}