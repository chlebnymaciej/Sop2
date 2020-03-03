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


int main(int argc, char **argv){
    char* st;
    st=(char*)malloc(sizeof(char)*5);
    st[0]='d';
    st[1]='u';
    st[2]='p';
    st[3]='a';
    st[4]='\0';


    printf("%s \n",st);
    if(fork()==0){
        free(st);
        printf("zwolnienie stringa %s \n",st);
        exit(EXIT_SUCCESS);
    }else{
        sleep(2);
        printf("string po spaniu %s\n",st);
    }
    return EXIT_SUCCESS;
}