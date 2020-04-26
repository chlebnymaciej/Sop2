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

//ustawienie handlera
int sethandler( void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL))
		return -1;
	return 0;
}

typedef struct ThreadArgs{
	int index;
	int n;
	mqd_t my;
	mqd_t next;
} ThreadArgs_t;


void NotifyWork(union sigval s)
{
	ThreadArgs_t* args = (ThreadArgs_t*)s.sival_ptr;
	//char* name = (char*)malloc(32*sizeof(char));
	int n;
	printf("dupa\n ");
	static struct sigevent event_not;
	event_not.sigev_notify = SIGEV_THREAD;
	event_not.sigev_value.sival_ptr = s.sival_ptr;
	event_not.sigev_notify_function = NotifyWork;

	if(mq_notify(args->my,&event_not) < 0) ERR("notify w work");

	printf("\t %d \n", args->index);
	 for(;;){
	 	if(mq_receive(args->my,(char*)&n,sizeof(int),NULL)<1){
			if(errno== EAGAIN) break;
			else ERR("receive");
		}
	 	//n = atoi(name);
	 	if(args->index+1==args->n)
	 	{
	 		printf("ENEN %d \n",n);
	 		continue;
	 	}
	 	if(n % (args->index+1)>0)
	 	{
	 		if( TEMP_FAILURE_RETRY(mq_send(args->next,(const char*)&n,sizeof(int),1)))
	 			ERR("work send");
		}

	 }
	//free(name);
	return;
}

// obsuła SIGCHILD
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
// parametry wejsciowe bledy
void usage(char * name){
	fprintf(stderr,"USAGE: %s n\n",name);
	fprintf(stderr,"0<n<=100 - number of children\n");
	exit(EXIT_FAILURE);
}

/* Przydatne makro 
    TEMP_FAILURE_RETRY(expression)

*/

int main(int argc, char** argv){

	int n = atoi(argv[1]);

	mqd_t* queue = (mqd_t*)malloc(sizeof(mqd_t)*n);

	char* name = (char*)malloc(32*sizeof(char));
	ThreadArgs_t* waty = (ThreadArgs_t*)malloc(sizeof(ThreadArgs_t)*n);

	struct mq_attr attrs;
	attrs.mq_maxmsg=4;
	attrs.mq_msgsize=sizeof(int);


	for(int i = n-1; i >-1; i--)
	{
		sprintf(name,"/dupa%d",i);
		if((queue[i] = TEMP_FAILURE_RETRY(mq_open(name, O_RDWR | O_CREAT, 0600, &attrs))) == (mqd_t)-1) 
			ERR("mq open in");
            printf("%s - %d\n", name, i + 1);
		waty[i].index = i;
		waty[i].n = n;
		waty[i].my = queue[i];
		if(i<n-1)
			waty[i].next = queue[i+1];

		static sigevent_t not;
		not.sigev_notify = SIGEV_THREAD;
		not.sigev_value.sival_ptr = (void*)&waty[i];
		not.sigev_notify_function = NotifyWork;

		if(mq_notify(queue[i],&not) < 0) ERR("notify w work");
	}
	srand(time(NULL));

	while (scanf("%s",name)!=EOF)
	{
		int num = atoi(name);

		if( TEMP_FAILURE_RETRY(mq_send(queue[1],(const char*)&num,sizeof(int),1 )) )
		ERR("blad");
	}
	



	sleep(2);
	for(int i = 0;i<n;i++)
	{
		TEMP_FAILURE_RETRY(mq_close(queue[i]));
		sprintf(name,"/dupa%d",i);
		TEMP_FAILURE_RETRY(mq_unlink(name));

	}
		free(name);
		free(queue);
		free(waty);
    return EXIT_SUCCESS;
}