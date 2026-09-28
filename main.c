#include <stdio.h> // i/o
#include <stdlib.h> // malloc  
#include <unistd.h> // syscalls
#include <sys/types.h> //wait
#include <sys/wait.h>
#include <signal.h>
#include "lector.h"
#include <string.h>


int revisarDependencias(struct Actividad *actividadesTotales, int id, int nroActividades){ //revisa si puede ser ejecutada dicha actividad
                                                                                            //si no tiene dependencias, o en caso contrario, que esten listas
    char **cDependencias = actividadesTotales[id].depend;
    int limite = actividadesTotales[id].num_depend;

    for(int i=0; i<limite; i++){

        char *idDependencia = cDependencias[i]; 

        for(int j=0; j<nroActividades; j++){

            if(strcmp(idDependencia, actividadesTotales[j].ID_Actividad) == 0 && actividadesTotales[j].estado != 2) return 0;
            
        }
    }

    return 1;
}


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
    int contadorActRunning = 0;

    while(contadorActReady < nroActividades){

        for(int i=0; i<nroActividades; i++){

            if(contadorActRunning < k && actividadesTotales[i].estado == WAITING){

                if(revisarDependencias(actividadesTotales, i, nroActividades)){ // revisa si todas sus dependencias estan listas, o si no tiene

                    printf("Iniciando actividad ID: %s\n", actividadesTotales[i].ID_Actividad);
                    printf("Accion:  %s\n\n", actividadesTotales[i].Nombre_Actividad);

                    actividadesTotales[i].estado = RUNNING;
                    contadorActRunning++;

                    pid_t pid = fork();

                    if(pid == 0){ 
                        
                        usleep(actividadesTotales[i].tiempo_ms * 1000); // usleep recibe microsegundos, por lo que se multiplica por 1000
                        exit(0);
                    }

                    actividadesTotales[i].PID = pid;
                }
        }

        if(contadorActRunning >0 ){ //esperar a que termine un proceso para continuar las otras ejecuciones

            pid_t pidListo = wait(NULL); // pid del primer proceso que termina

            for(int i=0; i<nroActividades; i++){

                if(actividadesTotales[i].PID == pidListo){

                    actividadesTotales[i].estado == READY;
                    contadorActReady++;
                    contadorActRunning--;
                    break;
                }
            }
        }
    }

    return 0;
}