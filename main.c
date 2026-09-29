#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h> // i/o
#include <stdlib.h> // malloc  
#include <unistd.h> // syscalls
#include <sys/types.h> //wait
#include <sys/wait.h>
#include <signal.h> // ctrl + c seremi
#include "lector.h"
#include <string.h>
#include <time.h>
#include <sys/resource.h> // libreria que permite pedir mas memoria para tuberias en caso de ejecucion de 10000 acts

struct Actividad actividadesTotales[10000]; // arreglo de actividades, global para que la funcion de la seremi tenga acceso
int nroActividades = 0;

void cayoLaSeremi(int sig){

    (void)sig;

    for(int i=0; i<nroActividades; i++){

        if(actividadesTotales[i].estado == RUNNING){ //solo mata a los que se estaban ejecutando, no a los ready ni los que no existian aun

            kill(actividadesTotales[i].PID, SIGKILL);
            actividadesTotales[i].estado = -1;
        }
        else if(actividadesTotales[i].estado == WAITING) actividadesTotales[i].estado = -1;
    }

    printf("Llego la Seremi, Todas las tareas en ejecucion han sido abortadas\n");
    exit(0);
}

int revisarDependencias(struct Actividad *actividadesTotalesMain, int id, int nroActividadesMain, int *contadorActReady){ //revisa si puede ser ejecutada dicha actividad

    char **cDependencias = actividadesTotalesMain[id].depend;
    int limite = actividadesTotalesMain[id].num_depend;

    for(int i=0; i<limite; i++){

        char *idDependencia = cDependencias[i]; 

        for(int j=0; j<nroActividadesMain; j++){

            if(strcmp(idDependencia, actividadesTotalesMain[j].ID_Actividad) == 0 && actividadesTotalesMain[j].estado == FAILED){ 

                actividadesTotalesMain[id].estado = FAILED;  //si su dependencia fallo, pasa a failed tmb
                printf("Actividad ID: %s CANCELADA. (Dependencia %s fallo)\n", actividadesTotalesMain[id].ID_Actividad, idDependencia);
                (*contadorActReady)++;
                return 0;
            }

            if(strcmp(idDependencia, actividadesTotalesMain[j].ID_Actividad) == 0 && actividadesTotalesMain[j].estado != READY) return 0;

        }
    }

    return 1;
}

int main(int argc, char *argv[]){

    srand(time(NULL));

    signal(SIGINT, cayoLaSeremi); // instruccion asincrona ctrl + c

    struct rlimit limite_pipes;
    limite_pipes.rlim_cur = 20000; // Limite actual que usara el programa
    limite_pipes.rlim_max = 20000; // Limite maximo absoluto
    setrlimit(RLIMIT_NOFILE, &limite_pipes);

    if(argc != 3){
        printf("Ejecucion incorrecta\n"); //revisa que la invocacion sea con el nro de argumentos correctos
        return -1;
    }

    int k = atoi(argv[2]);

    if(procesar_archivo(argv[1], actividadesTotales, &nroActividades) == -1){ //se lee plan.txt en lector.c para guardar la lista de actividades
        return -1;
    }

    int contadorActReady = 0;
    int contadorActRunning = 0;

    while(contadorActReady < nroActividades){ //hasta que se ejecuten todas las actividades

        for(int i=0; i<nroActividades && contadorActRunning < k; i++){ //recorre el total de act

            if(actividadesTotales[i].estado == WAITING){ //se puede crear una nueva act?

                if(revisarDependencias(actividadesTotales, i, nroActividades, &contadorActReady)){ // revisa si todas sus dependencias estan listas, o si no tiene

                    actividadesTotales[i].estado = RUNNING;
                    contadorActRunning++;

                    pipe(actividadesTotales[i].fd); //inicializa el pipe 

                    pid_t pid = fork();

                    if(pid == 0){ 
                        
                        printf("Actividad ID: %s iniciada (PID: %d).\n", actividadesTotales[i].ID_Actividad, getpid());
                        printf("Accion: %s\n\n", actividadesTotales[i].Nombre_Actividad);

                        char buffer[15];
                        char mensaje[] = "Listo Brother";

                        for(int d = 0; d < actividadesTotales[i].num_depend; d++){ //lee el mensaje del pipe que le dejo cada dependencia
                            
                            char *id_buscado = actividadesTotales[i].depend[d];
                            
                            for(int j = 0; j < nroActividades; j++){
                                
                                if(strcmp(id_buscado, actividadesTotales[j].ID_Actividad) == 0){
                                    
                                    read(actividadesTotales[j].fd[0], buffer, sizeof(mensaje));
                                    printf("[PIPE-READ] Actividad ID: %s leyo el Listo Brother de su dependencia %s.\n", actividadesTotales[i].ID_Actividad, id_buscado);
                                    break; 
                                }
                            }
                        }

                        close(actividadesTotales[i].fd[0]);  
                        
                        for(int j = 0; j < actividadesTotales[i].cantidad_dependientes; j++){ //le deja el mensaje a sus dependientes
                            
                            write(actividadesTotales[i].fd[1], mensaje, sizeof(mensaje));
                        }

                        if(actividadesTotales[i].cantidad_dependientes > 0){
                            printf("[PIPE-WRITE] Actividad ID: %s dejo su mensaje listo para %d dependientes.\n", actividadesTotales[i].ID_Actividad, 
                                actividadesTotales[i].cantidad_dependientes);
                        }

                        close(actividadesTotales[i].fd[1]); 

                        usleep(actividadesTotales[i].tiempo_ms * 1000); //realiza su actividad
                        exit(0);
                    }

                    actividadesTotales[i].PID = pid;
                    close(actividadesTotales[i].fd[1]);
                }
            }
        }

        if(contadorActRunning > 0){
            
            int status;
            pid_t pidListo = wait(&status); // pid del primer hijo que termine
        
            for(int i=0; i<nroActividades; i++){
                
                if(actividadesTotales[i].PID == pidListo){
                    
                    if(WIFEXITED(status)){
                        
                        int exit_code = WEXITSTATUS(status);
                        
                        if(exit_code == 0){

                         actividadesTotales[i].estado = READY;
                         printf("Actividad ID: %s terminada con exito.\n", actividadesTotales[i].ID_Actividad);
                        }
                               
                        else{

                            actividadesTotales[i].estado = FAILED; 
                            printf("ERROR: Actividad ID: %s ha fallado.\n", actividadesTotales[i].ID_Actividad);
                        }   
                    }
                    
                    contadorActRunning--;
                    contadorActReady++; 
                    break;
                }
            }
        }
    }

    return 0;
}