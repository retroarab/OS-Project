//program that cretaes a aparrennt 4 childen. each children will print 
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>
    int pid;
    int pid_child;
    char bufferP[200];
     char buffer[200] = "";
int main() {
    int status;
    for(int i = 0; i < 4; i++) {
       
        if((pid = fork()) < 0) {
            perror("\nerror");

        }
        if(pid == 0) {
            sprintf(buffer, "\nprocess %d and parent with %d and  child  %d",i, getppid, getpid());
            printf("\n%s", buffer);
            exit(i);
        }
        

    
    sleep(1);
    }

    sprintf(bufferP, "\nprocess parent with pid %d with parent %d", getpid(), getppid());
    printf("\n%s", bufferP);
    for(int i =0; i < 4; i++) {
        pid_child = wait(&status);
        if(pid_child < 0) {
            perror("\nzombie error ");
        }
        if(WIFEXITED(status)) {
            printf("\nprocess with pid %d with status %d", pid_child, WEXITSTATUS(status));
        }
    }
     printf("\n");
    return 0;
}