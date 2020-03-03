#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
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
	fprintf(stderr,"0<n<=10 - number of children\n");
	exit(EXIT_FAILURE);
}

/* Przydatne makro 
    TEMP_FAILURE_RETRY(expression)

*/

int main(int argc, char** argv){
    /*
    kod zadania
    */

    return EXIT_SUCCESS;
}