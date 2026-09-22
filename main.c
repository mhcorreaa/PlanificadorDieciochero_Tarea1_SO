#include <stdio.h> // i/o
#include <stdlib.h> // malloc  
#include <unistd.h> // syscalls
#include <string.h>
#include <sys/types.h> //wait
#include <sys/wait.h>
#include <signal.h>

struct Actividad{

    char *id;
    char *nombreAct;
    int time;
    char **depend;
    int num_depend;
};

int main(){

    FILE *read = fopen("plan.txt", "r"); //puntero que lee archivo en modo read

    if(read == NULL){

        printf("ERROR: Archivo no encontrado");
        return -1;
    }

    fseek(read, 0, SEEK_END); //saber tamano del archivo moviendo el puntero al final
    long tamanoArchivo = ftell(read);
    rewind(read);

    char buffer[tamanoArchivo];

    while(fgets(buffer, tamanoArchivo, read) != NULL){

        
    }

    fclose(read);

    int 
}