#include <stdio.h> // i/o
#include <stdlib.h> // malloc  
#include <unistd.h> // syscalls
#include <string.h>
#include <sys/types.h> //wait
#include <sys/wait.h>
#include <signal.h>

struct Actividad{

    char **id;
    char nombreAct[50];
    int time;
    char **depend;

};

int main(){

    printf("Hello world");

    return 0;
}