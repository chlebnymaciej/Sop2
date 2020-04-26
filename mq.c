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

#define MAX_NUM 10
#define LIFE_SPAN 10
volatile sig_atomic_t children_left = 0;
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
		children_left--;
	}
}
// parametry wejsciowe bledy
void usage(char * name){
	fprintf(stderr,"USAGE: %s n\n",name);
	fprintf(stderr,"0<n<=10 - number of children\n");
	exit(EXIT_FAILURE);
}

/* Przydatne makro 
    TEMP_FAILURE_RETRY(expression)

*/
void ChildWork(int n, mqd_t psend, mqd_t preceive){
	srand(getpid());
	int8_t life = rand()%LIFE_SPAN+1;
	int8_t bingo = (uint8_t)(rand()%MAX_NUM);

	
}


void Create_Children(int n, mqd_t psend, mqd_t preceive)
{
	while(n-->0)
	{
		switch (fork())
		{
		case 0:
			// ChildWork(n, psend,preceive);
			break;
		case -1:
			ERR("fork");
		}
		children_left++;

	}
}


int main(int argc, char** argv){

	//int n = atoi(argv[1]);
	mqd_t parent_send, parent_receive;

	struct mq_attr attr;
	attr.mq_maxmsg=10;
	attr.mq_msgsize=1;

	if( 
		(mqd_t)-1 == (parent_send = TEMP_FAILURE_RETRY( mq_open("/parent_out",O_CREAT | O_RDWR, 0600, &attr) ) )
	)
	ERR("mq create send");


	if( 
		(mqd_t)-1 == (parent_receive = TEMP_FAILURE_RETRY( mq_open("/parent_in",O_CREAT | O_RDWR | O_NONBLOCK, 0600, &attr) ) )
	)
	ERR("mq create receive");


    return EXIT_SUCCESS;
}