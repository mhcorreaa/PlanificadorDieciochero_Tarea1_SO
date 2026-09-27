#include <stdio.h> // i/o
#include <stdlib.h> // malloc  
#include <unistd.h> // syscalls
#include <sys/types.h> //wait
#include <sys/wait.h>
#include <signal.h>
#include "lector.h"

int main(int argc, char *argv[]){

    if(argc != 3){
        printf("Ejecucion incorrecta\n");
        return -1;
    }

    int k = atoi(argv[2]);

    struct Actividad actividadesTotales[10000]; // arreglo de actividades
    int nroActividades = 0;

    if(procesar_archivo(argv[1], actividadesTotales, &nroActividades) == -1){ //se lee plan.txt en lector.c para guardar la lista de actividades
        return -1;
    }

    int contadorActReady = 0;

    while(contadorActReady < nroActividades){

        for(int i=0; i<nroActividades; i++){



        }
    }

    return 0;
}