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
#include <errno.h>
#include <string.h>
#include <time.h>
#include <limits.h>

//makro err
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     exit(EXIT_FAILURE))

// struktura sigevent dla wątków

static sigevent_t not;
not.sigev_notify = SIGEV_THREAD;
not.sigev_value.sival_ptr = // argumenty
not.sigev_notify_function = // funkcja sygnatura: void **nazwa**(union sigval s)

// notyfikacja
if(mq_notify(queue[i],&not) < 0) ERR("notify w work");

// atrybuty kolejki
struct mq_attr attrs;
attrs.mq_maxmsg=4;
attrs.mq_msgsize=sizeof(int);

// otwieranie kolejek

if((queue = TEMP_FAILURE_RETRY(mq_open(name, O_RDWR | O_CREAT, 0600, &attrs))) == (mqd_t)-1) 
	ERR("mq open in");

// zamykanie i usuwanie

		if(TEMP_FAILURE_RETRY(mq_close( /*file deskryptor */))<0)
			ERR("close");

		if(TEMP_FAILURE_RETRY(mq_unlink(/*nazwa kolejki*/)))
			ERR("Unlink")''

// wysyłanie
if( TEMP_FAILURE_RETRY(mq_send(queue[1], (const char*)&num, sizeof(int), /*prorytet*/ )) )
		ERR("blad send");

// odbieranie
if( TEMP_FAILURE_RETRY(mq_receive(pout,(char*)&ni,1,NULL))<1 )
		ERR("mq_receive");


// maska sygnału
sigset_t set;
sigemptyset(&set);
sigaddset(&set,SIGINT);
sigprocmask(SIG_BLOCK,&set,NULL);


// tworzenie wątków
int err = pthread_create(&tid, NULL, void* function, &argumenty);
if (err != 0) ERR("Couldn't create thread");
// czekanie na wątki
int err = pthread_join(tid, (void*)&subresult);
if (err != 0) ERR("Can't join with a thread");


// msleep

void msleep(UINT milisec) {
    time_t sec= (int)(milisec/1000);
    milisec = milisec - (sec*1000);
    timespec_t req= {0};
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    if(nanosleep(&req,&req)) ERR("nanosleep");
}